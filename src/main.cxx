#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <cstdio>
#include <thread>
#include <chrono>
#include "game_server.hpp"
#include "protocol_detail.hpp"
#include "serializer.hpp"

using namespace boost;
using namespace boost::asio::ip;
using namespace std::chrono_literals;

boost::asio::io_service service;

void server_listener()
{
	//service.run();
	tcp::endpoint endpoint{tcp::v4(), 6663};
	tcp::acceptor accepter{service, endpoint};
	tcp::socket socket{service};
	accepter.accept(socket);
	std::array<uint8_t, 128> buf;
	auto size = socket.read_some(asio::buffer(buf));
	std::cout.write(reinterpret_cast<const char*>(buf.data()), buf.size());
}

int main()
{
	std::thread server_thread{server_listener};
	std::this_thread::sleep_for(1s);
	
	t3o::detail::protocol::field_set_packet_t packet{};
	tcp::endpoint endpoint{address::from_string("127.0.0.1"), 6663};
	asio::ip::tcp::socket socket{service};
	socket.connect(endpoint);
	t3o::game_server server{service};
	std::cout << "server started" << std::endl;
	t3o::detail::async_serializer serializer{socket};	
	serializer << packet;
	server_thread.join();
}
