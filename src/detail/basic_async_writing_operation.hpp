#pragma once
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <tuple>
#include <array>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "operation_base.hpp"
#include "operation_environment.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

template<typename OutputSerializer, typename Serializable>
class basic_async_writing_operation : 
	public operation_base
{
public:
	basic_async_writing_operation(
			operation_environment& environment,
			std::function<void()> completion_handler,
			const Serializable& object
			) :
		_environment(environment),
		_completion_handler(completion_handler),
		_object(object)
	{
	}

	void async_execute() override
	{
		using namespace std::placeholders;
		auto buf_ptr = boost::asio::buffer_cast<uint8_t*>(
				_environment.work_buffer) + 2;
		auto buf_size = boost::asio::buffer_size(
				_environment.work_buffer) - 2;
		auto new_size = OutputSerializer::process_output_data(_object,
				buf_ptr, buf_size);	
		buf_ptr[0] = static_cast<uint8_t>(Serializable::packet_id);
		buf_ptr[1] = static_cast<uint8_t>(new_size);
		auto binder = std::bind(&basic_async_writing_operation
				<OutputSerializer, Serializable>::_on_pacekt_written,
				this, _1, _2);
		boost::asio::async_write(_environment.socket,
				_environment.work_buffer, binder);
	}

private:
	bool _check_connection(const boost::system::error_code& er)
	{
		if(er != boost::system::errc::success)
		{
			_environment.disconnected_event();
			return true;
		}
		return false;
	}

	void _on_pacekt_written(const boost::system::error_code& er, 
			std::size_t)
	{
		if(_check_connection(er)) return;
		_completion_handler();
		_environment.inner_operation_complete_handler(*this);
	}

	operation_environment& _environment;
	std::function<void()> _completion_handler;
	Serializable _object;
};

}
}
