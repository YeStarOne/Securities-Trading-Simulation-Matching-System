cmake_minimum_required(VERSION 3.5)

PROJECT(TransServer)

file(
        GLOB_RECURSE sources
        "pool/*.cpp"
        "timer/*.cpp"
        "http/*.cpp"
        "log/*.cpp"
        "server/*.cpp"
        "buffer/*.cpp"
)

ADD_EXECUTABLE(
        TransServer
        main.cpp
        ${sources}
)

TARGET_LINK_LIBRARIES(
        TransServer
        pthread
        mysqlclient
)

