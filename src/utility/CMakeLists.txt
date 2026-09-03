file(GLOB_RECURSE OTTER_UTILITY_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE OTTER_UTILITY_IMPLEMENT_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE OTTER_UTILITY_TEST_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.test.cpp")

list(FILTER OTTER_UTILITY_IMPLEMENT_SOURCE EXCLUDE REGEX "\\.test\\.cpp$")

add_library(otter_utility ${OTTER_UTILITY_IMPLEMENT_SOURCE})
add_library(otter::utility ALIAS otter_utility)

target_compile_features(otter_utility PUBLIC cxx_std_23)
target_include_directories(otter_utility PUBLIC ${OTTER_INCLUDE_DIRS})
target_link_libraries(otter_utility
    PUBLIC
        spdlog::spdlog
        magic_enum::magic_enum
        Microsoft.GSL::GSL
)

if (${PROJECT_NAME}_BUILD_TESTS AND OTTER_UTILITY_TEST_SOURCE)
    include(Catch)
    
    add_executable(otter_utility.test ${OTTER_UTILITY_TEST_SOURCE})
    
    target_link_libraries(otter_utility.test
        PRIVATE
            otter::utility
            Catch2::Catch2WithMain
    )

    catch_discover_tests(otter_utility.test)
endif()