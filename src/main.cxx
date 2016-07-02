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
	server.event_session_started() += [&server](auto& session){
		std::cout << "hi" << std::endl;
		session.event_logged() += [&server, &session]{
			std::cout << "logged" << std::endl;
			std::cout << session.name() << std::endl;
			session.begin_game(2, 3, 3);
		};
	};
	server.event_session_ended() += [](auto& session){
		std::cout << "bye" << std::endl;
	};
	server.event_user_field_set() += [](auto& session, auto x, auto y, auto field){
		std::cout << x << ", " << y << ", " << field << std::endl;
	};
	server.start_listen_for_players();
	std::cout << "server has beend started" << std::endl;
	service.run();
}
