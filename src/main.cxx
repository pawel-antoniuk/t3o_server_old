#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <iterator>
#include <cstdio>
#include <thread>
#include <chrono>

#include "game_server.hpp"

unsigned count_online_players(const t3o::game_server& server)
{
	unsigned counter = 0;
	for(auto& s : server.sessions()) counter += s->is_logged();
	return counter;
}


int main()
{
	using namespace boost;
	using namespace boost::asio::ip;
	using namespace std::chrono_literals;

	asio::io_service service;
	tcp::endpoint endpoint{tcp::v4(), 6667};
	t3o::game_server server{service, endpoint};
	server.event_session_started() += [&server](auto& session){
		std::cout << "new connection" << std::endl;
		session.event_logged() += [&server, &session]{
			std::cout << "logged as: " << session.name() << std::endl;
			auto players_online = count_online_players(server);
			std::cout << "players online: " << players_online << std::endl;
			if(players_online == 2)
			{
				std::cout << "game has begun" << std::endl;
				unsigned symbol = 1;
				for(auto& s : server.sessions()) if(s->is_logged()){
					s->begin_game(symbol++, 3, 3);				
				}
			}
		};
	};
	server.event_session_ended() += [](auto& session){
		std::cout << "disconnection" << std::endl;
	};
	server.event_user_field_set() += [&server](auto& session, auto x, auto y, auto field){
		if(session.symbol() == field)
		{
			std::cout << x << ", " << y << ", " << field << std::endl;
			for(auto& s : server.sessions()) if(s->is_logged())
			{
				s->async_send_field_set(field, x, y, []{});
			}
		}
	};
	server.start_listen_for_players();
	std::cout << "server has beend started" << std::endl;
	service.run();
}
