#pragma once
// CMSIS bits used by util_sync.h. The firmware thread and the core ISR share memory like the
// Teensy did; these map the exclusive access instructions to plain loads and stores.
#include <stdint.h>
#include <atomic>

inline void __DMB() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline uint32_t __LDREXW(volatile uint32_t* p) { return *p; }
inline uint32_t __STREXW(uint32_t v, volatile uint32_t* p) { *p = v; return 0; }
inline void __CLREX() {}
