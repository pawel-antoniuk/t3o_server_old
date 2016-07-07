#pragma once
#include <boost/asio.hpp>
#include <queue>
#include <functional>
#include <memory>
#include "operation_base.hpp"

namespace t3o
{
namespace detail
{

namespace detail
{
	using namespace boost::asio;
}

class operation_queue_base
{
public:
	virtual ~operation_queue_base(){}

	operation_queue_base() :
		_is_executing(false)
	{
	}

protected:
	void push_operation(std::unique_ptr<operation_base>&& operation)
	{
		_operations.push(std::move(operation));
		_update_queue();
	}
	
	void on_inner_operation_complete(operation_base& operation)
	{
		_operations.pop();
		_is_executing = false;
		_update_queue();
	}

private:
	void _update_queue()
	{
		if(_is_executing) return;
		if(!_operations.empty()) _operations.front()->async_execute();		
	}
	
	std::queue<std::unique_ptr<operation_base>> _operations;
	bool _is_executing;
};

}
}
