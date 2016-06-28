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
				uint8_t x, y, field;

				template<typename Archive> 
				void serialize(Archive& ar, const unsigned) 
				{ 
					ar & x & y & field;
				}

				static const uint8_t packet_id = 1;
			};
		}
	}
}
