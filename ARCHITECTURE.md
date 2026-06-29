# Architecture

The internal design of `nanosleep`: a single inline function that turns the
POSIX `nanosleep(2)` into an EINTR-safe sleep over a plain nanosecond count. For
usage see `README.md`; for the repo map and invariants see `AGENTS.md`.

## Shape

One header, header-only, nothing behind it but `<time.h>` and `<errno.h>`:

```
  nanosleep.h   one inline EINTR-safe wrapper around POSIX nanosleep(2)
```

## What the wrapper does

The whole function is small:

```cpp
inline void nanosleep(unsigned long long nsec) {
    if (nsec > 0) {
        struct timespec ts;
        ts.tv_sec = static_cast<long>(nsec / 1000000000);
        ts.tv_nsec = static_cast<long>(nsec % 1000000000);
        while (nanosleep(&ts, &ts) < 0 && errno == EINTR) { }
    }
}
```

Three things are going on.

First, **the zero guard.** `nsec == 0` returns without touching the kernel. There
is no point building a `timespec` and entering a syscall to sleep for no time.

Second, **the split.** A nanosecond count is divided into whole seconds
(`tv_sec`) and the leftover nanoseconds (`tv_nsec`), because that is the shape
`struct timespec` wants. `tv_nsec` always lands in `[0, 1000000000)` by
construction, which is exactly the range POSIX requires.

Third, **the retry loop.** This is the reason the wrapper exists. `nanosleep(2)`
can return `-1` with `errno == EINTR` when a signal arrives mid-sleep, having
slept only part of the interval. It writes the *remaining* time back into its
second argument. By passing the same `ts` as both the request and the remainder
(`nanosleep(&ts, &ts)`), each interrupted call leaves `ts` holding exactly what is
left, and the loop re-arms with that until the call returns `0` (slept the whole
time) or fails for a reason other than `EINTR`. The net effect is that the
function blocks for the full requested duration regardless of how many signals
land while it runs.

## The two `nanosleep`s

The inline function and the POSIX syscall share the name `nanosleep`, which looks
like recursion but is not. Overload resolution separates them by signature: the
wrapper is `nanosleep(unsigned long long)`, the syscall is
`nanosleep(const struct timespec*, struct timespec*)`. Inside the loop, the
two-argument form can only resolve to the system call, so the wrapper reaches the
real sleep without calling itself. Callers who pass a bare nanosecond count get
the wrapper; callers who pass two `timespec*` get the syscall, unchanged.

## Why nanoseconds, why unsigned long long

The unit is the smallest the syscall supports, so the wrapper imposes no coarser
granularity than the OS itself. `unsigned long long` is wide enough to express
any sleep the kernel will honor (well over a century in nanoseconds) without
signed-overflow worries, and being unsigned it sidesteps the question of what a
negative duration would mean. The `static_cast<long>` on each field matches the
`time_t` / `long` typing of `timespec` on the target platforms.
