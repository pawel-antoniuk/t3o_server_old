#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <cstdio>
#include "server.hpp"
#include "protocol_detail.hpp"

void print_packet(t3o::detail::protocol::field_set_packet& p)
{
	std::cout << (unsigned) p.in.msg_id << ", "
		<< (unsigned) p.in.x << ", "
		<< (unsigned) p.in.y << ", "
		<< (unsigned) p.in.field << std::endl;

}

int main()
{
	using namespace boost;
	using namespace boost::asio::ip;
	asio::io_service service;
	t30::server server{service};
	std::cout << "server started" << std::endl;
		
	server.run([](tcp::socket& socket, tcp::endpoint& endpoint){
			auto addr_str = endpoint.address().to_string();
			std::cout << "new connection: " << addr_str << std::endl;

			t3o::detail::protocol::field_set_packet io_packet;
			socket.read_some(asio::buffer(&io_packet, t3o::detail::protocol::packet_size));
			
			print_packet(io_packet);			
			
			io_packet.out.msg_id = 2;
			io_packet.out.result = 1;
			socket.write_some(asio::buffer(&io_packet, t3o::detail::protocol::packet_size));
	});
}
