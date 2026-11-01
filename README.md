# nanosleep

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/nanosleep/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/nanosleep/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

Header-only, EINTR-safe nanosleep for C++20: sleep a nanosecond count and keep sleeping when a signal interrupts.
One inline function wraps POSIX `nanosleep(2)` and calls it again with the time
that is left whenever a signal cuts the sleep short. Reach for it in code that
installs signal handlers and still needs every sleep to last its full interval.

## ✨ Highlights

- **EINTR-safe.** After `EINTR` it sleeps again for the remaining time, so the
  full interval elapses.
- **Plain integers.** `nanosleep(500'000)` takes a nanosecond count; there is no
  `timespec` to fill in.
- **Zero is free.** `nanosleep(0)` returns without a system call.
- **One header, no dependencies** beyond `<time.h>` and `<errno.h>`.

## 🚀 Quick start

```cpp
#include "nanosleep.h"

#include <chrono>
#include <cstdio>

int main() {
	auto start = std::chrono::steady_clock::now();
	nanosleep(500'000ULL);   // 500 µs, even if a signal arrives
	auto elapsed = std::chrono::steady_clock::now() - start;
	std::printf("%d\n", elapsed >= std::chrono::microseconds(500));   // 1
}
```

The name overloads POSIX `nanosleep`: one integer argument selects this wrapper,
and two `timespec*` arguments still reach the system call.

## 📦 Installation

Requires a C++20 compiler, a POSIX system, and CMake 3.14 or newer; no
dependencies. CI builds it on Linux (x86_64, arm64), macOS (arm64, x86_64), and
FreeBSD, plus ASan, UBSan, and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(nanosleep
  GIT_REPOSITORY https://github.com/Kronuz/nanosleep.git
  GIT_TAG        c89453147c6fbbb50929727d6013b1f7ad1732f5)  # or main for the tip
FetchContent_MakeAvailable(nanosleep)

target_link_libraries(my_app PRIVATE nanosleep::nanosleep)
```

```cpp
#include "nanosleep.h"
```

## 🎯 When to use it

Use it for short sleeps that must last the full interval in a process that
handles signals.

Not for:

- **Code that already uses `std::this_thread::sleep_for`.** libstdc++ and libc++
  also retry it on `EINTR`.
- **Error reporting.** It returns nothing; a failure other than `EINTR` ends the
  sleep early without a sign.
- **Windows.** It needs POSIX `nanosleep(2)`.

## 📚 Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md): the retry loop and the two `nanosleep`s.
- [AGENTS.md](AGENTS.md): repository map and traps.
- [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#nanosleep): the catalog entry.

## 🛠️ Building and testing

```sh
cmake -B build
cmake --build build
ctest --test-dir build
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) and the
catalog's [rules of engagement](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#rules-of-engagement)
first; CI must pass on every platform.

## 📄 License

MIT, Copyright (c) 2026 Germán Méndez Bravo and Copyright (c) 2015-2020 Dubalu
LLC. See [LICENSE](LICENSE). Extracted from [Xapiand](https://github.com/Kronuz/Xapiand).
