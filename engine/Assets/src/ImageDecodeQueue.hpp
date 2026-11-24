#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <future>
#include <type_traits>
#include <vector>

namespace ForgeSim::Assets::Detail
{
    // Keep workers occupied across uneven images without batch barriers. Consume
    // results (and propagate decoder errors) in source order. Async job lifetimes
    // join before decoder state/promise storage can unwind.
    template<class Decode, class Consume>
    void ProcessImagesInOrder(std::size_t count, std::size_t workerLimit,
        Decode&& decode, Consume&& consume)
    {
        const auto workerCount = std::min(count, std::clamp(workerLimit, std::size_t{1}, std::size_t{4}));
        if (workerCount <= 1)
        {
            for (std::size_t index = 0; index < count; ++index) consume(decode(index));
            return;
        }
        using Value = std::invoke_result_t<Decode&, std::size_t>;
        std::vector<std::promise<Value>> promises(count);
        std::vector<std::future<Value>> results;
        results.reserve(count);
        for (auto& promise : promises) results.push_back(promise.get_future());
        std::atomic<std::size_t> next{0};
        std::vector<std::future<void>> jobs;
        jobs.reserve(workerCount);
        for (std::size_t worker = 0; worker < workerCount; ++worker)
            jobs.push_back(std::async(std::launch::async, [&]
            {
                for (;;)
                {
                    const auto index = next.fetch_add(1, std::memory_order_relaxed);
                    if (index >= count) return;
                    try { promises[index].set_value(decode(index)); }
                    catch (...) { promises[index].set_exception(std::current_exception()); }
                }
            }));
        for (auto& result : results) consume(result.get());
        for (auto& job : jobs) job.get();
    }
}
