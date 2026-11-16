add_executable(hello hello.cc)
target_link_libraries(hello PRIVATE s2::s2 gRPC::grpc++ protobuf::libprotobuf)
