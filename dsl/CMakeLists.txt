add_library(dsl)

target_sources(dsl PRIVATE dsl.cpp)

target_link_libraries(dsl PUBLIC pico_stdlib lib sdhi midi ymf262 action_pp)

target_include_directories(dsl PUBLIC include/)
