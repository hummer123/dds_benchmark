/*
 * bcli_cputime.hpp - CPU and memory monitoring
 *
 * Lightweight monitoring using /proc filesystem.
 */

#ifndef BCLI_CPUTIME_HPP
#define BCLI_CPUTIME_HPP

#include <cstdint>
#include <cstddef>

constexpr size_t BCLI_MAX_THREADS = 64;

struct BcliThreadStat {
    uint64_t tid;
    char name[32];
    double user_sec;
    double sys_sec;
};

struct BcliSnapshot {
    uint64_t timestamp_ns;
    uint64_t vcsw;
    uint64_t ivcsw;
    uint64_t rss_kb;
    uint32_t num_threads;
    BcliThreadStat threads[BCLI_MAX_THREADS];
};

class BcliMonitor {
public:
    BcliMonitor();
    ~BcliMonitor();

    bool snapshot(BcliSnapshot& out);
    void print(const BcliSnapshot& s, const char* prefix = "");

private:
    static uint64_t get_timestamp_ns();
    static void read_status(uint64_t& vcsw, uint64_t& ivcsw, uint64_t& rss_kb);
    static size_t get_thread_tids(uint64_t tids[], size_t max);
    static bool read_thread_stat(uint64_t tid, double& utime, double& stime);
    static void read_thread_name(uint64_t tid, char* name, size_t maxlen);

    uint64_t prev_vcsw_ = 0;
    uint64_t prev_ivcsw_ = 0;
    uint64_t prev_timestamp_ns_ = 0;
    BcliThreadStat prev_threads_[BCLI_MAX_THREADS];
    size_t prev_thread_count_ = 0;
};

#endif  // BCLI_CPUTIME_HPP
