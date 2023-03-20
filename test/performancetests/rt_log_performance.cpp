/**
 * @brief Example of how to use the elklogger directly
 */

#include <string>
#include <iostream>
#include <vector>
#include <numeric>
#include <atomic>

#include "elklog/elk_logger.h"
#include "../twine/src/twine_internal.h"

constexpr int ITERATIONS = 10000;
constexpr int WORKERS = 4;
constexpr auto SLEEP_TIME = std::chrono::milliseconds(50);

std::atomic<int> thread_counter = 0;


void logger_worker(elklog::ElkLogger* logger, std::vector<std::chrono::nanoseconds>* times, int iterations)
{
    times->reserve(iterations);
    twine::ThreadRtFlag rt_flag;
    int thread_id = thread_counter.fetch_add(1);

    while (iterations > 0)
    {
        int one = std::rand();
        int two = std::rand();

        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto start_time = twine::current_rt_time();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        logger->info("Logging rt from thread {}, {}, {}, {}", thread_id, one, two, iterations);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        auto stop_time = twine::current_rt_time() - start_time;
        std::atomic_thread_fence(std::memory_order_seq_cst);

        times->push_back(stop_time);
        iterations--;
        //std::this_thread::sleep_for(SLEEP_TIME + std::chrono::milliseconds(thread_id));

        if (iterations % 100 == 0)
        {
            std::this_thread::sleep_for(SLEEP_TIME + std::chrono::milliseconds(thread_id));
        }
    }
}

int main()
{
    std::string log_level = "info";
    std::string log_filename = "log.txt";
    std::string log_name = "Performance testing logger";
    std::chrono::seconds log_flush_interval(1);

    // Logger configuration
    auto logger = elklog::ElkLogger("info");

    auto res = logger.initialize(log_filename, log_name, log_flush_interval);
    if (res != elklog::Status::OK)
    {
        std::cout << "Failed to initialize logger: " << res << std::endl;
        return -1;
    }

    std::vector<std::vector<std::chrono::nanoseconds>> time_logs(WORKERS);

    logger.info("Starting logging");
    std::vector<std::thread> workers(WORKERS);

    for (int i = 0; i < WORKERS; ++i)
    {
        workers[i] = std::thread(&logger_worker, &logger, &(time_logs[i]), ITERATIONS);
    }

    for (auto& w : workers)
    {
        w.join();
    }

    std::vector<std::chrono::nanoseconds> times;
    for (auto& t : time_logs)
    {
        times.insert(times.end(), t.begin(), t.end());
    }


    auto min_time = *std::min_element(times.begin(), times.end());
    auto max_time = *std::max_element(times.begin(), times.end());
    auto avg_time = std::accumulate(times.begin(), times.end(), std::chrono::nanoseconds(0)) / times.size();

    logger.info("Finished logging");

    std::cout << times.size() << std::endl;

    std::cout << "Min: " << min_time.count() << "ns, max: " << max_time.count() << " ns, avg: " << avg_time.count() << std::endl;

    return 0;
}

