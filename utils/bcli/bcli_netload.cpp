/*
 * bcli_netload.cpp - Network load / throughput statistics
 */

#include "bcli_netload.hpp"
#include <cmath>

LatencyCalculator::LatencyCalculator() : samples_() {}

void LatencyCalculator::add_sample(int64_t latency_ns) {
    samples_.push_back(latency_ns);
}

LatencyStats LatencyCalculator::compute() const {
    LatencyStats stats = {};
    if (samples_.empty()) return stats;

    std::vector<int64_t> sorted = samples_;
    std::sort(sorted.begin(), sorted.end());

    size_t n = sorted.size();
    double sum = 0;
    for (int64_t s : sorted) {
        sum += s;
    }
    stats.mean_us = sum / n / 1000.0;
    stats.count = n;
    stats.p50_us = sorted[n * 50 / 100] / 1000;
    stats.p90_us = sorted[n * 90 / 100] / 1000;
    stats.p99_us = sorted[n * 99 / 100] / 1000;
    stats.max_us = sorted.back() / 1000;
    return stats;
}

void LatencyCalculator::reset() {
    samples_.clear();
}

ThroughputCalculator::ThroughputCalculator(size_t payload_size)
    : payload_size_(payload_size), expected_seq_(1),
      total_received_(0), total_lost_(0), window_start_sec_(0), window_count_(0) {}

void ThroughputCalculator::add_received(uint64_t seq) {
    if (expected_seq_ == 0) {
        expected_seq_ = seq;
    }
    if (seq > expected_seq_) {
        total_lost_ += (seq - expected_seq_);
    }
    expected_seq_ = seq + 1;
    total_received_++;
}

ThroughputStats ThroughputCalculator::compute() const {
    ThroughputStats stats = {};
    stats.total_msgs = total_received_;
    stats.lost_msgs = total_lost_;
    stats.loss_rate = (total_received_ + total_lost_) > 0 ?
        (100.0 * total_lost_) / (total_received_ + total_lost_) : 0.0;
    stats.rate_msgl = 0;
    stats.bandwidth_mbps = 0;
    return stats;
}

void ThroughputCalculator::reset() {
    expected_seq_ = 1;
    total_received_ = 0;
    total_lost_ = 0;
    window_start_sec_ = 0;
    window_count_ = 0;
}
