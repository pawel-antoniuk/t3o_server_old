#pragma once
#include <boost/asio.hpp>
#include <array>
#include <cstdint>
#include <utility>
#include <iostream> //debug
#include <functional>

#include "protocol_detail.hpp"
#include "async_serializer.hpp"
#include "event.hpp"

namespace t3o
{
	namespace detail
	{
		using namespace boost::asio;
		using boost::asio::ip::tcp;
		using std::array;
	}
	class game_session
	{
		public:
			explicit game_session(detail::io_service& service) :
				_socket{service},
				_serializer{_socket},
				_is_closed{false}
			{
				_serializer.event_disconnected() += std::bind(&game_session::close, this);
			}

			~game_session()
			{
				close();
			}

			void async_run()
			{
				//do handshake
				using namespace std::placeholders;
				using namespace detail::protocol;
				auto field_set_binder = std::bind(&game_session::_on_field_set, this, _1);
				auto handshake_binder = std::bind(&game_session::_on_handshake, this, _1);
				_serializer.async_read<handshake_t, field_set_packet_t>(handshake_binder, field_set_binder);
			}
			
			template<typename Handler>
			void async_send_field_set(unsigned x, unsigned y, unsigned field, Handler handler)
			{
				detail::protocol::field_set_packet_t packet;
				packet.x = x;
				packet.y = y;
				packet.field = field;
				_serializer.async_write(packet, handler);
			}

			auto& event_field_set()
			{
				return _field_set_event;
			}

			detail::tcp::socket& socket()
			{
				return _socket;
			}

			auto& event_disconnected()
			{
				return _serializer.event_disconnected();
			}

			void close()
			{
				if(_is_closed) return;
				_disconnected_event();
				_socket.close();
				_is_closed = true;
			}

			bool is_closed()
			{
				return _is_closed;
			}

		private:
			void _on_handshake(const detail::protocol::handshake_t& data)
			{

			}

			void _on_field_set(const detail::protocol::field_set_packet_t& data)
			{
				using namespace std::placeholders;
				auto binder = std::bind(&game_session::_on_field_set, this, _1);
				_serializer.async_read<detail::protocol::field_set_packet_t>(binder);
			}

			detail::tcp::socket _socket;
			detail::text_async_serializer _serializer;
			bool _is_closed;

			//events
			event<void(unsigned x, unsigned y, unsigned field)> _field_set_event;
			event<void()> _disconnected_event;
	};
}

