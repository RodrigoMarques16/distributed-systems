add_library(common INTERFACE)
target_compile_options(common INTERFACE -Wall -Wextra)
target_link_libraries(common INTERFACE grpc++)

add_subdirectory(broker)
add_subdirectory(publisher)
add_subdirectory(subscriber)