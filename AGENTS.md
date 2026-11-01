# AGENTS.md

Working notes for agents modifying this repository. For the design read
`ARCHITECTURE.md`; for usage read `README.md`. This file covers the repo layout,
how to build and test, the invariants you must not break, and the traps that are
easy to fall into.

## Repo map

```
nanosleep.h                  One inline EINTR-safe nanosleep(unsigned long long nsec) wrapping POSIX nanosleep(2). Header. Verbatim from Xapiand.
test/test.cc                 Runnable smoke test: small intervals sleep at least their requested time; nanosleep(0) is a no-op.
CMakeLists.txt               INTERFACE library `nanosleep` (+ alias nanosleep::nanosleep); CTest test `nanosleep`.
LICENSE                      MIT, Copyright (c) 2015-2019 Dubalu LLC.
README.md                    What it is, install, usage.
ARCHITECTURE.md              Internal design: the zero guard, the second/nanosecond split, the EINTR retry loop.
```

Everything is header-only. There is no `.cc` to compile except the test. The
CMake target is a pure `INTERFACE` library that only adds the source dir to the
include path and requests `cxx_std_20`.

## Verification and benchmarks

CI (`.github/workflows/ci.yml`) calls the catalog's shared
[`cmake.yml`](https://github.com/Kronuz/cpp-libraries/blob/main/.github/workflows/cmake.yml) on every push and nightly:
CTest on Linux (x86-64 GCC and Clang, arm64), macOS (arm64, x86-64) and FreeBSD, then again under
ASan+UBSan and TSan. The shared method is the catalog's
[PERFORMANCE.md](https://github.com/Kronuz/cpp-libraries/blob/main/docs/PERFORMANCE.md).

```sh
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
# ASan+UBSan as CI's linux-asan-ubsan job runs it (Linux, Clang)
CC=clang CXX=clang++ cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS" \
  "-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined"
cmake --build build-asan && ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-asan --output-on-failure
```

The test target is `nanosleep_test`, registered in CTest as `nanosleep`; its output ends with
`all nanosleep tests passed`, exit 0. It uses `assert`, which a Release build compiles out; the
default (empty) build type keeps it. The reference is `std::chrono::steady_clock`: 0.1, 0.25 and
0.5 ms sleeps must last at least their length less 20 us, and `nanosleep(0)` must return within
1 ms. No test delivers a signal, so the `EINTR` retry loop rests on review of `nanosleep.h`, not
on a test. There is no bench, fuzz target or mutation script: the library makes no performance
claim.

## Conventions

- **C++20.** The target requests `cxx_std_20` to stay uniform with the sibling
  libraries; the function itself is older-standard-friendly, but don't drop the
  target below it.
- **Zero external dependencies.** The only includes are `<errno.h>` (for `errno`
  and `EINTR`) and `<time.h>` (for `struct timespec` and the POSIX `nanosleep`).
  Don't add anything else.
- **Filename is stable.** The header keeps its original Xapiand name
  (`nanosleep.h`) so a consumer that already `#include`s it just needs this repo
  on the include path. Don't rename it.
- Tabs for indentation, double quotes in code, no em dashes in prose.

## Load-bearing invariants

- **The retry loop is the whole point.** `nanosleep(2)` can return early with
  `EINTR` after a signal, having slept only part of the interval and written the
  remainder back into its second argument. The `while (nanosleep(&ts, &ts) < 0 &&
  errno == EINTR) {}` loop re-arms with that remainder until the full duration is
  served. Don't simplify it to a single un-retried call; that reintroduces the
  bug the wrapper fixes.
- **`&ts` is passed as both request and remainder.** That aliasing is deliberate:
  it lets each interrupted call leave `ts` holding exactly the time left, so the
  next iteration sleeps the rest. Keep passing the same object for both arguments.
- **`nsec == 0` is a no-op.** The `if (nsec > 0)` guard returns immediately
  without building a `timespec` or entering the kernel. Keep it; the test asserts
  the zero path returns essentially instantly.
- **The wrapper and the syscall share a name on purpose.** The one-argument
  `nanosleep(unsigned long long)` and the two-argument POSIX
  `nanosleep(const struct timespec*, struct timespec*)` are distinguished by
  overload resolution. Don't rename the wrapper to "fix" the apparent recursion:
  the two-argument call inside the loop already resolves to the syscall, not to
  the wrapper.

## Traps

- **Don't confuse the two `nanosleep`s.** Inside the function, `nanosleep(&ts,
  &ts)` is the system call (two `timespec*` arguments), not a recursive call to
  this wrapper. If you ever change the wrapper's signature, double-check that the
  inner call still binds to the POSIX function and not to the wrapper.
- **`tv_nsec` must stay in `[0, 1000000000)`.** The split `nsec / 1000000000` and
  `nsec % 1000000000` keeps it there by construction. If you refactor the
  arithmetic, preserve that range or the syscall rejects the request with
  `EINVAL`.
- **Sleeps overshoot, never undershoot.** The OS may sleep slightly longer than
  asked (scheduling granularity), but a non-interrupted sleep won't return early.
  The test asserts a lower bound with a little slack for that reason; don't assert
  an exact duration.

## Standalone vs. Xapiand

This is a standalone extraction from
[Xapiand](https://github.com/Kronuz/Xapiand). `nanosleep.h` had zero dependencies
already (only `<time.h>` and `<errno.h>`), so it was copied verbatim, there is no
decoupling delta. Keep it that way; any change here should be reconcilable with
upstream as a plain edit.
