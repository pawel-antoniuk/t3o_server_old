#include "event.hpp"

struct foo
{
	auto& event_xxx()
	{
		return _xxx_event;
	}

	void zrob_cos()
	{
		_xxx_event(2);
	}

	private:
		t3o::event<void(int)> _xxx_event;
};

void test()
{

	foo f;
	f.event_xxx() += [](int i) {};
	f.zrob_cos();
}
