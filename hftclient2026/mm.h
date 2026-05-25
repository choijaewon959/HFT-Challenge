#ifndef HIGH_FREQUENCY_TRADING_CHALLENGE_MM_H
#define HIGH_FREQUENCY_TRADING_CHALLENGE_MM_H

#include <vector>
#include <algorithm>

constexpr int N = 128;
constexpr int BS = 32;
constexpr int S = N * N;
constexpr int MOD = 997;

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
                            int a = A[i * N + k] % MOD;

                            for (int j = jj; j < jj + BS; ++j) {
                                C[i * N + j] += 1LL * a * (B[k * N + j] % MOD);
                            }
                        }
                    }
                }
            }
        }

        for (int idx = 0; idx < S; ++idx) {
            C[idx] %= MOD;
        }
    }

    static long long checksum_from_C(const std::vector<long long>& Cflat) {
        long long ans = 0;

        for (int idx = 0; idx < S; ++idx) {
            ans += Cflat[idx];
            ans %= MOD;
        }

        return ans;
    }

public:
    static long long checksum_via_matmul(const std::vector<int>& Aflat,
                                         const std::vector<int>& Bflat) {
        std::vector<long long> Cflat(S);

        matmul_128_blocked(Aflat, Bflat, Cflat);

        return checksum_from_C(Cflat);
    }

    static long long checksum_direct(const std::vector<int>& Aflat,
                                     const std::vector<int>& Bflat) {
        long long ans = 0;

        for (int i = 0; i < N; ++i) {
            int row = i * N;

            for (int j = 0; j < N; ++j) {
                long long cij = 0;

                for (int k = 0; k < N; ++k) {
                    cij += 1LL * (Aflat[row + k] % MOD) * (Bflat[k * N + j] % MOD);
                }

                ans += cij % MOD;
                ans %= MOD;
            }
        }

        return ans;
    }

    static long long checksum_direct_openmp(const std::vector<int>& Aflat,
                                            const std::vector<int>& Bflat) {
#ifdef USE_OPENMP
        long long ans = 0;

        #pragma omp parallel for reduction(+:ans)
        for (int i = 0; i < N; ++i) {
            long long local = 0;
            int row = i * N;

            for (int j = 0; j < N; ++j) {
                long long cij = 0;

                for (int k = 0; k < N; ++k) {
                    cij += 1LL * (Aflat[row + k] % MOD) * (Bflat[k * N + j] % MOD);
                }

                local += cij % MOD;
            }

            ans += local % MOD;
        }

        return ans % MOD;
#else
        return checksum_direct(Aflat, Bflat);
#endif
    }


    static long long checksum_via_row_col_sums(const std::vector<int>& Aflat,
                                   const std::vector<int>& Bflat) {
        long long colSumA[N] = {0};
        long long rowSumB[N] = {0};

#ifdef USE_OPENMP
        // Compute column sums of A.
        // Parallelize by column to avoid race conditions.
#pragma omp parallel for schedule(static)
        for (int j = 0; j < N; ++j) {
            long long sum = 0;

            for (int i = 0; i < N; ++i) {
                sum += Aflat[i * N + j];
            }

            colSumA[j] = sum % MOD;
        }

        // Compute row sums of B.
        // Parallelize by row to avoid race conditions.
#pragma omp parallel for schedule(static)
        for (int i = 0; i < N; ++i) {
            long long sum = 0;
            int row = i * N;

            for (int j = 0; j < N; ++j) {
                sum += Bflat[row + j];
            }

            rowSumB[i] = sum % MOD;
        }

        long long ans = 0;

#pragma omp parallel for reduction(+:ans) schedule(static)
        for (int k = 0; k < N; ++k) {
            ans += colSumA[k] * rowSumB[k];
        }

        return ans % MOD;

#else
        // Single-threaded version.
        for (int i = 0; i < N; ++i) {
            int row = i * N;

            for (int j = 0; j < N; ++j) {
                colSumA[j] += Aflat[row + j];
                rowSumB[i] += Bflat[row + j];
            }
        }

        long long ans = 0;

        for (int k = 0; k < N; ++k) {
            ans += (colSumA[k] % MOD) * (rowSumB[k] % MOD);
            ans %= MOD;
        }

        return ans;
#endif
    }
};

#endif