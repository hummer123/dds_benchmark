/*
 * adapter_cyclonedds.hpp - CycloneDDS transport adapter
 */

#ifndef ADAPTER_CYCLONEDDS_HPP
#define ADAPTER_CYCLONEDDS_HPP

#include "bcli_transport.hpp"
#include "bcli_types.hpp"
#include <map>
#include <memory>
#include <mutex>

// Forward declarations instead of including dds/dds.hpp
namespace org { namespace eclipse { namespace cyclonedds {
    template<typename T>
    class DomainParticipant;
    template<typename T>
    class Topic;
    namespace pub {
        template<typename T>
        class Publisher;
        template<typename T>
        class DataWriter;
    }
    namespace sub {
        template<typename T>
        class Subscriber;
        template<typename T>
        class DataReader;
    }
}}}

class CycloneTransport : public ITransport {
public:
    CycloneTransport();
    ~CycloneTransport() override;

    handle create_participant(int domain_id) override;
    handle create_topic(handle participant, const char* name, const char* type_name) override;
    handle create_writer(handle topic) override;
    handle create_reader(handle topic, DataCallback cb, void* user_data) override;

    int write(handle writer, const void* data, size_t len) override;
    uint64_t get_timestamp_ns() override;

    void delete_entity(handle entity) override;

private:
    struct Impl;
    Impl* impl_;
    int32_t next_id_;
};

#endif // ADAPTER_CYCLONEDDS_HPP
