#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <cstdio>
#include "game_server.hpp"
#include "protocol_detail.hpp"
#include "serializer.hpp"


int main()
{
	using namespace boost;
	using namespace boost::asio::ip;
	asio::io_service service;
	asio::ip::tcp::socket socket{service};
	t3o::game_server server{service};
	std::cout << "server started" << std::endl;
	t3o::detail::async_serializer serializer{socket};	
}
