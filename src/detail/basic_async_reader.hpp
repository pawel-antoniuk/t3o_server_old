#pragma once
#include <boost/asio.hpp>
#include <queue>
#include <functional>
#include <memory>
#include <array>
#include <cstdint>
#include "operation_queue_base.hpp"
#include "basic_async_reading_operation.hpp"
#include "reading_operation_environment.hpp"
#include "../event.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

template<typename InputSerializer>
class basic_async_reader : public operation_queue_base
{
public:
	basic_async_reader(detail::ip::tcp::socket& socket) :
		_environment{socket,
			boost::asio::buffer(_work_buffer.data(), _work_buffer.size())}
	{
		using namespace std::placeholders;
		auto binder = std::bind(&basic_async_reader<InputSerializer>
				::on_inner_operation_complete, this, _1);
		_environment.inner_operation_complete_handler = binder;
	}

	template<typename... Serializables>
	void async_read(std::function<void(const Serializables&)>... completion_handlers)
	{
		push_operation(
				std::make_unique<basic_async_reading_operation<InputSerializer, Serializables...>>(
					_environment, completion_handlers...));
	}

	auto& event_disconnected()
	{
		return _disconnected_event;
	}

private:
	std::array<uint8_t, 128> _work_buffer;
	reading_operation_environment _environment;

	//events
	event<void()> _disconnected_event;
};

}
}
