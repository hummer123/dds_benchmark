#include <cstdlib>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

/* Include the C++ DDS API. */
#include "dds/dds.hpp"

/* Include data type and specific traits to be used with the C++ DDS API. */
#include "test_data.hpp"

using namespace org::eclipse::cyclonedds;


/* Signal handler for graceful shutdown */
std::atomic_bool g_running(true);
void signal_handler(int)
{
    g_running.store(false);
}



class TalkListener : public virtual dds::sub::NoOpDataReaderListener<bm_test::TestData>
{
public:
    TalkListener() = default;

    virtual void on_data_available(dds::sub::DataReader<bm_test::TestData>& reader) override
    {
        try {
            dds::sub::LoanedSamples<bm_test::TestData> samples = reader.take();
            
            for (const auto& sample : samples) {
                if (sample.info().valid()) {
                    const bm_test::TestData& data = sample.data();

                    std::cout << "=== [Subscriber] Received seq:" << data.seq() << ", timestamp:" << data.timestamp() << ", baggage:" << data.baggage() << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "=== [Subscriber] Exception in on_data_available: " << e.what() << std::endl;
        }
    }
};


int main()
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {        
        dds::domain::DomainParticipant participant(0);
        dds::topic::Topic<bm_test::TestData> topic(participant, "BM_TalkData");
        dds::sub::Subscriber subscriber(participant);
        
        TalkListener listener;
        dds::sub::DataReader<bm_test::TestData> reader(
            subscriber, 
            topic,
            subscriber.default_datareader_qos(),
            &listener,
            dds::core::status::StatusMask::data_available());

        std::cout << "=== [Subscriber] Waiting for data..." << std::endl;

        /* Keep the subscriber running until a shutdown signal is received */
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "=== [Subscriber] Shutting down gracefully." << std::endl;
    } catch (const dds::core::Exception& e) {
        std::cerr << "=== [Subscriber] DDS Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "=== [Subscriber] C++ Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "=== [Subscriber] Done." << std::endl;
    
    return EXIT_SUCCESS;
}