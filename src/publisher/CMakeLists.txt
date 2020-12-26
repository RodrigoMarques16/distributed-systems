add_executable(publisher publisher.cpp ${proto_src} ${grpc_src})
target_compile_options(publisher PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(publisher grpc++ grpc++_reflection libprotobuf)