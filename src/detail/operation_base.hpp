#pragma once
#include <boost/asio.hpp>

namespace t3o
{
namespace detail
{

class operation_base
{
public:
	virtual void async_execute() = 0;
};

}
}
