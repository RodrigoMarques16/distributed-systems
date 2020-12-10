add_executable(client client.cpp ${proto_src} ${grpc_src})
target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(client grpc++ grpc++_reflection libprotobuf)