project(Gif2Pid)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        "../../libgd/src"
        "../../hge/include"
)

link_directories(
        "../../hge/linklib/${CMAKE_BUILD_TYPE}"
        "../../libgd/build/Bin/${CMAKE_BUILD_TYPE}"
)

add_executable(Gif2Pid
        "${PROJECT_SOURCE_DIR}/Gif2Pid/main.cpp"
        "${PROJECT_SOURCE_DIR}/../shared/cPID.cpp"
)

target_link_libraries(Gif2Pid libgd.lib hge-static.lib)