#include <boost/asio.hpp>
#include <array>
#include <cstdint>

#include "protocol_detail.hpp"

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
			void async_run()
			{
				_socket.async_read_some(detail::buffer(_packet_buffer,
							sizeof(_packet_buffer.header)),
						_read_next_header);
			}

		private:
			void _read_next_header()
			{
				if
			}

			void _read_next_position_set()
			{
			}

			detail::array<uint8_t, 8> _packet_buffer;
			
			detail::tcp::socket _socket;
	};
}

