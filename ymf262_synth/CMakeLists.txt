add_library(ymf262_synth)

target_sources(ymf262_synth PRIVATE ymf262_synth.cpp)

target_link_libraries(ymf262_synth PRIVATE pico_stdlib action_pp sdhi ymf262 setup)

target_include_directories(ymf262_synth PUBLIC include/)
