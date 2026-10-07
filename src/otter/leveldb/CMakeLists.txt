file(GLOB_RECURSE OTTER_LEVELDB_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE OTTER_LEVELDB_IMPLEMENT_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE OTTER_LEVELDB_TEST_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.test.cpp")

list(FILTER OTTER_LEVELDB_IMPLEMENT_SOURCE EXCLUDE REGEX "\\.test\\.cpp$")

add_library(otter_leveldb ${OTTER_LEVELDB_IMPLEMENT_SOURCE})
add_library(otter::leveldb ALIAS otter_leveldb)

target_compile_features(otter_leveldb PUBLIC cxx_std_23)

target_include_directories(otter_leveldb PUBLIC ${OTTER_INCLUDE_DIR})

target_link_libraries(otter_leveldb PUBLIC otter::utility)

if (${PROJECT_NAME}_BUILD_TESTS AND OTTER_LEVELDB_TEST_SOURCE)
    include(Catch)

    add_executable(otter_leveldb.test ${OTTER_LEVELDB_TEST_SOURCE})

    target_link_libraries(otter_leveldb.test
        PRIVATE
            otter::leveldb
            Catch2::Catch2WithMain
    )

    catch_discover_tests(otter_leveldb.test)
endif()