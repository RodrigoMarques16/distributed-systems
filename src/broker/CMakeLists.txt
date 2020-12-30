add_executable(broker broker.cpp ${proto_src} ${grpc_src})
target_compile_options(broker PRIVATE -Wall -Wextra)
target_link_libraries(broker common grpc++ grpc++_reflection libprotobuf)