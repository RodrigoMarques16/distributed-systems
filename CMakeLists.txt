cmake_minimum_required(VERSION 3.15)
project(distributed-systems VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-std=c++20")

find_package(Threads REQUIRED)

## GRPC
# Automatically fetch grpc from github

include(FetchContent)
FetchContent_Declare(
  grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG        v1.33.2
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(grpc)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)

if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

## Generate source files from proto file
get_filename_component(proto "./protos/message.proto" ABSOLUTE)
get_filename_component(proto_path "${proto}" PATH)

set(out_dir "${CMAKE_SOURCE_DIR}/generated")
set(proto_src "${out_dir}/message.pb.cc")
set(proto_hdr "${out_dir}/message.pb.h")
set(grpc_src "${out_dir}/message.grpc.pb.cc")
set(grpc_hdr "${out_dir}/message.grpc.pb.h")

add_custom_command(
    OUTPUT "${proto_src}" "${proto_hdr}" "${grpc_src}" "${grpc_hdr}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${out_dir}"
      --cpp_out "${out_dir}"
      -I "${proto_path}"
      --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
      "${proto}")

    add_custom_target(protos DEPENDS "${proto_src}" "${grpc_src}")

# Include directories
include_directories(generated)
include_directories(lib)
include_directories(include)

add_subdirectory(src)

add_custom_target(my_target)
add_dependencies(my_target client broker server)