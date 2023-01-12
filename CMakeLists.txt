cmake_minimum_required(VERSION 3.8)

add_library(folder_watcher)
add_library(folder_watcher::folder_watcher ALIAS folder_watcher)
target_compile_features(folder_watcher PUBLIC cxx_std_20)

# ---Add source files---
target_include_directories(folder_watcher PUBLIC include)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
target_sources(folder_watcher PRIVATE ${SRC_FILES})

# ---Set warning level---
if(MSVC)
    target_compile_options(folder_watcher PRIVATE /W4)
else()
    target_compile_options(folder_watcher PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion)
endif()

# ---Maybe enable warnings as errors---
set(WARNINGS_AS_ERRORS_FOR_FOLDER_WATCHER OFF CACHE BOOL "ON iff you want to treat warnings as errors")
if(WARNINGS_AS_ERRORS_FOR_FOLDER_WATCHER)
    if(MSVC)
        target_compile_options(folder_watcher PRIVATE /WX)
    else()
        target_compile_options(folder_watcher PRIVATE -Werror)
    endif()
endif()
