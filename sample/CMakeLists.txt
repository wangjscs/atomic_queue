CMAKE_MINIMUM_REQUIRED( VERSION 3.22 )

PROJECT(sample VERSION 1.5.0)
find_package(Threads REQUIRED)

add_executable(sample
    sample.cpp
)
target_include_directories(sample
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(sample PRIVATE Threads::Threads)
