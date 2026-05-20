# High-Frequency Trading (HFT) Challenge

## Group Name
## Compilation instructions

## Optimization

### 1. Message Parsing
To secure a competitive edge in latency, we optimized the client-side socket reading and message parsing pipeline. By eliminating high-level abstractions, heap allocations, and redundant system calls, we reduced message ingestion latency down to near-hardware limits.

#### 1.1. Network Layer: Block Reads vs. Byte-by-Byte `recv`
*   **The Problem:** Main bottleneck of socket streaming is latency from calling the `recv` function forcing the CPU to constantly switch between User Space and Kernel Space.
*   **The Optimization:** Implemented a **Chunked Buffer Strategy** utilizing a static 64 KB ($1 \ll 16$) memory array. We grab massive blocks of data from the OS kernel in a single system call (`recv`), minimizing context-switching overhead.
*   **The Tradeoff:** Balanced memory allocation to fit entirely within the CPU's **L1/L2 Cache**. This prevents cache pollution and avoids expensive RAM fetch cycles (which are up to 100x slower than cache).

#### 1.2. Parsing Layer: Custom Pointer Math vs. `stringstream`/`atoi`
*   **The Problem:** Standard library tools (`std::stringstream`, `std::stoi`, or `atoi`) are plagued by heavy heap memory allocations and local geography/locale lookups that stall the CPU.
*   **The Optimization:** Designed a high-performance, inline `FastParser` utilizing raw pointer scanning. It processes numbers straight out of the flat memory buffer via localized register accumulation:
    ```cpp
    while (c >= '0' && c <= '9') {
        x = x * 10 + (c - '0');
    }

#### 1.3. Performance Benchmarking

To validate our optimizations, we profiled the parsing latency for two 128x128 matrices over 20 distinct trials. The results demonstrate a near 10-fold improvement in data ingestion speed.

- **v1 vs. v2 Latency Comparison:**
  The following table tracks the average time in microseconds (μs) required to move data from the socket buffer into binary integer arrays.

| Optimization Version    | Avg Read Matrix A (μs) | Avg Read Matrix B (μs) | Total Parsing Time (μs) | Performance Gain |
|:------------------------| :---: | :---: | :---: | :---: |
| **v1: Standard Parser** | 6,094.74 | 5,700.11 | 11,794.85 | Baseline |
| **v2: FastParser**      | **729.63** | **557.58** | **1,287.21** | **9.16x Faster** |

- **Optimization Impact Analysis:**
  By replacing high-level string abstractions with our custom **FastParser**, we achieved the following:

    1.  **91% Latency Reduction:** Total parsing time dropped from ~11.8ms to ~1.3ms.
    2.  **Zero-Allocation Pipeline:** v2 avoids all `std::string` heap allocations during the read loop, preventing GC-like pauses and memory fragmentation.
    3.  **Cache Efficiency:** The optimized parser operates within the L1/L2 cache boundaries, ensuring the CPU never stalls waiting for system RAM.


## optimization techniques
### 1. Flattened matrix representation
Incoming matrices are converted from
```cpp
vector<vector<int>>
```
to:
```cpp
vector<int>
```
using row major contiguous storage.

Flattening improves: cache efficiency & vectorization opportunities.
vector<vector<int>> causes poor spacial locality & cache misses and overall inefficient column access.

### 2. Blocked matrix multiplication
The multiplication uses cache blocking
```cpp
constexpr int BS = 32;
```
instead of taking entire rows / columns.

This improves L1/L2 cache reuse and reduces memory bandwidth pressure which is the main bottleneck in this context.
This outperforms naive O(n^3).

### 3. Contiguous pointer access
Vectors are accessed using:
```cpp
&Aflat[0]
```
to obtain raw pointers giving a lower abstraction overhead and compiler friendly pointer arithmetic.


## Network layer

### Send/Receive model
Sockets transmit raw bytes only so matrices are serialized before transmission.
```cpp
send(sock, answerStr.c_str(), answerStr.size(), 0);
```

for maximum throughput, binary transmission can also be used:
```cpp
send(sock, 
    reinterpret_cast<char*>(&Cflat[0]),
    answerStr.size() * sizeof(long long),
    0);
```

### Build
```cpp
g++ -03 -march=native -std=c++17 main.cpp main
```
We use these flags at compilation:
```cpp
-03
-march=native
```
to further optimize aggressively and facilitate auto-vectorization.



