add_executable(broker broker.cpp)
target_compile_options(broker PUBLIC -Wall -Wextra -Wpedantic)
target_link_libraries(broker grpc++)