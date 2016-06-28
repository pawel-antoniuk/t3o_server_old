#pragma once
#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/system/error_code.hpp>
#include <sstream>
#include <string>
#include <array>
#include <utility>
#include <functional>
#include <memory>
#include <tuple>

namespace t3o
{
	namespace detail
	{
		namespace detail
		{
			using std::ostringstream;
			using std::istringstream;
			using boost::archive::binary_oarchive;
			using boost::archive::binary_iarchive;
			using boost::asio::io_service;
			using boost::asio::ip::tcp; 
			using boost::asio::buffer;
			using boost::asio::const_buffer;
			using boost::asio::async_write;
			using boost::asio::async_read;
			using boost::system::error_code;

			class basic_serializer
			{
				protected:

					using packet_size_t = std::size_t;

					template<typename Serializable>
					std::array<detail::const_buffer, 2> prepare_output_packet(const Serializable& t)
					{
						detail::ostringstream oss;
						detail::binary_oarchive archive{oss};
						archive << t;
						_tmp_output = oss.str();
						_tmp_packet_size  = static_cast<uint8_t>(_tmp_output.size());
						std::array<detail::const_buffer, 2> packet{{
							detail::buffer(&_tmp_packet_size, sizeof(_tmp_packet_size)),
							detail::buffer(_tmp_output) 
						}};
						return packet;
					}

					template<typename Serializable>
					Serializable prepare_input_packet(const std::string& data, packet_size_t size)
					{
						detail::istringstream iss{data};
						detail::binary_iarchive archive{iss};
						Serializable t;
						archive >> t;
						return t;
					}

				private:
					packet_size_t _tmp_packet_size;
					std::string _tmp_output;

			};

		}

		class async_serializer : detail::basic_serializer
		{
			public:
				async_serializer(detail::tcp::socket& socket) :
					_socket(socket)
				{
				}
				
				template<typename Serializable, typename Handler>
				void async_write(const Serializable& t, Handler handler)
				{
					auto packet = prepare_output_packet(t);
					detail::async_write(_socket, packet, handler);
				}

				template<typename Serializable, typename Handler>
				void async_read(Handler handler)
				{
					using namespace std::placeholders;
					auto packet = detail::buffer(&_tmp_packet_size, sizeof(_tmp_packet_size));
					detail::async_read(_socket, packet, std::bind(
								&async_serializer::_read_header<Serializable, Handler>, 
								this, handler, _1, _2));
				}

			private:
				
				template<typename Serializable, typename Handler>
				void _read_header(Handler handler, const detail::error_code& er, std::size_t size)
				{
					using namespace std::placeholders;
					_tmp_packet_body.resize(_tmp_packet_size, 0);
					detail::async_read(_socket, detail::buffer(&_tmp_packet_body[0], _tmp_packet_size),
							std::bind(&async_serializer::_read_body<Serializable, Handler>, 
								this, handler, _1, _2));
				}

				template<typename Serializable, typename Handler>
				void _read_body(Handler handler, const detail::error_code& er, std::size_t size)
				{
					auto t = prepare_input_packet<Serializable>(_tmp_packet_body, _tmp_packet_size);
					handler(t);
				}

				detail::tcp::socket& _socket;
				packet_size_t _tmp_packet_size;
				std::string _tmp_packet_body;
		};
	}
}
