add_executable(server server.cpp ${proto_src} ${grpc_src})
target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(server grpc++ grpc++_reflection libprotobuf)