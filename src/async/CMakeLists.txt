file(GLOB_RECURSE OTTER_ASYNC_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE OTTER_ASYNC_IMPLEMENT_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE OTTER_ASYNC_TEST_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.test.cpp")

list(FILTER OTTER_ASYNC_IMPLEMENT_SOURCE EXCLUDE REGEX "\\.test\\.cpp$")

add_library(otter_async ${OTTER_ASYNC_IMPLEMENT_SOURCE})
add_library(otter::async ALIAS otter_async)

target_compile_features(otter_async PUBLIC cxx_std_23)
target_include_directories(otter_async PUBLIC ${OTTER_INCLUDE_DIRS})
target_link_libraries(otter_async
    PUBLIC
        Microsoft.GSL::GSL
        otter::utility
        PkgConfig::liburing
)

if (${PROJECT_NAME}_BUILD_TESTS AND OTTER_ASYNC_TEST_SOURCE)
    include(Catch)
    
    add_executable(otter_async.test ${OTTER_ASYNC_TEST_SOURCE})
    
    target_link_libraries(otter_async.test
        PRIVATE
            otter::async
            Catch2::Catch2WithMain
    )

    catch_discover_tests(otter_async.test)
endif()