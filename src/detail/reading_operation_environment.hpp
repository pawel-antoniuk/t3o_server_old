#pragma once
#include <boost/asio.hpp>
#include <functional>
#include "operation_base.hpp"

namespace t3o
{
namespace detail
{

struct reading_operation_environment
{
	boost::asio::ip::tcp::socket& socket;
	boost::asio::mutable_buffers_1 work_buffer;	
	std::function<void(operation_base&)> inner_operation_complete_handler;
	std::function<void()> disconnected_handler;
};

}
}
