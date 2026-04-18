cmake_minimum_required(VERSION 3.25)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5) # Fixes the Mac backward compatibility error
project(lab4_queues)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. DEPENDENCIES
# ==============================================================================
include(FetchContent)
FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(doctest)

# ==============================================================================
# 2. CORE SOURCES
# ==============================================================================
# Define all the files that contain both your logic AND your inline tests.
# Do NOT include main.cpp or test_main.cpp here.
set(CORE_SOURCES
        time.cpp
        time.h
        event.h
        event.cpp
        teller.cpp
        teller.h
)

# ==============================================================================
# 3. MAIN APPLICATION (Production)
# ==============================================================================
# Build the main executable
add_executable(banksim main.cpp ${CORE_SOURCES})

# It needs doctest to resolve the includes in your files...
target_link_libraries(banksim PRIVATE doctest::doctest)

# ...BUT we tell the compiler to completely erase all tests from this executable!
target_compile_definitions(banksim PRIVATE DOCTEST_CONFIG_DISABLE)


# ==============================================================================
# 4. TESTING SUITE (Development)
# ==============================================================================
enable_testing()

# Build a separate executable using test_main.cpp as the entry point
add_executable(banksim_tests test_main.cpp ${CORE_SOURCES})

# Link doctest, but DO NOT disable the tests here
target_link_libraries(banksim_tests PRIVATE doctest::doctest)

# Register the executable with CTest
add_test(NAME RunDoctest COMMAND banksim_tests)