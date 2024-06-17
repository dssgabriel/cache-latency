#define _GNU_SOURCE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

constexpr size_t MAX_BUFFER = 2048;

void* pos       = nullptr;
void** memblock = nullptr;

static inline uint64_t rnd(uint64_t threshold) {
    uint64_t a = (uint64_t)rand() << 48;
    uint64_t b = (uint64_t)rand() << 32;
    uint64_t c = (uint64_t)rand() << 16;
    uint64_t d = (uint64_t)rand();
    return (a ^ b ^ c ^ d) % threshold;
}

/// Using a pointer chasing benchmark to measure the cache latency in ns.
double cache_latency(uint64_t size, uint64_t iterations) {
    int cycle_len    = 1;
    double ns_per_it = 0.0;

    if (nullptr == pos) {
        pos = &memblock[0];
    }

    // Initialize
    for (size_t i = 0; i < size; ++i) {
        memblock[i] = &memblock[i];
    }

    // Shuffle pointer addresses
    for (ssize_t i = size - 1; i >= 0; --i) {
        if (i < cycle_len) {
            continue;
        }
        uint64_t j  = rnd(i / cycle_len) * cycle_len + (i % cycle_len);
        void* swp   = memblock[i];
        memblock[i] = memblock[j];
        memblock[j] = swp;
    }

    void* p        = pos;
    double elapsed = 0.0;
    struct timespec t1, t2;
    do {
        clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
        // Random pointer chasing loop, unrolled 16 times
        for (ssize_t i = iterations; i; --i) {
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
            p = *(void**)p;
        }
        clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
        elapsed = (double)(t2.tv_nsec - t1.tv_nsec);
    } while (elapsed <= 0.0);

    ns_per_it = elapsed / ((double)iterations * 16.0);
    pos       = p;

    return ns_per_it;
}

int main(int argc, char* argv[argc + 1]) {
    uint64_t addr_space_sz = 67108864;
    if (argc > 1) {
        addr_space_sz = atoll(argv[1]) * sizeof(void*);
    }
    fprintf(
        stderr,
        "Measuring cache/memory latency up to %lu B (%.3lf MiB)\n",
        addr_space_sz / sizeof(void*),
        ((double)addr_space_sz / (double)sizeof(void*)) / 1024.0 / 1024.0
    );

    uint64_t rounds         = 0;
    uint64_t min_iterations = 1024;
    uint64_t max_iterations = 8388608;
    uint64_t iterations     = max_iterations;

    double nanos[MAX_BUFFER];
    uint64_t sizes[MAX_BUFFER];

    memblock = aligned_alloc(64, addr_space_sz);
    // Initial value is the address of the element
    for (size_t i = 0; i < addr_space_sz / sizeof(void*); ++i) {
        memblock[i] = &memblock[i];
    }

    printf("%s,%s\n", "Memory size (B)", "Cache latency (ns)");
    for (size_t size = 512, step = 16; size <= addr_space_sz / sizeof(void*); size += step) {
        fprintf(stderr, "Current cache size: %zu B (%.3lf KiB)\r", size, (double)size / 1024.0);
        double ns_per_it = cache_latency(size, iterations);

        sizes[rounds] = size;
        nanos[rounds] = ns_per_it;
        rounds++;
        // Double step size every powers of two
        if (0 == ((size - 1) & size)) {
            step <<= 1;
        }

        iterations = (uint64_t)((double)max_iterations / ns_per_it);
        if (iterations < min_iterations) {
            iterations = min_iterations;
        }

        if (rounds == MAX_BUFFER) {
            for (size_t r = 0; r < MAX_BUFFER; ++r) {
                printf("%lu,%lf\n", sizes[r], nanos[r]);
            }
            rounds = 0;
        }
    }
    for (size_t r = 0; r < rounds; ++r) {
        printf("%lu,%lf\n", sizes[r], nanos[r]);
    }

    free(memblock);
    return 0;
}
