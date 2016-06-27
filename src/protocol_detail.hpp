#pragma once
#include <cstdint>

namespace t3o
{
	namespace detail
	{
		namespace protocol
		{
			struct field_set_packet_t
			{
				static const unsigned id = 1;

				uint8_t x, y, field;

				template<typename Archive> 
				void serialize(Archive& ar, const unsigned) 
				{ 
					ar & x & y & field;
				}
			};
		}
	}
}
