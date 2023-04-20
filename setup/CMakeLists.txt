add_library(setup INTERFACE)

target_include_directories(setup INTERFACE include/)

target_link_libraries(setup INTERFACE
        midi
        sdhi
        action
)
