add_executable(vk_ctx_instance_tests
    instance_tests.cpp
)

add_executable(vk_ctx_surface_tests
    surface_tests.cpp
)

add_executable(vk_ctx_device_tests
    device_tests.cpp
)

target_link_libraries(vk_ctx_instance_tests
    PRIVATE
        vk_ctx_lib
        GTest::gtest_main
)

target_link_libraries(vk_ctx_surface_tests
    PRIVATE
        vk_ctx_lib
        GTest::gtest_main
)

target_link_libraries(vk_ctx_device_tests
    PRIVATE
        vk_ctx_lib
        GTest::gtest_main
)

target_include_directories(vk_ctx_device_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)
gtest_discover_tests(vk_ctx_instance_tests)
gtest_discover_tests(vk_ctx_surface_tests)
gtest_discover_tests(vk_ctx_device_tests)

add_test(NAME vk_ctx_instance_tests COMMAND vk_ctx_instance_tests)
add_test(NAME vk_ctx_surface_tests COMMAND vk_ctx_surface_tests)
add_test(NAME vk_ctx_device_tests COMMAND vk_ctx_device_tests)
