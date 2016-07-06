#pragma once
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <tuple>
#include <array>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "operation_base.hpp"
#include "writing_operation_environment.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

template<typename InputSerializer, typename Serializable>
class basic_async_writing_operation : 
	public operation_base
{
public:
	basic_async_writing_operation(
			writing_operation_environment& environment,
			std::function<void()> completion_handler,
			const Serializable& object
			) :
		_environment(environment),
		_completion_handler(completion_handler),
		_object(object)
	{
	}

	void async_execute() override
	{
		
	}

private:
	writing_operation_environment& _environment;
	std::function<void()> _completion_handler;
	Serializable _object;
};

}
}
