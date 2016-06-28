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
#include <iostream> //debug

#include "event.hpp"

namespace t3o
{
	namespace detail
	{
		namespace detail
		{
			using std::ostringstream;
			using std::istringstream;
			using boost::archive::text_oarchive;
			using boost::archive::text_iarchive;
			using boost::asio::io_service;
			using boost::asio::ip::tcp; 
			using boost::asio::buffer;
			using boost::asio::const_buffer;
			using boost::asio::mutable_buffer;
			using boost::asio::async_write;
			using boost::asio::async_read;
			using boost::system::error_code;
			using namespace boost::system::errc;
			using packet_size_t = uint8_t;
			using packet_id_t = uint8_t;
		}

		class async_serializer 
		{
			public:
				explicit async_serializer(detail::tcp::socket& socket) :
					_socket(socket)
				{
					_clear_read_state();
					_clear_write_state();
				}

				template<typename Serializable, typename Handler>
				void async_write(const Serializable& t, Handler handler)
				{
					auto packet = _prepare_output_packet(t);
					detail::async_write(_socket, packet, handler);
				}

				template<typename Serializable, typename Handler>
				void async_read(Handler handler)
				{
					using namespace std::placeholders;
					std::array<detail::mutable_buffer, 2> packet{{
						detail::buffer(&_tmp_read_packet_size, sizeof(_tmp_read_packet_size)),
						detail::buffer(&_tmp_read_packet_id, sizeof(_tmp_read_packet_id))
					}};
					auto binder = std::bind(&async_serializer::_read_header<Serializable, Handler>, 
								this, handler, _1, _2);
					detail::async_read(_socket, packet, binder);
				}

				auto& event_disconnected()
				{
					return _disconnected_event;
				}

			
			private:

				template<typename Serializable>
				std::array<detail::const_buffer, 3> _prepare_output_packet(const Serializable& t)
				{
					detail::ostringstream oss;
					detail::text_oarchive archive{oss, boost::archive::no_header};
					archive << t;
					_tmp_write_output = oss.str();
					_tmp_write_packet_size  = static_cast<uint8_t>(_tmp_write_output.size());
					std::array<detail::const_buffer, 3> packet{{
						detail::buffer(&_tmp_write_packet_size, sizeof(_tmp_write_packet_size)),
						detail::buffer(&Serializable::packet_id, sizeof(Serializable::packet_id)),
						detail::buffer(_tmp_write_output) 
					}};
					return packet;
				}

				template<typename Serializable>
				Serializable _prepare_input_packet(const std::string& data, detail::packet_size_t size)
				{
					detail::istringstream iss{data};
					detail::text_iarchive archive{iss};
					Serializable t;
					archive >> t;
					return t;
				}


				bool _check_connection(const detail::error_code& er)
				{
					if(er == detail::success) return true;
					_disconnected_event();
					return false;
				}
				
				template<typename Serializable, typename Handler>
				void _read_header(Handler handler, const detail::error_code& er, std::size_t size)
				{
					if(!_check_connection(er)) return;
					if(_tmp_read_packet_id != Serializable::packet_id) throw "bad packet"; //TODO

					using namespace std::placeholders;
					_tmp_read_packet_body.resize(_tmp_read_packet_size, 0);
					auto binder = std::bind(&async_serializer::_read_body<Serializable, Handler>, 
								this, handler, _1, _2);
					detail::async_read(_socket, detail::buffer(&_tmp_read_packet_body[0], _tmp_read_packet_size),
							binder);
				}

				template<typename Serializable, typename Handler>
				void _read_body(Handler handler, const detail::error_code& er, std::size_t size)
				{
					if(!_check_connection(er)) return;

					auto t = _prepare_input_packet<Serializable>(_tmp_read_packet_body, _tmp_read_packet_size);
					handler(t);
				}

				void _clear_write_state()
				{
					_tmp_write_packet_size = 0;
					_tmp_write_output.clear();
				}

				void _clear_read_state()
				{
					_tmp_read_packet_size = 0;
					_tmp_read_packet_id = 0;
					_tmp_read_packet_body.clear();
				}

				detail::tcp::socket& _socket;

				//read
				detail::packet_size_t _tmp_read_packet_size;
				detail::packet_id_t _tmp_read_packet_id;
				std::string _tmp_read_packet_body;

				//write
				detail::packet_size_t _tmp_write_packet_size;
				std::string _tmp_write_output;


				//events
				event<void()> _disconnected_event;
		};
	}
}
