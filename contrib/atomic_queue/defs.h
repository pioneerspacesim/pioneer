#pragma once
/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/*
 * Copyright (c) 2019 Maxim Egorushkin
 * MIT License. See the full licence in file LICENSE.
 *
 * This file provides fundamental definitions and operations for atomic queues,
 * including architecture-specific CPU pause instructions to optimize spin loops,
 * and basic macros for memory-order hints.
 */

#ifndef ATOMIC_QUEUE_DEFS_H_INCLUDED
#define ATOMIC_QUEUE_DEFS_H_INCLUDED

#include <atomic>

// ----------------------------------------------------------------------------
// Architecture-specific detection and definitions
// ----------------------------------------------------------------------------

// NOTE: If you add support for additional architectures or refine the existing ones,
// please confirm that `CACHE_LINE_SIZE` and `spin_loop_pause()` are correct on real hardware.

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

#   include <emmintrin.h> // For _mm_pause on x86/x86_64

namespace atomic_queue {

/// Size (in bytes) of the L1 cache line on x86/x86_64 typically 64 bytes.
constexpr int CACHE_LINE_SIZE = 64;

/**
 * On x86, `_mm_pause()` is used to reduce power consumption and memory pipeline
 * conflicts within spin-wait loops.
 */
static inline void spin_loop_pause() noexcept {
    _mm_pause();
}

} // namespace atomic_queue

#elif defined(__arm__) || defined(__aarch64__)

namespace atomic_queue {

/**
 * Size (in bytes) of the L1 cache line on most ARM-based chips is typically 64 bytes.
 * This is a safe assumption, but verify for specialized SoCs.
 */
constexpr int CACHE_LINE_SIZE = 64;

/**
 * On ARM (and ARM64), the recommended instruction is `yield` if the architecture
 * version is at least ARMv6K or newer. Otherwise, we fallback to `nop`.
 * This helps the CPU's hardware thread scheduler realize we are in a spin loop.
 */
static inline void spin_loop_pause() noexcept {
#if defined(__ARM_ARCH_6K__)    \
 || defined(__ARM_ARCH_6Z__)    \
 || defined(__ARM_ARCH_6ZK__)   \
 || defined(__ARM_ARCH_6T2__)   \
 || defined(__ARM_ARCH_7__)     \
 || defined(__ARM_ARCH_7A__)    \
 || defined(__ARM_ARCH_7R__)    \
 || defined(__ARM_ARCH_7M__)    \
 || defined(__ARM_ARCH_7S__)    \
 || defined(__ARM_ARCH_8A__)    \
 || defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#else
    asm volatile("nop" ::: "memory");
#endif
}

} // namespace atomic_queue

#elif defined(__ppc64__) || defined(__powerpc64__)

namespace atomic_queue {

/**
 * The L1 cache line size on powerpc64 can be 128 on some implementations,
 * but this may differ on certain microarchitectures.
 */
constexpr int CACHE_LINE_SIZE = 128; // TODO: Verify for your specific PowerPC64 platform.

/**
 * For PowerPC64, we use a minimal-latency hint. The example uses the trick
 * "or 31,31,31" as a possible spin-loop step. Benchmark on real hardware
 * to confirm if this is best practice or use `nop`.
 */
static inline void spin_loop_pause() noexcept {
    asm volatile("or 31,31,31 # spin_pause_hint" ::: "memory");
}

} // namespace atomic_queue

#elif defined(__s390x__)

namespace atomic_queue {

/**
 * The L1 cache line size for s390x is often 256 bytes, but verify for the specific system.
 */
constexpr int CACHE_LINE_SIZE = 256; // TODO: Confirm this is correct for your environment.

/**
 * Currently, we do not have a recommended spin instruction for s390x.
 * The function is left as a no-op until a verified instruction is found.
 */
static inline void spin_loop_pause() noexcept {
    // TODO: Implement if there's a known instruction that aids spin loops on s390x.
}

} // namespace atomic_queue

#elif defined(__riscv)

namespace atomic_queue {

/**
 * The L1 cache line size for many RISC-V platforms is 64, though this can vary widely.
 */
constexpr int CACHE_LINE_SIZE = 64;

/**
 * For RISC-V, `nop` is a safe fallback. If there's a better spin-wait hint
 * instruction in a future extension, substitute it here.
 */
static inline void spin_loop_pause() noexcept {
    asm volatile("nop");
}

} // namespace atomic_queue

#else
#warning "Unknown CPU architecture. Using 64 bytes as the default L1 cache line size and no spin-loop pause instruction."

namespace atomic_queue {

/// Default assumption of 64 bytes for the L1 cache line on unknown architectures.
constexpr int CACHE_LINE_SIZE = 64;

/**
 * On unknown architectures, we do not have a spin-loop pause instruction,
 * so we leave this function empty. If your platform has an alternative,
 * replace this with the correct inline assembly or intrinsic.
 */
static inline void spin_loop_pause() noexcept {}

} // namespace atomic_queue
#endif

// ----------------------------------------------------------------------------
// Macros for branch prediction, inline usage, and memory-order convenience
// ----------------------------------------------------------------------------

namespace atomic_queue {

#if defined(__GNUC__) || defined(__clang__)
#   define ATOMIC_QUEUE_LIKELY(expr)     __builtin_expect(static_cast<bool>(expr), 1)
#   define ATOMIC_QUEUE_UNLIKELY(expr)   __builtin_expect(static_cast<bool>(expr), 0)
#   define ATOMIC_QUEUE_NOINLINE         __attribute__((noinline))
#else
#   define ATOMIC_QUEUE_LIKELY(expr)     (expr)
#   define ATOMIC_QUEUE_UNLIKELY(expr)   (expr)
#   define ATOMIC_QUEUE_NOINLINE
#endif

// Common memory orders, shorter aliases to reduce clutter.
constexpr auto A = std::memory_order_acquire;
constexpr auto R = std::memory_order_release;
constexpr auto X = std::memory_order_relaxed;
constexpr auto C = std::memory_order_seq_cst;

} // namespace atomic_queue

#endif // ATOMIC_QUEUE_DEFS_H_INCLUDED
