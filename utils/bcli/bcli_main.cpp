/*
 * bcli_main.cpp - bcli benchmark tool main entry
 *
 * Usage:
 *   bcli pub <rate> <size>    - Publish mode
 *   bcli sub                  - Subscribe mode
 *   bcli ping [rate]          - Latency test (sender)
 *   bcli pong                 - Latency test (receiver)
 *
 * Options:
 *   -i <id>     Domain ID (default: 0)
 *   -t <sec>    Run duration (default: 0 = infinite)
 *   -n <N>      Key instance count (default: 1)
 *   -b          Best-effort mode (default: reliable)
 *   -c          Enable CPU/memory monitoring
 *   -h          Show help
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <chrono>
#include <thread>
#include <csignal>
#include <getopt.h>
#include <unistd.h>

#include "test_data.hpp"
#include "bcli_netload.hpp"
#include "bcli_cputime.hpp"
#include "bcli_transport.hpp"
#include "bcli_types.hpp"
#include "adapters/adapter_cyclonedds.hpp"

static void print_help(const char* prog) {
    fprintf(stderr,
        "Usage: %s <command> [options]\n"
        "\n"
        "Commands:\n"
        "  pub <rate> <size>    Publish mode\n"
        "  sub                  Subscribe mode\n"
        "  ping [rate]          Latency test (sender)\n"
        "  pong                 Latency test (receiver)\n"
        "\n"
        "Options:\n"
        "  -i <id>     Domain ID (default: 0)\n"
        "  -t <sec>    Run duration (default: 0 = infinite)\n"
        "  -n <N>      Key instance count (default: 1)\n"
        "  -b          Best-effort mode (default: reliable)\n"
        "  -c          Enable CPU/memory monitoring\n"
        "  -h          Show this help\n"
        "\n"
        "Examples:\n"
        "  %s pub 1000 256      # Publish at 1kHz with 256B payload\n"
        "  %s sub -c            # Subscribe with CPU monitoring\n"
        "  %s ping             # Latency test at max rate\n"
        "  %s pong             # Latency responder\n",
        prog, prog, prog, prog, prog);
}

enum class Command {
    NONE,
    PUB,
    SUB,
    PING,
    PONG
};

struct Options {
    Command cmd = Command::NONE;
    int domain_id = 0;
    int run_time = 0;       // 0 = infinite
    int key_count = 1;
    int rate = 0;           // 0 = as fast as possible
    size_t payload_size = 256;
    bool best_effort = false;
    bool monitor = false;
};

static std::atomic<bool> g_running(true);

static void signal_handler(int) {
    g_running.store(false);
}

static bool parse_args(int argc, char* argv[], Options& opts) {
    if (argc < 2) {
        return false;
    }

    // Parse command
    if (strcmp(argv[1], "pub") == 0) {
        opts.cmd = Command::PUB;
    } else if (strcmp(argv[1], "sub") == 0) {
        opts.cmd = Command::SUB;
    } else if (strcmp(argv[1], "ping") == 0) {
        opts.cmd = Command::PING;
    } else if (strcmp(argv[1], "pong") == 0) {
        opts.cmd = Command::PONG;
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        exit(0);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return false;
    }

    // Parse options
    int opt;
    optind = 1;
    while ((opt = getopt(argc - 1, argv + 1, "i:t:n:cbh")) != -1) {
        switch (opt) {
            case 'i':
                opts.domain_id = atoi(optarg);
                break;
            case 't':
                opts.run_time = atoi(optarg);
                break;
            case 'n':
                opts.key_count = atoi(optarg);
                break;
            case 'c':
                opts.monitor = true;
                break;
            case 'b':
                opts.best_effort = true;
                break;
            case 'h':
                print_help(argv[0]);
                exit(0);
            default:
                return false;
        }
    }

    return true;
}

static void run_publisher(const Options& opts) {
    CycloneTransport transport;
    BcliMonitor monitor;
    BcliSnapshot snap;

    handle participant = transport.create_participant(opts.domain_id);
    if (participant == HANDLE_INVALID) {
        fprintf(stderr, "Failed to create participant\n");
        return;
    }

    handle topic = transport.create_topic(participant, "BM_TalkData", "bm_test::TestData");
    if (topic == HANDLE_INVALID) {
        fprintf(stderr, "Failed to create topic\n");
        return;
    }

    handle writer = transport.create_writer(topic);
    if (writer == HANDLE_INVALID) {
        fprintf(stderr, "Failed to create writer\n");
        return;
    }

    // Wait for subscriber
    fprintf(stderr, "Waiting for subscriber...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    bm_test::TestData data;
    uint64_t seq = 0;
    auto start = std::chrono::steady_clock::now();
    auto interval = opts.rate > 0 ?
        std::chrono::milliseconds(1000 / opts.rate) : std::chrono::milliseconds(0);

    while (g_running.load()) {
        auto loop_start = std::chrono::steady_clock::now();

        data.seq(++seq);
        data.timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        data.baggage("test");

        transport.write(writer, &data, sizeof(data));

        if (opts.monitor && (seq % 100) == 0) {
            if (monitor.snapshot(snap)) {
                printf("CPU: ");
                monitor.print(snap, "");
            }
        }

        if (opts.rate > 0) {
            auto elapsed = std::chrono::steady_clock::now() - loop_start;
            if (elapsed < interval) {
                std::this_thread::sleep_for(interval - elapsed);
            }
        }

        if (opts.run_time > 0) {
            auto total = std::chrono::steady_clock::now() - start;
            if (std::chrono::duration_cast<std::chrono::seconds>(total).count() >= opts.run_time) {
                break;
            }
        }
    }

    printf("Sent %lu messages\n", seq);
}

static void run_subscriber(const Options& opts) {
    (void)opts;
    fprintf(stderr, "Subscriber not yet implemented\n");
}

static void run_ping(const Options& opts) {
    (void)opts;
    fprintf(stderr, "Ping not yet implemented\n");
}

static void run_pong(const Options& opts) {
    (void)opts;
    fprintf(stderr, "Pong not yet implemented\n");
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_args(argc, argv, opts)) {
        print_help(argv[0]);
        return 1;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    switch (opts.cmd) {
        case Command::PUB:
            run_publisher(opts);
            break;
        case Command::SUB:
            run_subscriber(opts);
            break;
        case Command::PING:
            run_ping(opts);
            break;
        case Command::PONG:
            run_pong(opts);
            break;
        default:
            fprintf(stderr, "No command specified\n");
            print_help(argv[0]);
            return 1;
    }

    return 0;
}
