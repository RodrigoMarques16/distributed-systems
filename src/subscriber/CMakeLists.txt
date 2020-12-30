add_executable(subscriber subscriber.cpp ${proto_src} ${grpc_src})
target_compile_options(subscriber PRIVATE -Wall -Wextra)
target_link_libraries(subscriber common grpc++ grpc++_reflection libprotobuf)