#ifndef HIGH_FREQUENCY_TRADING_CHALLENGE_MM_H
#define HIGH_FREQUENCY_TRADING_CHALLENGE_MM_H

#include <vector>
#include <algorithm>

constexpr int N = 128;
constexpr int BS = 32;
constexpr int S = N * N;

class mm {
private:
    static void matmul_128_blocked(const std::vector<int>& Aflat,
                                   const std::vector<int>& Bflat,
                                   std::vector<long long>& Cflat) {
        std::fill(Cflat.begin(), Cflat.end(), 0);

        const int* A = Aflat.data();
        const int* B = Bflat.data();
        long long* C = Cflat.data();

        for (int ii = 0; ii < N; ii += BS) {
            for (int kk = 0; kk < N; kk += BS) {
                for (int jj = 0; jj < N; jj += BS) {
                    for (int i = ii; i < ii + BS; ++i) {
                        for (int k = kk; k < kk + BS; ++k) {
                            int a = A[i * N + k];

                            for (int j = jj; j < jj + BS; ++j) {
                                C[i * N + j] += 1LL * a * B[k * N + j];
                            }
                        }
                    }
                }
            }
        }
    }

    static long long trace_from_C(const std::vector<long long>& Cflat) {
        long long ans = 0;

        for (int i = 0; i < N; ++i) {
            ans += Cflat[i * N + i];
        }

        return ans;
    }

public:
    static long long trace_via_matmul(const std::vector<int>& Aflat,
                                      const std::vector<int>& Bflat) {
        std::vector<long long> Cflat(S);

        matmul_128_blocked(Aflat, Bflat, Cflat);

        return trace_from_C(Cflat);
    }

    static long long trace_direct(const std::vector<int>& Aflat,
                                  const std::vector<int>& Bflat) {
        long long ans = 0;

        for (int i = 0; i < N; ++i) {
            int row = i * N;

            for (int k = 0; k < N; ++k) {
                ans += 1LL * Aflat[row + k] * Bflat[k * N + i];
            }
        }

        return ans;
    }

    static long long trace_direct_openmp(const std::vector<int>& Aflat,
                                         const std::vector<int>& Bflat) {
#ifdef USE_OPENMP
        long long ans = 0;

        #pragma omp parallel for reduction(+:ans)
        for (int i = 0; i < N; ++i) {
            int row = i * N;

            for (int k = 0; k < N; ++k) {
                ans += 1LL * Aflat[row + k] * Bflat[k * N + i];
            }
        }

        return ans;
#else
        return trace_direct(Aflat, Bflat);
#endif
    }
};

#endif