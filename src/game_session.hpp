#pragma once
#include <boost/asio.hpp>
#include <array>
#include <cstdint>
#include <utility>
#include <iostream> //debug

#include "protocol_detail.hpp"
#include "serializer.hpp"

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
			explicit game_session(detail::tcp::socket&& socket) :
				_socket{std::move(socket)},
				_serializer{_socket}
			{
				async_run();
			}

			void async_run()
			{
				_serializer.async_read<detail::protocol::field_set_packet_t>([this](auto& data){
					std::cout << "kek" << std::endl;
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

		private:

			detail::array<uint8_t, 8> _packet_buffer;
			detail::tcp::socket _socket;
			detail::async_serializer _serializer;
	};
}

