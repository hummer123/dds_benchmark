#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

/* Include the C++ DDS API. */
#include "dds/dds.hpp"

/* Include data type and specific traits to be used with the C++ DDS API. */
#include "test_data.hpp"

using namespace org::eclipse::cyclonedds;


int main()
{
    try {
        dds::domain::DomainParticipant participant(0);
        dds::topic::Topic<bm_test::TestData> topic(participant, "BM_TalkData");
        dds::pub::Publisher publisher(participant);
        dds::pub::DataWriter<bm_test::TestData> writer(publisher, topic);

        std::cout << "=== [Publisher] Publish Msg Construct." << std::endl;
        bm_test::TestData data;
        std::string seedMsg;
        
        uint64_t seq = 1;
        auto publish_once = [&](size_t subscriber_count, const char* msg) {
            data.seq(static_cast<int32_t>(seq++));
            data.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

            seedMsg = "[All Known] Subscriber count:" + std::to_string(subscriber_count) + ", msg:" + msg;
            data.baggage(seedMsg);

            writer.write(data);
            std::cout << "=== [Publisher] Sent seq=" << data.seq() << " to " << seedMsg << std::endl;
        };

        /* Wait for the first subscriber to connect. */
        std::cout << "=== [Publisher] Waiting for first subscriber..." << std::endl;
        while (writer.publication_matched_status().current_count() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        size_t last_count = 0;
        size_t current_count = 0;
        
        while (true) {
            current_count = writer.publication_matched_status().current_count();

            if (current_count == 0 ) {
                std::cout << "=== [Publisher] No subscribers connected. Break..." << std::endl;
                break; 
            } else if (current_count != last_count) {
                publish_once(current_count, "Subscriber count changed");
                last_count = current_count;
            } 
            else {}
            
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    } catch (const dds::core::Exception& e) {
        std::cerr << "=== [Publisher] DDS Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "=== [Publisher] Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "=== [Publisher] Done." << std::endl;

    return EXIT_SUCCESS;
}

