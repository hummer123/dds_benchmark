/*
 * bcli_types.hpp - Common types for bcli benchmark tool
 */

#ifndef BCLI_TYPES_HPP
#define BCLI_TYPES_HPP

#include <cstdint>
#include <cstddef>

// DDS-style entity handle (non-negative = valid, negative = error code)
typedef int32_t handle;

#define HANDLE_INVALID (-1)
#define RET_OK         (0)
#define RET_ERROR      (-1)

// Callback for data reception
typedef void (*DataCallback)(const void* data, size_t len, uint64_t timestamp_ns, void* user_data);

#endif // BCLI_TYPES_HPP
