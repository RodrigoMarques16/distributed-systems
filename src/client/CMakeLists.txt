add_executable(client client.cpp)
target_compile_options(client PUBLIC -Wall -Wextra -Wpedantic)
target_link_libraries(client grpc++)