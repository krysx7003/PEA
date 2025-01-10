cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static ")

add_executable(Proj_1
        Proj_1/Proj_1.cpp
        include/Graph.cpp
        )
add_executable(Proj_2
        Proj_2/Proj_2.cpp
        include/Graph.cpp
        )
target_include_directories(Proj_1 PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Proj_2 PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_compile_options(Proj_1 PRIVATE
        -O0
        -march=x86-64
)
target_compile_options(Proj_2 PRIVATE
        -O0
        -march=x86-64
)

