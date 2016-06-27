#include <boost/asio.hpp>
#include <functional>
#include <vector>

#include "game_session.hpp"

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
			explicit game_server(detail::io_service& io_service) : 
				_io_service{io_service}, 
				_acceptor{_io_service, detail::tcp::endpoint{detail::tcp::v4(), 6667}}
			{
			}

			void run(detail::function<void(detail::tcp::socket&,
						detail::tcp::endpoint&)> callback) 
			{
				using namespace detail;
				
				for(;;)
				{
					tcp::socket socket{_io_service};
					tcp::endpoint endpoint;
					_acceptor.accept(socket, endpoint);
					callback(socket, endpoint);	
				}
			}

			void set_field(unsigned x, unsigned y, unsigned field)
			{
				for(auto& session : _sessions)
				{
					session.async_send_field_set(x, y, field);
				}
			}
			
		private:
			detail::io_service& _io_service;
			detail::tcp::acceptor _acceptor;
			detail::vector<game_session> _sessions;
	};
}

