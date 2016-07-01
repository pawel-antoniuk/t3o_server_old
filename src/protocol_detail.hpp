#pragma once
#include <cstdint>

namespace t3o
{
	namespace detail
	{
		namespace protocol
		{
			struct handshake_t
			{
				uint8_t width, height, field;

				template<typename Archive>
				void serialize(Archive& ar, const unsigned)
				{
					ar & width & height & field;
				}

				static const uint8_t packet_id = 1;
			};

			struct feedback_t
			{
				uint8_t result;

				template<typename Archive>
				void serialize(Archive& ar, const unsigned)
				{
					ar & result;
				}

				static const uint8_t packet_id = 2;
			};

			struct field_set_packet_t
			{
				uint8_t x, y, field;

				template<typename Archive> 
				void serialize(Archive& ar, const unsigned) 
				{ 
					ar & x & y & field;
				}

				static const uint8_t packet_id = 3;
			};

		}
	}
}
