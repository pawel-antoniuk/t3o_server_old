#pragma once
#include <vector>
#include <functional>
#include <utility>

namespace t3o
{
	template<typename T>
	class event;

	template<typename ...Args>
	class event<void(Args...)>
	{
	public:
		using callable_t = std::function<void(Args...)>;

		event& operator+=(const callable_t& e)
		{
			_callables.push_back(e);
			return *this;
		}

		void operator()(Args... args) 
		{
			for (auto& e : _callables) e(std::forward<Args>(args)...);
		}

		template<typename ...HandlerArgs>
		auto make_handler(HandlerArgs... args)
		{
			return std::bind(&event::operator(), this, std::forward<HandlerArgs>(args)...);
		}

	private:
		std::vector<callable_t> _callables;
	};
}
