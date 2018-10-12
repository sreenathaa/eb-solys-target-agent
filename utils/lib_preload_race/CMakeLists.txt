cmake_minimum_required(VERSION 2.8.12)

project(PreLoadRace)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-undefined")

find_package (Threads REQUIRED)

add_library(PreLoadRace SHARED
  src/custom_allocs.c
  src/clock_times.c
  src/clock_times.h
  src/main_logic.c
  src/libc_internals.h
  src/thread_storage.c
  src/thread_storage.h
  )

target_include_directories(PreLoadRace
  PRIVATE
    src
  )

target_compile_definitions(PreLoadRace
  PRIVATE
    _GNU_SOURCE
  )

target_link_libraries(PreLoadRace
  PRIVATE
    rt
    ${CMAKE_THREAD_LIBS_INIT}
  )

###############################################################################
# test

add_executable(PreLoadRaceTestReceiver
  test/socket_receiver.c
  )

target_compile_definitions(PreLoadRaceTestReceiver
  PRIVATE
    _GNU_SOURCE
  )

target_link_libraries(PreLoadRaceTestReceiver
  PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
  )

