add_executable(broker broker.cpp ${proto_src} ${grpc_src})
target_compile_options(broker PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(broker grpc++ grpc++_reflection libprotobuf)