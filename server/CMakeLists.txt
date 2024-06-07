project(server)

add_executable(main main.cpp logger.cpp thread.cpp threadpool.cpp)
target_link_libraries(main pthread)