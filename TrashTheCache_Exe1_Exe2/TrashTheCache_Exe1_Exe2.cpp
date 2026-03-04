#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdint>

struct Transform
{
    float matrix[16] =
    {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
};

class GameObject3D
{
public:
    Transform transform;
    int ID;
};

class GameObject3DAlt
{
public:
    int ID;
};

static double average_excluding_outliers(std::vector<long long>& samples)
{
    std::sort(samples.begin(), samples.end());

    if (samples.size() <= 2)
        return samples.empty() ? 0.0 : static_cast<double>(samples[0]);

    long long sum = 0;
    for (size_t i = 1; i + 1 < samples.size(); ++i)
        sum += samples[i];

    return static_cast<double>(sum) /
        static_cast<double>(samples.size() - 2);
}

int main()
{
    constexpr int runs = 11;

    // =========================================================
    // ===================== EXERCISE 1 ========================
    // =========================================================

    constexpr std::size_t kLogN_int = 26;
    constexpr std::size_t kN_int = 1ULL << kLogN_int;

    std::vector<int> int_buffer(kN_int, 1);

    std::cout << "========== EXERCISE 1 (INT BUFFER) ==========\n";
    std::cout << "Elements: " << kN_int << "\n";
    std::cout << "Step\tTime(us)\tChecksum\n";

    for (std::size_t step = 1; step <= 1024; step <<= 1)
    {
        std::vector<long long> times;
        times.reserve(runs);

        volatile uint64_t checksum = 0;

        for (int r = 0; r < runs; ++r)
        {
            const auto start =
                std::chrono::high_resolution_clock::now();

            for (std::size_t i = 0; i < kN_int; i += step)
            {
                int_buffer[i] *= 2;
                checksum += int_buffer[i];
            }

            const auto end =
                std::chrono::high_resolution_clock::now();

            times.push_back(
                std::chrono::duration_cast<
                std::chrono::microseconds>(end - start).count());
        }

        const double avg =
            average_excluding_outliers(times);

        std::cout << step << "\t"
            << static_cast<long long>(avg) << "\t"
            << checksum << "\n";
    }

    // =========================================================
    // ===================== EXERCISE 2 ========================
    // =========================================================

    constexpr std::size_t kLogN_obj = 22;
    constexpr std::size_t kN_obj = 1ULL << kLogN_obj;

    std::vector<GameObject3D> aos(kN_obj);
    std::vector<GameObject3DAlt> soa_ids(kN_obj);
    std::vector<Transform> soa_transforms(kN_obj);

    for (std::size_t i = 0; i < kN_obj; ++i)
    {
        aos[i].ID = static_cast<int>(i);
        soa_ids[i].ID = static_cast<int>(i);
        soa_transforms[i] = Transform();
    }

    std::cout << "\n========== EXERCISE 2 (AoS vs SoA) ==========\n";
    std::cout << "Elements: " << kN_obj << "\n";
    std::cout << "Step\tAoS(us)\tSoA(us)\n";

    for (std::size_t step = 1; step <= 1024; step <<= 1)
    {
        std::vector<long long> times_aos;
        std::vector<long long> times_soa;

        times_aos.reserve(runs);
        times_soa.reserve(runs);

        volatile uint64_t checksum_aos = 0;
        volatile uint64_t checksum_soa = 0;

        for (int r = 0; r < runs; ++r)
        {
            // AoS
            {
                const auto start =
                    std::chrono::high_resolution_clock::now();

                for (std::size_t i = 0; i < kN_obj; i += step)
                {
                    aos[i].ID *= 2;
                    checksum_aos += aos[i].ID;
                }

                const auto end =
                    std::chrono::high_resolution_clock::now();

                times_aos.push_back(
                    std::chrono::duration_cast<
                    std::chrono::microseconds>(end - start).count());
            }

            // SoA
            {
                const auto start =
                    std::chrono::high_resolution_clock::now();

                for (std::size_t i = 0; i < kN_obj; i += step)
                {
                    soa_ids[i].ID *= 2;
                    checksum_soa += soa_ids[i].ID;
                }

                const auto end =
                    std::chrono::high_resolution_clock::now();

                times_soa.push_back(
                    std::chrono::duration_cast<
                    std::chrono::microseconds>(end - start).count());
            }
        }

        const double avg_aos =
            average_excluding_outliers(times_aos);
        const double avg_soa =
            average_excluding_outliers(times_soa);

        std::cout << step << "\t"
            << static_cast<long long>(avg_aos) << "\t"
            << static_cast<long long>(avg_soa) << "\n";
    }

    return 0;
}