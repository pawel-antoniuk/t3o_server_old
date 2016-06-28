#pragma once
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <vector>
#include <thread>
#include <functional>
#include <utility>
#include <algorithm>

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
		using session_ptr = std::shared_ptr<game_session>;
	}

	class game_server 
	{
		public:
			explicit game_server(detail::io_service& io_service, detail::tcp::endpoint ep) : 
				_io_service{io_service}, 
				_acceptor{_io_service, ep}
			{
				_create_session();

			}

			void set_field(unsigned x, unsigned y, unsigned field)
			{
				for(auto& session : _sessions)
				{
					session->async_send_field_set(x, y, field);
				}
			}

			//events
			auto& event_session_started()
			{
				return _session_started_event;
			}

			auto& event_session_ended()
			{
				return _session_ended_event;
			}

			auto& event_user_field_set()
			{
				return _user_field_set_event;
			}
			
		private:

			void _acceptor_proc(detail::session_ptr& session, 
					const boost::system::error_code& code)
			{
				//std::cout << "new client " << _sessions.size() << std::endl;
				using namespace std::placeholders;
				session->event_field_set() += _user_field_set_event.make_handler(_1, _2, _3);
				session->event_disconnected() += 
					std::bind(&game_server::_client_disconnected, this, session);
				_session_started_event(*session);
				session->async_run();
				_create_session();
			}

			void _create_session()
			{
				auto session = std::make_shared<game_session>(_io_service);
				_sessions.push_back(session);
				using namespace std::placeholders;
				_acceptor.async_accept(session->socket(), 
						std::bind(&game_server::_acceptor_proc, this, session, _1));
				//return session;
			}

			void _client_disconnected(detail::session_ptr& session)
			{
				//std::cout << "client disconeccted" << _sessions.size() << std::endl;
				auto it = std::find(std::begin(_sessions), std::end(_sessions), session);
				if(it != std::end(_sessions)){
					_sessions.erase(it);
					_session_ended_event(*session);
				}
			}

			detail::io_service& _io_service;
			detail::tcp::acceptor _acceptor;
			detail::vector<detail::session_ptr> _sessions;

			//events
			event<void(game_session&)> _session_started_event;
			event<void(game_session&)> _session_ended_event;
			event<void(unsigned, unsigned, unsigned)> _user_field_set_event;
	};
}

