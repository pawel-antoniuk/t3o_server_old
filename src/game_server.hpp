#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <vector>

#include "game_session.hpp"
#include "event.hpp"

namespace t3o
{
	namespace detail
	{
		using namespace boost::asio;
		using boost::asio::ip::tcp;
		using std::function;
		using std::vector;
	}

	class game_server 
	{
		public:
			explicit game_server(detail::io_service& io_service, detail::tcp::endpoint ep) : 
				_io_service{io_service}, 
				_acceptor{_io_service, ep}
			{
			}

			void run()
			{
				for(;;)
				{
					detail::tcp::socket socket{_io_service};
					detail::tcp::endpoint endpoint;
					_acceptor.accept(socket, endpoint);
					_sessions.emplace_back(std::move(socket));
				}
			}

			void set_field(unsigned x, unsigned y, unsigned field)
			{
				for(auto& session : _sessions)
				{
					session.async_send_field_set(x, y, field);
				}
			}

			//events
			auto& event_session_started()
			{
				return _session_started_event;
			}

			auto& event_user_field_set()
			{
				return _user_field_set_event;
			}
			
		private:
			detail::io_service& _io_service;
			detail::tcp::acceptor _acceptor;
			detail::vector<game_session> _sessions;

			//events
			event<void(game_session&)> _session_started_event;
			event<void(unsigned, unsigned, unsigned)> _user_field_set_event;
	};
}

