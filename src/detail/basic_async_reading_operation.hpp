#pragma once
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <tuple>
#include <array>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "operation_base.hpp"
#include "reading_operation_environment.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

template<typename InputSerializer, typename... Serializables>
class basic_async_reading_operation : 
	public operation_base
{
public:
	basic_async_reading_operation(
			reading_operation_environment& environment,
			std::function<void(const Serializables&)>... completion_handlers
			) :
		_environment(environment),
		_completion_handlers(completion_handlers...)
	{
	}

	void async_execute() override
	{
		using namespace std::placeholders;
		auto buffer = detail::buffer(_environment.work_buffer, 2);
		auto binder	= std::bind(
				&basic_async_reading_operation<InputSerializer, Serializables...>
				::_on_header_read, this, _1, _2);
		detail::async_read(_environment.socket, buffer, binder);
	}

private:
	bool _check_connection(const boost::system::error_code& er)
	{
		if(er != boost::system::errc::success)
		{
			_environment.disconnected_handler();
			return true;
		}
		return false;
	}

	template<typename Serializable>
	bool _process_header()
	{
		using namespace std::placeholders;
		auto buffer = boost::asio::buffer_cast<uint8_t*>(_environment.work_buffer);
		if(buffer[0] != Serializable::packet_id)
			return false;
		auto binder = std::bind(
				&basic_async_reading_operation<InputSerializer, Serializables...>
				::_on_body_read<Serializable>, this, _1, _2);
		detail::async_read(_environment.socket, _environment.work_buffer, binder);
		return true;
	}

	void _on_header_read(const boost::system::error_code& er, std::size_t)
	{
		if(_check_connection(er)) return;
		auto results = { _process_header<Serializables>()... };
		auto it = std::find(std::begin(results), std::end(results), true);
		if(it == std::end(results)) throw std::runtime_error("header not found"); //TODO
	}

	template<typename Serializable>
	void _on_body_read(const boost::system::error_code& er, std::size_t)
	{
		if(_check_connection(er)) return;
		auto data = boost::asio::buffer_cast<uint8_t*>(_environment.work_buffer);
		auto data_size = boost::asio::buffer_size(_environment.work_buffer) - 1;
		InputSerializer::template process_input_data<Serializable>(&data[2], data[1], 
				data, data_size);
		auto serializable_obj = reinterpret_cast<Serializable*>(&data[2]);
		using handle_t = std::function<void(const Serializable&)>;
		std::get<handle_t>(_completion_handlers)(*serializable_obj);
		_environment.inner_operation_complete_handler(*this);
	}

	reading_operation_environment& _environment;
	std::tuple<std::function<void(const Serializables&)>...> _completion_handlers;
};

}
}
