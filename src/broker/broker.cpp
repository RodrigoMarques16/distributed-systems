/**
 * Broker for a message oriented architecture
 *
 * Receives connections from publishing processes wanting to provide tagged
 * messages, registers these processes and sends messages to clients who
 * subscribe for messages with that specific tag
 */

#include <string>
#include <unordered_map>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

struct Publisher {
    
};

struct PublisherRegistry{
    std::array<Publisher, 4> registry;
};

int main() {

    std::string server_address("127.0.0.1:5555");
    // PubSubServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    // ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    // builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    // builder.RegisterService(&service);
    // Finally assemble the server.
    // std::unique_ptr<Server> server(builder.BuildAndStart());
    // std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    // server->Wait();
}