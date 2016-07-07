#pragma once
#include <boost/asio.hpp>
#include <queue>
#include <functional>
#include <memory>
#include <array>
#include <cstdint>
#include "basic_async_writing_operation.hpp"
#include "operation_queue_base.hpp"
#include "operation_environment.hpp"
#include "../event.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

template<typename OutputSerializer>
class basic_async_writer : public operation_queue_base
{
public:
	basic_async_writer(detail::ip::tcp::socket& socket) :
		_environment{socket, 
			boost::asio::buffer(_work_buffer.data(), _work_buffer.size())}
	{
		using namespace std::placeholders;
		auto binder = std::bind(&basic_async_writer<OutputSerializer>
				::on_inner_operation_complete, this, _1);
		_environment.inner_operation_complete_handler = binder;
	}

	template<typename Serializable>
	void async_write(const Serializable& object, std::function<void()> completion_handler)
	{
		push_operation(std::make_unique<
				basic_async_writing_operation<OutputSerializer,	Serializable>>(
					_environment, completion_handler, object));
	}

	auto& event_disconnected()
	{
		return _disconnected_event;
	}

private:
	std::array<uint8_t, 128> _work_buffer;
	operation_environment _environment;

	//events
	event<void()> _disconnected_event;
};

}
}
