file(GLOB_RECURSE OTTTER_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE OTTER_IMPLEMENT_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE OTTER_TEST_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.test.cpp")

list(FILTER OTTER_IMPLEMENT_SOURCE EXCLUDE REGEX "\\.test\\.cpp$")

add_library(otter ${OTTER_IMPLEMENT_SOURCE})
add_library(otter::otter ALIAS otter)

target_compile_features(otter PUBLIC cxx_std_23)
target_include_directories(otter PUBLIC ${OTTER_INCLUDE_DIR})
target_link_libraries(otter
    PUBLIC
        Microsoft.GSL::GSL
        fmt::fmt
        spdlog::spdlog
)

if (${PROJECT_NAME}_BUILD_TESTS AND OTTER_TEST_SOURCE)
    include(Catch)

    add_executable(otter.test ${OTTER_TEST_SOURCE})

    target_link_libraries(otter.test
        PRIVATE
            Catch2::Catch2WithMain
    )

    catch_discover_tests(otter.test)
endif()