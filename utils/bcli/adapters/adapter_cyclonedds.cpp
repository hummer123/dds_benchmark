/*
 * adapter_cyclonedds.cpp - CycloneDDS transport adapter implementation
 *
 * Note: CycloneDDS DataWriter/DataReader cannot be heap-allocated (private operator new).
 * They must be used as stack variables. This adapter stores them directly in the Impl struct.
 */

#include "adapter_cyclonedds.hpp"
#include <dds/dds.hpp>
#include <stdexcept>
#include <chrono>

// Use existing bm_test::TestData from msg/ directory
#include "test_data.hpp"

using namespace org::eclipse::cyclonedds;

struct CycloneTransport::Impl {
    dds::domain::DomainParticipant participant;
    dds::topic::Topic<bm_test::TestData> topic;
    dds::pub::Publisher publisher;
    dds::sub::Subscriber subscriber;
    dds::pub::DataWriter<bm_test::TestData> writer;
    dds::sub::DataReader<bm_test::TestData> reader;
    bool writer_created = false;
    bool reader_created = false;

    Impl(int domain_id, const char* topic_name)
        : participant(domain_id),
          topic(participant, topic_name),
          publisher(participant),
          subscriber(participant),
          writer(publisher, topic),
          reader(subscriber, topic) {
    }
};

CycloneTransport::CycloneTransport() : impl_(nullptr), next_id_(1) {}

CycloneTransport::~CycloneTransport() {
    delete impl_;
}

handle CycloneTransport::create_participant(int domain_id) {
    (void)domain_id;
    return next_id_++;
}

handle CycloneTransport::create_topic(handle participant, const char* name, const char* type_name) {
    (void)participant;
    (void)type_name;
    if (!impl_) {
        impl_ = new CycloneTransport::Impl(0, name);
    }
    return next_id_++;
}

handle CycloneTransport::create_writer(handle topic) {
    (void)topic;
    if (!impl_) return HANDLE_INVALID;
    impl_->writer_created = true;
    return 1;  // Single writer, return handle 1
}

handle CycloneTransport::create_reader(handle topic, DataCallback cb, void* user_data) {
    (void)topic;
    (void)cb;
    (void)user_data;
    if (!impl_) return HANDLE_INVALID;
    impl_->reader_created = true;
    return 1;  // Single reader, return handle 1
}

int CycloneTransport::write(handle writer, const void* data, size_t len) {
    (void)len;
    (void)writer;
    if (!impl_ || !impl_->writer_created) return RET_ERROR;

    try {
        const bm_test::TestData* pd = static_cast<const bm_test::TestData*>(data);
        impl_->writer.write(*pd);
        return RET_OK;
    } catch (const std::exception& e) {
        (void)e;
        return RET_ERROR;
    }
}

uint64_t CycloneTransport::get_timestamp_ns() {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
}

void CycloneTransport::delete_entity(handle entity) {
    (void)entity;
    // Writer and reader are cleaned up with Impl
}
