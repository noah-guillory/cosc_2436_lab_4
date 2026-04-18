cmake_minimum_required(VERSION 3.25)
project(lab4_queues)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(banksim main.cpp
        time.cpp
        time.h
        event.h
        teller.cpp
        teller.h
        event.cpp
)
