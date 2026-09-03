add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE otter::otter spdlog::spdlog)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE otter::otter)