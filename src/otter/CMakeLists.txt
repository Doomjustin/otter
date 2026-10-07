add_subdirectory(utility)


add_library(otter INTERFACE)
add_library(otter::otter ALIAS otter)

target_include_directories(otter INTERFACE ${OTTER_INCLUDE_DIR})

target_link_libraries(otter INTERFACE
    otter::utility
)