#pragma once
#include <boost/asio.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <string>
#include <array>
#include <utility>
#include <functional>
#include <memory>
#include <tuple>
#include <algorithm>
#include <iostream> //debug

#include "event.hpp"
#include "exceptions.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using boost::asio::io_service;
	using boost::asio::ip::tcp; 
	using boost::asio::buffer;
	using boost::asio::const_buffer;
	using boost::asio::mutable_buffer;
	using boost::asio::async_write;
	using boost::asio::async_read;
	using boost::system::error_code;
	using namespace boost::system::errc;
}

template<typename InputArchive>
class basic_async_iserializer 
{
public:
	explicit basic_async_iserializer(detail::tcp::socket& socket) :
			_socket(socket)
	{
	}
	template<typename ...Serializables>
	void async_read(std::function<void(const Serializables&)>... handlers)
	{
		using namespace std::placeholders;
		auto metadata = std::make_shared<packet_header_t>();
		std::array<detail::mutable_buffer, 2> packet{{
			detail::buffer(&metadata->size, sizeof(metadata->size)),
			detail::buffer(&metadata->packet_id, sizeof(metadata->packet_id))
		}};
		auto binder = std::bind(&basic_async_iserializer<InputArchive>
				::_on_header_read<Serializables...>, this, _1, _2, metadata, handlers...);
		detail::async_read(_socket, packet, binder);
	}
	auto& event_disconnected()
	{
		return _disconnected_event;
	}

private:
	struct packet_header_t
	{
		uint8_t size;
		uint8_t packet_id;
	};
	
	struct packet_body_t
	{
		std::string data;
	};

	template<typename Serializable>
	Serializable _prepare_input_packet(const std::string& data)
	{
		std::istringstream iss{data};
		InputArchive archive{iss, boost::archive::no_header};
		Serializable t;
		archive >> t;
		return t;
	}

	bool _check_connection(const detail::error_code& er)
	{
		if(er == detail::success) return false;
		_disconnected_event();
		return true;
	}

	template<typename ...Serializables>
	void _on_header_read(const detail::error_code& er, std::size_t,
			std::shared_ptr<packet_header_t> data,
			std::function<void(const Serializables&)>... handlers)
	{
		if(_check_connection(er)) return;
		auto results = { _process_header(handlers, data)... };
		auto it = std::find(std::begin(results), std::end(results), true);
			if(it == std::end(results)) _disconnected_event(); //packet error
	}

	template<typename Serializable>
	bool _process_header(std::function<void(const Serializable&)> handler,
					const std::shared_ptr<packet_header_t>& data)
	{
		if(data->packet_id != Serializable::packet_id) return false;
		using namespace std::placeholders;
		auto metadata = std::make_shared<packet_body_t>();
		metadata.data.resize(data.size, 0);
		auto binder = std::bind(&basic_async_iserializer<InputArchive>
				::_on_body_read<Serializable>, this, _1, _2, handler, metadata);
		detail::async_read(_socket, detail::buffer(&metadata->data[0], data->size),
				binder);
		return true;
	}
	
	template<typename Serializable>
	void _on_body_read(const detail::error_code& er, std::size_t size,
			std::function<void(const Serializable&)> handler, 
			std::shared_ptr<packet_body_t> data)
	{
		if(_check_connection(er)) return;
		auto t = _prepare_input_packet<Serializable>(data->data);
		handler(t);
	}

	detail::tcp::socket& _socket;
	
	//events
	event<void()> _disconnected_event;
};

using text_async_iserializer = basic_async_iserializer<boost::archive::text_iarchive>;

}
}
