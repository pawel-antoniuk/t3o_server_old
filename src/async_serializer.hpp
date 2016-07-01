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
#include "exceptions.hpp"

namespace t3o
{
	namespace detail
	{
		namespace detail
		{
			using std::ostringstream;
			using std::istringstream;
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

		template<typename InputArchive, typename OutputArchive>
		class basic_async_serializer 
		{
			using iarchive_t = InputArchive;
			using oarchive_t = OutputArchive;
			public:
				explicit basic_async_serializer(detail::tcp::socket& socket) :
					_socket(socket)
				{
					_clear_read_state();
					_clear_write_state();
				}

				template<typename Serializable, typename Handler>
				void async_write(const Serializable& t, Handler handler)
				{
					auto packet = _prepare_output_packet(t);
					using namespace std::placeholders;
					auto binder = std::bind(&basic_async_serializer<InputArchive, OutputArchive>
							::_on_packet_written<Serializable, Handler>, this, handler, _1, _2);
					detail::async_write(_socket, packet, binder);
				}

				template<typename T>
				void test(T handler)
				{
				}
				
				template<typename Serializable, typename Handler>
				void async_read(Handler handler)
				{
					Serializable t;
					handler(t);
					using namespace std::placeholders;
					std::array<detail::mutable_buffer, 2> packet{{
						detail::buffer(&_tmp_read_packet_size, sizeof(_tmp_read_packet_size)),
						detail::buffer(&_tmp_read_packet_id, sizeof(_tmp_read_packet_id))
					}};
					//auto binder = std::bind(&basic_async_serializer<InputArchive, OutputArchive>
					//		::_on_header_read<Serializable, Handler>, this, handler, _1, _2);
					//auto binder = std::bind(&basic_async_serializer<InputArchive, OutputArchive>
					//		::_on_packet_written<Serializable, Handler>, this, handler, _1, _2);

					//binder(boost::system::errc::success, 0);
					auto binder = std::bind(
								&basic_async_serializer<InputArchive,OutputArchive>::test<Handler>, this, 
								_1);

					binder(handler);
					
				//i	detail::async_write(_socket, packet, binder);
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
					oarchive_t archive{oss, boost::archive::no_header};
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
					iarchive_t archive{iss, boost::archive::no_header};
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
				void _on_packet_written(Handler handler, const detail::error_code& er, std::size_t size)
				{
					if(!_check_connection(er)) return;

					handler();
				}
				
				template<typename Serializable, typename Handler>
				void _on_header_read(Handler handler, const detail::error_code& er, std::size_t size)
				{
					if(!_check_connection(er)) return;
					if(_tmp_read_packet_id != Serializable::packet_id) throw packet_format_error();

					using namespace std::placeholders;
					_tmp_read_packet_body.resize(_tmp_read_packet_size, 0);
					auto ptr = &basic_async_serializer<InputArchive, OutputArchive>
							::_on_body_read<Serializable, Handler>;
					//(*this.*ptr)(handler, er, size);
					auto binder = std::bind(ptr, this, handler,er, size);
					//binder();
					//detail::async_read(_socket, detail::buffer(&_tmp_read_packet_body[0], _tmp_read_packet_size),
					//		binder);
				}

				template<typename Serializable, typename Handler>
				void _on_body_read(Handler handler, const detail::error_code& er, std::size_t size)
				{
					if(!_check_connection(er)) return;

					auto t = _prepare_input_packet<Serializable>(_tmp_read_packet_body, _tmp_read_packet_size);
					handler(t);
					_clear_read_state();
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

		using text_async_serializer 
			= basic_async_serializer<boost::archive::text_iarchive, boost::archive::text_oarchive>;
	}
}
