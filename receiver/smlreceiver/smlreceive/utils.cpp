#include "utils.h"


std::uint16_t crc16_ccitt(const std::uint8_t* data, size_t length)
{
	uint16_t crc;
	uint32_t c;
	uint8_t i;

	uint8_t byte;
	uint8_t crcbit;
	uint8_t databit;

	crc = 0xFFFF;
	for (c = 0; c < length; c++) {
		byte = data[c];
		for (i = 0; i < 8; i++) {
			crcbit = (crc & 0x8000) ? 1 : 0;
			databit = (byte & 0x80) ? 1 : 0;
			crc = (uint16_t)(crc << 1);
			byte = (uint8_t)(byte << 1);
			if (crcbit != databit) {
				crc = crc ^ 0x1021;
			}
		}
	}

	return crc;
}

std::uint16_t calculate_crc(const Message& msg)
{
	if (msg.size < 2)
	{
		return 0;
	}

	return crc16_ccitt(msg.data, msg.size - 2);
}