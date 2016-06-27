#include <boost/asio.hpp>
#include <array>
#include <cstdint>
#include <utility>

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

			}

			void async_run()
			{
			}

			void async_send_field_set(unsigned x, unsigned y, unsigned field)
			{
				detail::protocol::field_set_packet_t packet;
				packet.x = x;
				packet.y = y;
				packet.field = field;
				_serializer << packet;
			}

		private:
			void _read_next_header()
			{
			}

			void _read_next_position_set()
			{
			}

			detail::array<uint8_t, 8> _packet_buffer;
			detail::tcp::socket _socket;
			detail::async_serializer _serializer;
	};
}

