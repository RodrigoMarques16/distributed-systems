add_executable(publisher publisher.cpp ${proto_src} ${grpc_src})
target_compile_options(publisher PRIVATE -Wall -Wextra)
target_link_libraries(publisher common grpc++ grpc++_reflection libprotobuf)