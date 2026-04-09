/*
 * bcli_netload.hpp - Network load / throughput statistics
 */

#ifndef BCLI_NETLOAD_HPP
#define BCLI_NETLOAD_HPP

#include <cstdint>
#include <vector>
#include <algorithm>

struct LatencyStats {
    double mean_us;      // average latency (microseconds)
    int64_t p50_us;      // 50th percentile
    int64_t p90_us;      // 90th percentile
    int64_t p99_us;      // 99th percentile
    int64_t max_us;      // maximum latency
    uint64_t count;      // total sample count
};

struct ThroughputStats {
    double rate_msgl;    // messages per second
    double bandwidth_mbps; // megabits per second
    uint64_t total_msgs;  // total messages received
    uint64_t lost_msgs;   // lost messages detected
    double loss_rate;     // loss percentage
};

class LatencyCalculator {
public:
    LatencyCalculator();
    void add_sample(int64_t latency_ns);
    LatencyStats compute() const;
    void reset();

private:
    std::vector<int64_t> samples_;
};

class ThroughputCalculator {
public:
    ThroughputCalculator(size_t payload_size);
    void add_received(uint64_t seq);
    ThroughputStats compute() const;
    void reset();

private:
    size_t payload_size_;
    uint64_t expected_seq_;
    uint64_t total_received_;
    uint64_t total_lost_;
    double window_start_sec_;
    uint64_t window_count_;
};

#endif // BCLI_NETLOAD_HPP
