add_library(action_pp)

target_sources(action_pp PRIVATE action.cpp)

target_link_libraries(action_pp PRIVATE pico_stdlib sdhi midi ymf262)

target_include_directories(action_pp PUBLIC include/)
