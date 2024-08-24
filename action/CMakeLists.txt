add_library(action)

target_sources(action PRIVATE action.cpp)

target_link_libraries(action PRIVATE pico_stdlib lib sdhi midi ymf262)

target_include_directories(action PUBLIC include/)
