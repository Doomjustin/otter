file(GLOB OTTER_NET_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB OTTER_NET_IMPLEMENT_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB OTTER_NET_TEST_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.test.cpp")

list(FILTER OTTER_NET_IMPLEMENT_SOURCE EXCLUDE REGEX "\\.test\\.cpp$")

add_library(otter_net ${OTTER_NET_IMPLEMENT_SOURCE})
add_library(otter::net ALIAS otter_net)

target_compile_features(otter_net PUBLIC cxx_std_23)
target_include_directories(otter_net PUBLIC ${OTTER_INCLUDE_DIRS})
target_link_libraries(otter_net
    PUBLIC
        spdlog::spdlog
        magic_enum::magic_enum
        Microsoft.GSL::GSL
        otter::async
)

if (${PROJECT_NAME}_BUILD_TESTS AND OTTER_NET_TEST_SOURCE)
    include(Catch)
    
    add_executable(otter_net.test ${OTTER_NET_TEST_SOURCE})
    
    target_link_libraries(otter_net.test
        PRIVATE
            otter::net
            Catch2::Catch2WithMain
    )

    catch_discover_tests(otter_net.test)
endif()