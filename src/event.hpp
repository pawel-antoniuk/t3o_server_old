#pragma once
#include <vector>
#include <functional>
#include <utility>

namespace t3o
{
	template<typename T>
	class event;

	template<typename ...Args, typename Ret>
	class event<Ret(Args...)>
	{
	public:
		using callable_t = std::function<Ret(Args...)>;

		event& operator+=(const callable_t& e)
		{
			_callables.push_back(e);
			return *this;
		}

		event& operator()(Args... args) 
		{
			for (auto& e : _callables) e(std::forward<Args>(args)...);
			return *this;
		}

	private:
		std::vector<callable_t> _callables;
	};
}
