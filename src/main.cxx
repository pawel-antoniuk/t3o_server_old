#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <cstdio>
#include <thread>
#include <chrono>

#include "game_server.hpp"

int main()
{
	using namespace boost;
	using namespace boost::asio::ip;
	using namespace std::chrono_literals;

	asio::io_service service;
	tcp::endpoint endpoint{tcp::v4(), 6667};
	t3o::game_server server{service, endpoint};
	server.event_session_started() += [](auto& session){
		std::cout << "hi" << std::endl;
	};
	server.event_session_ended() += [](auto& session){
		std::cout << "bye" << std::endl;
	};
	server.event_user_field_set() += [](auto& session, auto x, auto y, auto field){
		std::cout << x << ", " << y << ", " << field << std::endl;
	};
	std::cout << "server have beend started" << std::endl;
	service.run();
}
