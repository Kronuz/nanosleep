# nanosleep

[![CI](https://github.com/Kronuz/nanosleep/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/nanosleep/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

`nanosleep(nsec)`: sleep for a plain nanosecond count, and keep sleeping when a
signal interrupts, in C++20. One header around POSIX `nanosleep(2)`.

## ✨ Highlights

- **Signal-safe**: retries with the remaining time after `EINTR`, so the full
  interval elapses.
- **Plain integers**: no `timespec` to fill in.
- **Zero is free**: `nanosleep(0)` returns without a system call.

## 📦 Installation

Requires a C++20 compiler and a POSIX system; CMake 3.14 or newer for the CMake
route. CI builds it on Linux (x86_64, arm64), macOS (arm64, x86_64), and FreeBSD,
plus ASan, UBSan, and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(nanosleep
  GIT_REPOSITORY https://github.com/Kronuz/nanosleep.git
  GIT_TAG        main)
FetchContent_MakeAvailable(nanosleep)

target_link_libraries(my_app PRIVATE nanosleep::nanosleep)
```

## 🚀 Quick start

```cpp
#include "nanosleep.h"

int main() {
	nanosleep(500'000ULL);   // 500 µs, even if a signal arrives
}
```

The name overloads POSIX `nanosleep`; a single `unsigned long long` selects this
one.

## 🎯 When to use it

- ✅ Short, exact sleeps in code that installs signal handlers.
- ❌ Code that already uses `std::this_thread::sleep_for`: it also retries on
  `EINTR` on mainstream implementations.

## 📚 Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md): the retry loop.
- [AGENTS.md](AGENTS.md): repository map and traps.

## 🛠️ Building and testing

```sh
cmake -B build && cmake --build build && ctest --test-dir build
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) first. CI must
pass on every platform.

## 📄 License

MIT, Copyright (c) 2015-2018 Dubalu LLC. See [LICENSE](LICENSE). Extracted from
[Xapiand](https://github.com/Kronuz/Xapiand).
