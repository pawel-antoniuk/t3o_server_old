#include <boost/asio.hpp>
#include <functional>

namespace t3o
{
	namespace detail
	{
		using namespace boost::asio;
		using boost::asio::ip::tcp;
		using std::function;
	}

	class game_server 
	{
		public:
			game_server(detail::io_service& io_service) : 
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
			
		private:
			detail::io_service& _io_service;
			detail::tcp::acceptor _acceptor;
	};
}

