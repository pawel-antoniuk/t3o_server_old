#pragma once
#include <boost/asio.hpp>
#include <array>
#include <cstdint>
#include <utility>
#include <iostream> //debug

#include "protocol_detail.hpp"
#include "serializer.hpp"
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
				_serializer.async_read<detail::protocol::field_set_packet_t>([this](auto& data){
					std::cout << "kek" << std::endl;
					_field_set_event(data.x, data.y, data.field);
				});
			}

			void async_send_field_set(unsigned x, unsigned y, unsigned field)
			{
				detail::protocol::field_set_packet_t packet;
				packet.x = x;
				packet.y = y;
				packet.field = field;
				_serializer.async_write(packet, [](auto er, auto size){});
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

			detail::tcp::socket _socket;
			detail::async_serializer _serializer;
			bool _is_closed;

			//events
			event<void(unsigned x, unsigned y, unsigned field)> _field_set_event;
			event<void()> _disconnected_event;
	};
}

