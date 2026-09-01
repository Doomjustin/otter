set(OTTER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(utility)
add_subdirectory(async)
add_subdirectory(net)

add_library(otter INTERFACE)
add_library(otter::otter ALIAS otter)
target_link_libraries(otter 
    INTERFACE
        otter::utility 
        otter::async 
        otter::net
)