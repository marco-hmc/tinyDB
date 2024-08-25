enable_testing()
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_SOURCES
    testCase/testCase1/test_case1.cpp
    testCase/testCase2/test_case2.cpp
)

add_executable(runTests ${TEST_SOURCES})

target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
target_link_libraries(runTests gtest gtest_main)
target_link_libraries(runTests spdlog::spdlog)

add_test(NAME Case1Test COMMAND runTests --gtest_filter=Case1Test.* --gtest_output=xml:Case1TestReport.xml)
add_test(NAME Case2Test COMMAND runTests --gtest_filter=Case2Test.* --gtest_output=xml:Case2TestReport.xml)
add_test(NAME Case3Test COMMAND runTests --gtest_filter=Case3Test.* --gtest_output=xml:Case3TestReport.xml)