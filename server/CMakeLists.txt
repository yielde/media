project(server)

add_executable(main main.cpp logger.cpp thread.cpp threadpool.cpp httpparser.cpp http_parser.c)
target_link_libraries(main pthread)