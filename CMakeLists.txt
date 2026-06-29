cmake_minimum_required(VERSION 3.14)
project(nanosleep LANGUAGES CXX)

# --- Library target ---
# nanosleep is header-only and dependency-free: one header (nanosleep.h) with a
# single inline function, an EINTR-safe wrapper around the POSIX nanosleep() that
# takes a plain nanosecond count and retries until the full interval elapses.
# Nothing but <time.h>/<errno.h> behind it. So the target is a pure INTERFACE
# library that just puts the source dir on the consumer's include path.
add_library(nanosleep INTERFACE)
target_include_directories(nanosleep INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(nanosleep INTERFACE cxx_std_20)
# Alias so consumers can link `nanosleep::nanosleep` whether they add this via
# add_subdirectory or FetchContent.
add_library(nanosleep::nanosleep ALIAS nanosleep)

# Only build the test when this repo is the top-level project, so consumers
# (e.g. via FetchContent) don't build our tests.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  enable_testing()
  add_executable(nanosleep_test test/test.cc)
  target_link_libraries(nanosleep_test PRIVATE nanosleep)
  add_test(NAME nanosleep COMMAND nanosleep_test)
endif()
