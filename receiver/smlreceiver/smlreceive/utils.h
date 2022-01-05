#pragma once

#include <cstdint>
#include "ReadMessage.h"

std::uint16_t crc16_ccitt(const std::uint8_t* data, size_t length);
std::uint16_t calculate_crc(const Message& msg);
