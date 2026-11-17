add_executable(index_test index_test.cc)
target_include_directories(index_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(index_test PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME index_test COMMAND index_test)
