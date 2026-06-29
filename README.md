# nanosleep

A small, header-only, dependency-free **EINTR-safe nanosleep** helper for C++20,
extracted from [Xapiand](https://github.com/Kronuz/Xapiand).

## What it is

One header, `nanosleep.h`: a single `inline void nanosleep(unsigned long long nsec)`
that wraps the POSIX `nanosleep(2)` so callers can sleep for a plain nanosecond
count and not worry about signals. The standard call can return early with
`EINTR` when a signal interrupts it, leaving part of the interval unslept; this
wrapper feeds the leftover `timespec` back in and retries until the full
duration has elapsed. Nothing behind it but `<time.h>` and `<errno.h>`.

The interface is just the one function:

```cpp
void nanosleep(unsigned long long nsec);   // sleep for nsec nanoseconds, EINTR-safe
```

A request of `0` is a no-op: it returns immediately without entering the kernel.
Anything larger is split into whole seconds and the remaining nanoseconds and
handed to the system call, which is retried on interruption.

## Install

CMake with `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(
  nanosleep
  GIT_REPOSITORY https://github.com/Kronuz/nanosleep.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(nanosleep)

target_link_libraries(your_target PRIVATE nanosleep::nanosleep)
```

The `nanosleep` target is a pure `INTERFACE` library: it compiles nothing,
requests `cxx_std_20`, and puts the header directory on your include path. Then:

```cpp
#include "nanosleep.h"
```

Requires C++20. On macOS it builds with AppleClang/libc++, the same toolchain
Xapiand uses. The header keeps its original filename, so a codebase that already
`#include "nanosleep.h"` just needs this repo on its include path.

## Usage

```cpp
#include "nanosleep.h"

// Sleep for 500 microseconds, robust against signal interruption:
nanosleep(500000ULL);          // 500 us = 500,000 ns

// Spin-friendly small waits, e.g. a backoff between retries:
nanosleep(100000ULL);          // 100 us

// Zero returns immediately, no syscall:
nanosleep(0);
```

The function name shadows the POSIX `nanosleep`, but the two are distinguished by
their arguments: this wrapper takes a single `unsigned long long`, the system call
takes a `const struct timespec*` and a `struct timespec*`. Call it with a plain
nanosecond count and you get the wrapper; the wrapper itself still reaches the
real syscall internally.

## Build & test

```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

The test calls `nanosleep` for a few small sub-millisecond intervals and checks
the measured elapsed time (via a steady `std::chrono` clock) is at least roughly
the requested duration, and that `nanosleep(0)` returns immediately. It prints
`all nanosleep tests passed` and exits 0.

## Provenance

Extracted from [Xapiand](https://github.com/Kronuz/Xapiand). `nanosleep.h` had
zero dependencies already (only `<time.h>` and `<errno.h>`), so it was copied
verbatim, no decoupling delta. See [ARCHITECTURE.md](ARCHITECTURE.md) for the
design and [AGENTS.md](AGENTS.md) for the repo map and invariants.

## License

MIT, Copyright (c) 2015-2019 Dubalu LLC. See [LICENSE](LICENSE).
