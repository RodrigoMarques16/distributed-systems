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
#include <numeric>

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
using grpc::ServerWriter;
using moa::RegisterRequest;
using moa::RegisterReply;
using moa::SuccessReply;
using moa::TagsRequest;
using moa::TagsReply;
using moa::SubscribeRequest;
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


struct BrokerServiceImpl final : public Broker::Service {
    using vector_pub = std::vector<Publisher>;
    using vector_sub = std::vector<Subscriber>;

    std::string tags_reply_text;

    std::array<vector_pub, 4> publisher_registry;  // not thread-safe
    std::array<vector_sub, 4> subscriber_registry; // not thread-safe
    // MessageDB db(10);   // todo: add constructor

    void build_tags_reply_text() {
        tags_reply_text = std::accumulate(tag_to_str.begin(), tag_to_str.end(), std::string(),
            [](const std::string& a, const std::string& b) -> std::string { 
                return a + (a.length() > 0 ? ", " : "") + b; 
            });
    }

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
            print_message(message);
            // todo: redirection
        }

        std::cout << "End of stream" << std::endl;
        return Status::OK;        
    }

    Status RequestTags(ServerContext* context, const TagsRequest* request, TagsReply* reply) override {
        reply->set_list(std::string(tags_reply_text));
        return Status::OK;
    }

    Status Subscribe(ServerContext* context, const SubscribeRequest* request, ServerWriter<Message>* writer) override {
        Message message;
        message.set_tag(tag_t::TRIAL);
        message.set_id(-1);
        message.set_timestamp(-1);
        message.set_msg("Hello World");

        writer->Write(message);
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
    
    auto server = std::unique_ptr<Server>(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    service.build_tags_reply_text();

    server->Wait();

    return 0;
}