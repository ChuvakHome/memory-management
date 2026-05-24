
#ifndef SAFE_READ_HPP
#define SAFE_READ_HPP

#include <cstdint>
#include <optional>

std::optional<std::uint8_t> safe_read_uint8(const std::uint8_t *p);

#endif
