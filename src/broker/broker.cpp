/**
 * Broker for a message oriented architecture
 *
 * Receives connections from publishing processes wanting to provide tagged
 * messages, registers these processes and sends messages to clients who
 * subscribe for messages with that specific tag
 */

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <mutex>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include <moa.pb.h>
#include <moa.grpc.pb.h>

#include "../common.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using moa::RegisterRequest;
using moa::RegisterReply;
using moa::SuccessReply;
using moa::Message;
using moa::Broker;
using moa::tag_t;

struct Publisher {
    tag_t tag;
    std::string address;
};

struct Subscriber {
    std::string address;
};

// struct MessageDB {
//     using vector_type = std::vector<Message>;
//     // std::array<vector_type, 4> db;
//     long TTL; // how long to keep messages around for

//     MessageDB(int t) : TTL(t) {}

//     void write(const tag_t tag, const Message& s) {
//         // todo: thread-safety
//         // db[tag].emplace_back(s);
//     }

//     void clear_expired() {}
// };

class BrokerServiceImpl final : public Broker::Service {
    using vector_pub = std::vector<Publisher>;
    using vector_sub = std::vector<Subscriber>;
    
    std::array<vector_pub, 4> publisher_registry;  // not thread-safe
    std::array<vector_sub, 4> subscriber_registry; // not thread-safe
    // MessageDB db(10);   // todo: add constructor

    Status Register(ServerContext* context, const RegisterRequest* request, RegisterReply* reply) override {
        auto tag = request->tag();
        auto id  = publisher_registry[tag].size() + 1;

        publisher_registry[tag].emplace_back(Publisher{tag, ""});
        reply->set_id(id);
        return Status::OK;
    }
    Status Publish(ServerContext* context, ServerReader<Message>* reader, SuccessReply* reply) override {
        Message message;
        while(reader->Read(&message)){
            std::cout << "Received message: \n"
                      << message.tag() << "\n"
                      << tag_to_str[message.tag()] << "\n"
                      << message.timestamp() << "\n"
                      << message.msg() << std::endl;
        
            // todo: redirection
        }

        std::cout << "End of stream" << std::endl;
        return Status::OK;        
    }
};

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    BrokerServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();

    return 0;
}