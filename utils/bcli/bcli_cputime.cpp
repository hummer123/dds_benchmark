/*
 * bcli_cputime.cpp - CPU and memory monitoring implementation
 */

#include "bcli_cputime.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <dirent.h>
#include <ctype.h>

uint64_t BcliMonitor::get_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
}

void BcliMonitor::read_status(uint64_t& vcsw, uint64_t& ivcsw, uint64_t& rss_kb) {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return;
    char line[256];
    vcsw = ivcsw = 0;
    rss_kb = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "voluntary_ctxt_switches:", 23) == 0) {
            sscanf(line + 23, "%lu", &vcsw);
        } else if (strncmp(line, "nonvoluntary_ctxt_switches:", 26) == 0) {
            sscanf(line + 26, "%lu", &ivcsw);
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%lu", &rss_kb);
        }
    }
    fclose(f);
}

size_t BcliMonitor::get_thread_tids(uint64_t tids[], size_t max) {
    DIR* dir = opendir("/proc/self/task");
    if (!dir) return 0;
    size_t n = 0;
    struct dirent* ent;
    while ((ent = readdir(dir)) && n < max) {
        if (isdigit(ent->d_name[0])) {
            tids[n++] = atoll(ent->d_name);
        }
    }
    closedir(dir);
    return n;
}

bool BcliMonitor::read_thread_stat(uint64_t tid, double& utime, double& stime) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/task/%lu/stat", tid);
    FILE* f = fopen(path, "r");
    if (!f) return false;
    char buf[1024];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return false;
    }
    fclose(f);
    char* p = strrchr(buf, ')');
    if (!p) return false;
    p++;
    int field = 0;
    char* saveptr;
    char* token = strtok_r(p, " ", &saveptr);
    while (token) {
        field++;
        if (field == 12) {
            utime = atof(token) / static_cast<double>(sysconf(_SC_CLK_TCK));
        } else if (field == 13) {
            stime = atof(token) / static_cast<double>(sysconf(_SC_CLK_TCK));
            break;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }
    return true;
}

void BcliMonitor::read_thread_name(uint64_t tid, char* name, size_t maxlen) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/task/%lu/comm", tid);
    FILE* f = fopen(path, "r");
    if (!f) {
        snprintf(name, maxlen, "tid:%lu", tid);
        return;
    }
    size_t len = fread(name, 1, maxlen - 1, f);
    name[len] = '\0';
    fclose(f);
    size_t i = len;
    while (i > 0 && (name[i-1] == '\n' || name[i-1] == '\r')) {
        name[--i] = '\0';
    }
}

BcliMonitor::BcliMonitor() {
    memset(prev_threads_, 0, sizeof(prev_threads_));
}

BcliMonitor::~BcliMonitor() {
}

bool BcliMonitor::snapshot(BcliSnapshot& out) {
    uint64_t now_ns = get_timestamp_ns();
    uint64_t vcsw, ivcsw, rss_kb;
    read_status(vcsw, ivcsw, rss_kb);

    uint64_t tids[BCLI_MAX_THREADS];
    size_t nthreads = get_thread_tids(tids, BCLI_MAX_THREADS);

    bool any_above = false;

    out.timestamp_ns = now_ns;
    out.vcsw = (vcsw > prev_vcsw_) ? vcsw - prev_vcsw_ : 0;
    out.ivcsw = (ivcsw > prev_ivcsw_) ? ivcsw - prev_ivcsw_ : 0;
    out.rss_kb = rss_kb;
    out.num_threads = static_cast<uint32_t>(nthreads);

    double dt = 0.0;
    if (prev_timestamp_ns_ > 0) {
        dt = static_cast<double>(now_ns - prev_timestamp_ns_) / 1e9;
    }

    BcliThreadStat current[BCLI_MAX_THREADS];
    for (size_t i = 0; i < nthreads; i++) {
        current[i].tid = tids[i];
        read_thread_name(tids[i], current[i].name, sizeof(current[i].name));
        double utime = 0, stime = 0;
        if (read_thread_stat(tids[i], utime, stime)) {
            current[i].user_sec = utime;
            current[i].sys_sec = stime;
        } else {
            current[i].user_sec = current[i].sys_sec = 0;
        }
    }

    for (size_t i = 0; i < nthreads && i < BCLI_MAX_THREADS; i++) {
        BcliThreadStat* prev = nullptr;
        for (size_t j = 0; j < prev_thread_count_; j++) {
            if (prev_threads_[j].tid == current[i].tid) {
                prev = &prev_threads_[j];
                break;
            }
        }

        BcliThreadStat* thr = &out.threads[i];
        thr->tid = current[i].tid;
        strncpy(thr->name, current[i].name, sizeof(thr->name) - 1);
        thr->name[sizeof(thr->name) - 1] = '\0';

        if (prev && dt > 0) {
            double du = (current[i].user_sec - prev->user_sec) / dt;
            double ds = (current[i].sys_sec - prev->sys_sec) / dt;
            thr->user_sec = du;
            thr->sys_sec = ds;
            if (du >= 0.005 || ds >= 0.005) {
                any_above = true;
            }
        } else {
            thr->user_sec = thr->sys_sec = 0;
        }

        memcpy(&prev_threads_[i], &current[i], sizeof(BcliThreadStat));
    }
    out.num_threads = static_cast<uint32_t>(nthreads);

    prev_vcsw_ = vcsw;
    prev_ivcsw_ = ivcsw;
    prev_timestamp_ns_ = now_ns;
    prev_thread_count_ = nthreads;

    return any_above;
}

void BcliMonitor::print(const BcliSnapshot& s, const char* prefix) {
    printf("%s vcsw:%lu ivcsw:%lu rss:%luKB",
           prefix, s.vcsw, s.ivcsw, s.rss_kb);
    for (uint32_t i = 0; i < s.num_threads; i++) {
        const auto& thr = s.threads[i];
        if (thr.user_sec >= 0.005 || thr.sys_sec >= 0.005) {
            printf(" %s:%.0f%%+%.0f%%",
                   thr.name, thr.user_sec * 100.0, thr.sys_sec * 100.0);
        }
    }
    printf("\n");
}
