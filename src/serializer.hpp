#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <string>
#include <array>

namespace t3o
{
	namespace detail
	{
		namespace detail
		{
			using std::array;
			using std::ostringstream;
			using boost::archive::binary_oarchive;
			using boost::asio::io_service;
			using boost::asio::ip::tcp; 
			using boost::asio::buffer;
			using boost::asio::const_buffer;

			class basic_serializer
			{
				protected:
					template<typename Serializable>
					detail::array<detail::const_buffer, 2> prepare_packet(Serializable& t)
					{
						detail::ostringstream oss;
						detail::binary_oarchive archive{oss};
						archive << t;
						auto output = oss.str();
						auto size = static_cast<uint8_t>(output.size());
						detail::array<detail::const_buffer, 2> packet{{
							detail::buffer(&size, 1),
							detail::buffer(output) 
						}};
						return packet;
					}
			};

		}

		class async_serializer : detail::basic_serializer
		{
			public:
				async_serializer(detail::tcp::socket& socket) :
					_socket(socket)
				{
				}
				
				template<typename Serializable>
				async_serializer& operator<<(Serializable& t)
				{
					auto packet = prepare_packet(t);
					_socket.async_write_some(packet);
					return *this;
				}

			private:
				detail::tcp::socket& _socket;
		};
	}
}
