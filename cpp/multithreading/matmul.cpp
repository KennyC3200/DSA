// At SIZE = 20
// Single-threaded: 1290.8 us total, 64.54 us per iteration
// Multithreaded:   17358 us total, 867.9 us per iteration

// At SIZE = 200
// Single-threaded: 605249 us total, 30262.4 us per iteration
// Multithreaded:   157413 us total, 7870.65 us per iteration

// At SIZE = 300
// Single-threaded: 1.95637e+06 us total, 97818.4 us per iteration
// Multithreaded:   401826 us total, 20091.3 us per iteration

#include <thread>
#include <algorithm>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <iostream>

template <int M, int N>
struct mat2 {
    using DataType = float;

    void Set(int row, int col, DataType val) {
        data[row * N + col] = val;
    }

    DataType Get(int row, int col) const {
        return data[row * N + col];
    }

    void Add(int row, int col, DataType val) {
        data[row * N + col] += val;
    }

    // Data is stored row-major
    std::array<DataType, M * N> data = { 0 };
};

template <int M, int R, int N>
void matmul_rows(
    const mat2<M, R>& left, const mat2<R, N>& right, 
    int row_begin, int row_end, mat2<M, N>* out) 
{
    // i, j, k order: inner loop walks right and out contiguously, better for cache locality
    for (int i = row_begin; i < row_end; i++) {
        for (int k = 0; k < R; k++) {
            auto a = left.Get(i, k);
            for (int j = 0; j < N; j++) {
                out->Add(i, j, a * right.Get(k, j));
            }
        }
    }
}

template <int M, int R, int N>
void matmul_parallel(const mat2<M, R>& left, const mat2<R, N>& right, mat2<M ,N>* out) {
    int n_threads = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    int chunk = (M + n_threads - 1) / n_threads;

    std::vector<std::thread> threads;
    for (int t = 0; t < n_threads; t++) {
        int start = t * chunk;
        int end = std::min(M, start + chunk);

        if (start >= end) break;

        // Use emplace_back because it constructs the threa object inside of the vector
        threads.emplace_back(
            matmul_rows<M, R, N>, 
            std::cref(left), std::cref(right), 
            start, end, out);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main() {
    constexpr int SIZE = 300;
    constexpr int ITERATIONS = 20;

    mat2<SIZE, SIZE> a, b;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            a.Set(i, j, static_cast<float>(i + j));
            b.Set(i, j, static_cast<float>(i - j));
        }
    }

    using clock = std::chrono::steady_clock;
    mat2<SIZE, SIZE> single_out, parallel_out;

    auto single_start = clock::now();
    for (int it = 0; it < ITERATIONS; it++) {
        single_out = {}; // Reset to zero since matmul_rows accumulates into out
        matmul_rows(a, b, 0, SIZE, &single_out);
    }
    auto single_end = clock::now();

    auto parallel_start = clock::now();
    for (int it = 0; it < ITERATIONS; it++) {
        parallel_out = {};
        matmul_parallel(a, b, &parallel_out);
    }
    auto parallel_end = clock::now();

    std::chrono::duration<double, std::micro> single_time = single_end - single_start;
    std::chrono::duration<double, std::micro> parallel_time = parallel_end - parallel_start;

    std::cout << SIZE << "x" << SIZE << " matmul, " << ITERATIONS << " iterations\n";
    std::cout << "Single-threaded: " << single_time.count() << " us total, "
              << single_time.count() / ITERATIONS << " us per iteration\n";
    std::cout << "Multithreaded:   " << parallel_time.count() << " us total, "
              << parallel_time.count() / ITERATIONS << " us per iteration\n";
    std::cout << "Results match: " << (single_out.data == parallel_out.data ? "yes" : "no") << "\n";
}