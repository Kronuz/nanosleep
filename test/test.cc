// Smoke test for the standalone nanosleep library.
//
// Exercises nanosleep.h: the single inline nanosleep(unsigned long long nsec)
// helper, an EINTR-safe wrapper around POSIX nanosleep() that takes a plain
// nanosecond count and retries on interruption until the requested interval has
// elapsed. The test calls it for a few small intervals and checks the measured
// wall-clock time is at least roughly the requested duration. Sleeps can only
// run long (the OS may overshoot, never undershoot a non-interrupted sleep), so
// the lower bound is the meaningful check; intervals are kept tiny so the test
// stays fast.
//
// Build via CMake: cmake -B build && cmake --build build && ctest --test-dir build
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>

#include "nanosleep.h"


// ---------------------------------------------------------------------------
// Measure how long nanosleep(nsec) actually blocks, in nanoseconds, using a
// steady clock so wall-clock adjustments can't skew the reading.
// ---------------------------------------------------------------------------

static unsigned long long measure(unsigned long long nsec) {
	const auto start = std::chrono::steady_clock::now();
	nanosleep(nsec);
	const auto end = std::chrono::steady_clock::now();
	return static_cast<unsigned long long>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}


// ---------------------------------------------------------------------------
// A requested sleep blocks for at least (roughly) the requested time. The OS
// may overshoot but must not return early on a non-interrupted sleep, so the
// lower bound is what we assert. A small slack absorbs clock granularity and
// the cost of the chrono calls themselves on the no-op path.
// ---------------------------------------------------------------------------

static void test_sleeps_at_least() {
	// A few small intervals, all sub-millisecond so the whole test runs fast.
	const unsigned long long intervals[] = { 100000ULL, 250000ULL, 500000ULL };  // 0.1, 0.25, 0.5 ms
	const unsigned long long slack = 20000ULL;  // 20 us of leeway below the target

	for (unsigned long long want : intervals) {
		unsigned long long got = measure(want);
		assert(got + slack >= want);
		std::printf("nanosleep slept >= %llu ns (measured %llu ns)\n", want, got);
	}

	std::printf("nanosleep duration OK: every interval blocked for at least the requested time\n");
}


// ---------------------------------------------------------------------------
// Zero is a no-op: nanosleep(0) takes the early-out branch and returns without
// sleeping. It must come back essentially immediately.
// ---------------------------------------------------------------------------

static void test_zero_is_noop() {
	unsigned long long got = measure(0);
	// No sleep requested, so this should be well under any real sleep interval.
	// Generous bound (1 ms) keeps it robust on a loaded machine.
	assert(got < 1000000ULL);
	std::printf("nanosleep zero OK: nanosleep(0) returned immediately (%llu ns)\n", got);
}


int main() {
	test_zero_is_noop();
	test_sleeps_at_least();
	std::printf("all nanosleep tests passed\n");
	return 0;
}
