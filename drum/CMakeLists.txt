add_library(drum)

target_sources(drum PRIVATE drum.c)

target_link_libraries(drum PRIVATE pico_stdlib action sdhi midi setup)

target_include_directories(drum PUBLIC include/)
