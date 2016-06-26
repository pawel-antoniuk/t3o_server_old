#pragma once
#include <cstdint>

namespace t3o
{
	namespace detail
	{
		namespace protocol
		{
			/* HEADER
			[8b] msg id
			[24b] BODY
			*/
			const std::size_t header_size = 1;
			const std::size_t body_size = 3;
			const std::size_t packet_size = header_size + body_size;

			union field_set_packet
			{
				/* BODY
				[8b] x
				[8b] y
				[8b] field
				*/
				struct 
				{
					uint8_t msg_id; //=1;
					uint8_t x;
					uint8_t y;
					uint8_t field;
				} in;
				/* BODY
				[8b] result
				*/
				struct
				{
					uint8_t msg_id; //=1;
					uint8_t result;
					uint16_t _unused;
				} out;
			};
		}
	}
}
