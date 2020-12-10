add_executable(server server.cpp)
target_compile_options(server PUBLIC -Wall -Wextra -Wpedantic)
target_link_libraries(server grpc++ grpc++_reflection libprotobuf)