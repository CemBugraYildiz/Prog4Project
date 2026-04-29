#include "BenchmarkPi.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <execution>
#include <future>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>


#if defined(__cpp_lib_parallel_algorithm) && __cpp_lib_parallel_algorithm >= 201603L
#define HAS_EXECUTION_POLICIES 1
#else
#define HAS_EXECUTION_POLICIES 0
#endif
namespace
{
    constexpr std::uint64_t kSteps = 100'000'000;
    constexpr int kRuns = 10;

    using Clock = std::chrono::steady_clock;

    unsigned GetHardwareThreads()
    {
        return std::max(1u, std::thread::hardware_concurrency());
    }

    unsigned GetTaskCount(std::uint64_t steps)
    {
        return std::max(1u, std::min<unsigned>(GetHardwareThreads(), static_cast<unsigned>(steps)));
    }

    void PrintHeader()
    {
        std::cout << "# threads: " << GetHardwareThreads() << '\n';
        std::cout << "# samples: " << kSteps << '\n';
        std::cout << "# measurements: " << kRuns << "\n\n";
    }

    void PrintResultBlock(const std::string& title, double elapsedMs, double piValue)
    {
        std::ostringstream line1;
        line1 << title << ": " << std::fixed << std::setprecision(0) << elapsedMs << " ms";

        std::ostringstream line2;
        line2 << "result: " << std::fixed << std::setprecision(5) << piValue;

        std::cout << line1.str() << '\n';
        std::cout << line2.str() << "\n\n";
    }

    // Simple counting iterator for transform_reduce
    class CountingIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = std::uint64_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type;
        using pointer = void;

        CountingIterator() noexcept = default;
        explicit CountingIterator(value_type v) noexcept
            : value_(static_cast<difference_type>(v))
        {
        }

        reference operator*() const noexcept
        {
            return static_cast<value_type>(value_);
        }

        reference operator[](difference_type n) const noexcept
        {
            return static_cast<value_type>(value_ + n);
        }

        CountingIterator& operator++() noexcept
        {
            ++value_;
            return *this;
        }

        CountingIterator operator++(int) noexcept
        {
            CountingIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        CountingIterator& operator--() noexcept
        {
            --value_;
            return *this;
        }

        CountingIterator operator--(int) noexcept
        {
            CountingIterator tmp(*this);
            --(*this);
            return tmp;
        }

        CountingIterator& operator+=(difference_type n) noexcept
        {
            value_ += n;
            return *this;
        }

        CountingIterator& operator-=(difference_type n) noexcept
        {
            value_ -= n;
            return *this;
        }

        friend CountingIterator operator+(CountingIterator it, difference_type n) noexcept
        {
            it += n;
            return it;
        }

        friend CountingIterator operator+(difference_type n, CountingIterator it) noexcept
        {
            it += n;
            return it;
        }

        friend CountingIterator operator-(CountingIterator it, difference_type n) noexcept
        {
            it -= n;
            return it;
        }

        friend difference_type operator-(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return a.value_ - b.value_;
        }

        friend bool operator==(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return a.value_ == b.value_;
        }

        friend bool operator!=(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return !(a == b);
        }

        friend bool operator<(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return a.value_ < b.value_;
        }

        friend bool operator>(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return b < a;
        }

        friend bool operator<=(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return !(b < a);
        }

        friend bool operator>=(const CountingIterator& a, const CountingIterator& b) noexcept
        {
            return !(a < b);
        }

    private:
        difference_type value_{ 0 };
    };

    double PartialSum(std::uint64_t begin, std::uint64_t end, double step)
    {
        double sum = 0.0;
        for (std::uint64_t i = begin; i < end; ++i)
        {
            const double x = (static_cast<double>(i) + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
        return sum;
    }

    double CalculatePiOriginal(std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const double step = 1.0 / static_cast<double>(steps);
        double sum = 0.0;

        for (std::uint64_t i = 0; i < steps; ++i)
        {
            const double x = (static_cast<double>(i) + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }

        return sum * step;
    }

    void AtomicAdd(std::atomic<double>& target, double value)
    {
        double current = target.load(std::memory_order_relaxed);
        while (!target.compare_exchange_weak(
            current,
            current + value,
            std::memory_order_relaxed,
            std::memory_order_relaxed))
        {
            // current is updated by compare_exchange_weak
        }
    }

    double CalculatePiMutex(std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const unsigned threadCount = GetTaskCount(steps);
        const std::uint64_t chunk = steps / threadCount;
        const double step = 1.0 / static_cast<double>(steps);

        double sum = 0.0;
        std::mutex sumMutex;
        std::vector<std::thread> threads;
        threads.reserve(threadCount);

        for (unsigned t = 0; t < threadCount; ++t)
        {
            const std::uint64_t begin = static_cast<std::uint64_t>(t) * chunk;
            const std::uint64_t end = (t == threadCount - 1) ? steps : begin + chunk;

            threads.emplace_back([begin, end, step, &sum, &sumMutex]()
                {
                    double localSum = 0.0;
                    for (std::uint64_t i = begin; i < end; ++i)
                    {
                        const double x = (static_cast<double>(i) + 0.5) * step;
                        localSum += 4.0 / (1.0 + x * x);
                    }

                    std::lock_guard<std::mutex> lock(sumMutex);
                    sum += localSum;
                });
        }

        for (auto& th : threads)
            th.join();

        return sum * step;
    }

    double CalculatePiAtomic(std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const unsigned threadCount = GetTaskCount(steps);
        const std::uint64_t chunk = steps / threadCount;
        const double step = 1.0 / static_cast<double>(steps);

        std::atomic<double> sum{ 0.0 };
        std::vector<std::thread> threads;
        threads.reserve(threadCount);

        for (unsigned t = 0; t < threadCount; ++t)
        {
            const std::uint64_t begin = static_cast<std::uint64_t>(t) * chunk;
            const std::uint64_t end = (t == threadCount - 1) ? steps : begin + chunk;

            threads.emplace_back([begin, end, step, &sum]()
                {
                    double localSum = 0.0;
                    for (std::uint64_t i = begin; i < end; ++i)
                    {
                        const double x = (static_cast<double>(i) + 0.5) * step;
                        localSum += 4.0 / (1.0 + x * x);
                    }

                    AtomicAdd(sum, localSum);
                });
        }

        for (auto& th : threads)
            th.join();

        return sum.load(std::memory_order_relaxed) * step;
    }

    double CalculatePiFuturePromise(std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const unsigned threadCount = GetTaskCount(steps);
        const std::uint64_t chunk = steps / threadCount;
        const double step = 1.0 / static_cast<double>(steps);

        std::vector<std::future<double>> futures;
        futures.reserve(threadCount);

        std::vector<std::thread> workers;
        workers.reserve(threadCount);

        for (unsigned t = 0; t < threadCount; ++t)
        {
            const std::uint64_t begin = static_cast<std::uint64_t>(t) * chunk;
            const std::uint64_t end = (t == threadCount - 1) ? steps : begin + chunk;

            std::promise<double> promise;
            futures.push_back(promise.get_future());

            workers.emplace_back([p = std::move(promise), begin, end, step]() mutable
                {
                    try
                    {
                        p.set_value(PartialSum(begin, end, step));
                    }
                    catch (...)
                    {
                        p.set_exception(std::current_exception());
                    }
                });
        }

        for (auto& worker : workers)
            worker.join();

        double sum = 0.0;
        for (auto& f : futures)
            sum += f.get();

        return sum * step;
    }

    double CalculatePiAsync(std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const unsigned threadCount = GetTaskCount(steps);
        const std::uint64_t chunk = steps / threadCount;
        const double step = 1.0 / static_cast<double>(steps);

        std::vector<std::future<double>> futures;
        futures.reserve(threadCount);

        for (unsigned t = 0; t < threadCount; ++t)
        {
            const std::uint64_t begin = static_cast<std::uint64_t>(t) * chunk;
            const std::uint64_t end = (t == threadCount - 1) ? steps : begin + chunk;

            futures.push_back(std::async(std::launch::async, [=]()
                {
                    return PartialSum(begin, end, step);
                }));
        }

        double sum = 0.0;
        for (auto& f : futures)
            sum += f.get();

        return sum * step;
    }

    template <typename ExecutionPolicy>
    double CalculatePiWithPolicy(ExecutionPolicy&& policy, std::uint64_t steps)
    {
        if (steps == 0)
            return 0.0;

        const double step = 1.0 / static_cast<double>(steps);

        const double sum = std::transform_reduce(
            std::forward<ExecutionPolicy>(policy),
            CountingIterator{ 0 },
            CountingIterator{ steps },
            0.0,
            std::plus<double>{},
            [step](std::uint64_t i)
            {
                const double x = (static_cast<double>(i) + 0.5) * step;
                return 4.0 / (1.0 + x * x);
            });

        return sum * step;
    }

    struct BenchmarkResult
    {
        std::string name;
        double pi{ 0.0 };
        double trimmedAverageMs{ 0.0 };
    };

    template <typename Func>
    BenchmarkResult Measure(const std::string& name, Func&& func, int runs = kRuns)
    {
        BenchmarkResult result;
        result.name = name;

        std::vector<double> samples;
        samples.reserve(runs);

        (void)func(); // warm-up

        for (int i = 0; i < runs; ++i)
        {
            const auto start = Clock::now();
            result.pi = func();
            const auto end = Clock::now();

            const double ms = std::chrono::duration<double, std::milli>(end - start).count();
            samples.push_back(ms);
        }

        const auto [minIt, maxIt] = std::minmax_element(samples.begin(), samples.end());
        const double total = std::accumulate(samples.begin(), samples.end(), 0.0);

        result.trimmedAverageMs = (total - *minIt - *maxIt) / static_cast<double>(runs - 2);
        return result;
    }
}

void RunPiBenchmarks()
{
    std::cout.setf(std::ios::unitbuf);

    PrintHeader();

    const auto original = Measure("Original version", [] { return CalculatePiOriginal(kSteps); });
    PrintResultBlock("Original version", original.trimmedAverageMs, original.pi);

    const auto mutexVersion = Measure("Mutex version", [] { return CalculatePiMutex(kSteps); });
    PrintResultBlock("Mutex version", mutexVersion.trimmedAverageMs, mutexVersion.pi);

    const auto atomicVersion = Measure("Atomic version", [] { return CalculatePiAtomic(kSteps); });
    PrintResultBlock("Atomic version", atomicVersion.trimmedAverageMs, atomicVersion.pi);

    const auto promiseVersion = Measure("Promise version", [] { return CalculatePiFuturePromise(kSteps); });
    PrintResultBlock("Promise version", promiseVersion.trimmedAverageMs, promiseVersion.pi);

    const auto asyncVersion = Measure("Async version", [] { return CalculatePiAsync(kSteps); });
    PrintResultBlock("Async version", asyncVersion.trimmedAverageMs, asyncVersion.pi);

#if HAS_EXECUTION_POLICIES
    const auto seqVersion = Measure("STL version seq", [] { return CalculatePiWithPolicy(std::execution::seq, kSteps); });
    PrintResultBlock("STL version seq", seqVersion.trimmedAverageMs, seqVersion.pi);

    const auto parVersion = Measure("STL version par", [] { return CalculatePiWithPolicy(std::execution::par, kSteps); });
    PrintResultBlock("STL version par", parVersion.trimmedAverageMs, parVersion.pi);

    const auto parUnseqVersion = Measure("STL version par_unseq", [] { return CalculatePiWithPolicy(std::execution::par_unseq, kSteps); });
    PrintResultBlock("STL version par_unseq", parUnseqVersion.trimmedAverageMs, parUnseqVersion.pi);

    const auto unseqVersion = Measure("STL version unseq", [] { return CalculatePiWithPolicy(std::execution::unseq, kSteps); });
    PrintResultBlock("STL version unseq", unseqVersion.trimmedAverageMs, unseqVersion.pi);
#endif
}