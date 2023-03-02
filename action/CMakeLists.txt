add_library(action)

target_sources(action PRIVATE action.c)

target_link_libraries(action PRIVATE pico_stdlib sdhi midi)

target_include_directories(action PUBLIC include/)
