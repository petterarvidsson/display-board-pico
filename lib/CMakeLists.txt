add_library(lib)

target_link_libraries(lib PRIVATE pico_stdlib)

target_include_directories(lib PUBLIC include/)
