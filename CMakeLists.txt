cmake_minimum_required(VERSION 3.8)

project(Jag)


find_package(Threads
    REQUIRED)

find_package(Boost
    REQUIRED
    COMPONENTS
        filesystem
        iostreams
        system
        program_options)


add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(jag src/main.cpp)

target_compile_features(jag PRIVATE cxx_std_17)

target_include_directories(jag PUBLIC
    "${Boost_INCLUDE_DIRS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(jag
    "${CMAKE_DL_LIBS}"
    "${CMAKE_THREAD_LIBS_INIT}"
    "${Boost_LIBRARIES}")


set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,thread,undefined")
set(CMAKE_LINKER_FLAGS_DEBUG
    "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,thread,undefined")
