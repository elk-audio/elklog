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

constexpr int ITERATIONS = 30000;
constexpr int WORKERS = 4;
constexpr auto SLEEP_TIME = std::chrono::milliseconds(50);

std::atomic<int> thread_counter = 0;


void logger_worker(elklog::ElkLogger* logger, std::vector<std::chrono::nanoseconds>* times, int iterations)
{
    times->reserve(iterations);
    twine::ThreadRtFlag rt_flag;
    int thread_id = thread_counter.fetch_add(1);
    std::string str_thread_id = "000" + std::to_string(thread_id);

    while (iterations > 0)
    {
        int one = std::rand() + iterations;
        float two = (float)std::rand() / 0.02f;

        std::atomic_thread_fence(std::memory_order_seq_cst);
        //auto start_time = twine::current_rt_time();
        auto start_time = std::chrono::high_resolution_clock::now();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        logger->info("Logging rt from thread {}, {}, {}", str_thread_id.c_str(), one, two);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        //auto stop_time = twine::current_rt_time() - start_time;
        auto stop_time = std::chrono::high_resolution_clock::now() - start_time;
        std::atomic_thread_fence(std::memory_order_seq_cst);

        times->push_back(stop_time);
        iterations--;

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

    //logger_worker(&logger, &(time_logs[0]), ITERATIONS);

    std::vector<std::chrono::nanoseconds> recs;
    for (auto& t : time_logs)
    {
        recs.insert(recs.end(), t.begin(), t.end());
    }

    std::sort(recs.begin(), recs.end());
    auto min_time = recs.front();
    auto max_time = recs.back();
    auto med_time = recs.at(recs.size() / 2);
    auto avg_time = std::accumulate(recs.begin(), recs.end(), std::chrono::nanoseconds(0)) / recs.size();

    logger.info("Finished logging");

    std::cout << "Min: " << min_time.count() << "ns, max: " << max_time.count() << " ns, avg: "
              << avg_time.count() << ", median: " << med_time.count() << std::endl;

    return 0;
}

