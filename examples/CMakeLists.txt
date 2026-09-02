add_executable(vk_ctx_example0
    example0.cpp
)

add_executable(vk_ctx_example1
    example1.cpp
)

add_executable(vk_ctx_example2
    example2.cpp
)

target_link_libraries(vk_ctx_example0 PRIVATE vk_ctx_lib)
target_link_libraries(vk_ctx_example1 PRIVATE vk_ctx_lib)
target_link_libraries(vk_ctx_example2 PRIVATE vk_ctx_lib)
