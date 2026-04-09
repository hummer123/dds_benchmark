/*
 * bcli_transport.hpp - Transport layer interface for bcli
 */

#ifndef BCLI_TRANSPORT_HPP
#define BCLI_TRANSPORT_HPP

#include "bcli_types.hpp"
#include <cstddef>

class ITransport {
public:
    virtual ~ITransport() = default;

    // Entity lifecycle
    virtual handle create_participant(int domain_id) = 0;
    virtual handle create_topic(handle participant, const char* name, const char* type_name) = 0;
    virtual handle create_writer(handle topic) = 0;
    virtual handle create_reader(handle topic, DataCallback cb, void* user_data) = 0;

    // Operations
    virtual int write(handle writer, const void* data, size_t len) = 0;
    virtual uint64_t get_timestamp_ns() = 0;

    // Cleanup
    virtual void delete_entity(handle entity) = 0;
};

#endif // BCLI_TRANSPORT_HPP
