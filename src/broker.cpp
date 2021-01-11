#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <moa.grpc.pb.h>
#include <moa.pb.h>

#include <array>
#include <chrono>
#include <deque>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "database.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::Status;

using moa::Broker;
using moa::Empty;
using moa::Message;
using moa::RegisterRequest;
using moa::SubscribeRequest;
using moa::TagsReply;

using Writer = ServerWriter<Message>;

struct Subscriber {
    tag_t tag;
    std::string uri;
    Writer* writer;

    Subscriber(tag_t tag, std::string uri, Writer* writer)
        : tag(tag), uri(uri), writer(writer){};
};

struct BrokerServiceImpl final : public Broker::Service {
    using vector_sub = std::vector<Subscriber>;

    std::array<vector_sub, 4> subscribers;
    std::array<std::mutex, 4> mutexes;

    std::string tags_reply_text;

    Database<Message> db;

    BrokerServiceImpl(long ttl) : db(ttl) {};

    void build_tags_reply_text() {
        tags_reply_text = std::accumulate(
            tag_to_str.begin(), tag_to_str.end(), std::string(),
            [](const std::string& a, const std::string& b) -> std::string {
                return a + (a.length() > 0 ? "," : "") + b;
            });
    }

    // --- Publisher implementations

    Status Register(ServerContext* context, const RegisterRequest* request, Empty* reply) override {
        auto tag = request->tag();
        std::cout << "Registered publisher: "
                  << "uri=" << context->peer() 
                  << "tag=" << tag << std::endl;
        return Status::OK;
    }

    Status Publish(ServerContext* context, ServerReader<Message>* reader, Empty* reply) override {
        Message message;
        while (reader->Read(&message)) {
            auto tag = parse_tag(message.tag());
            if (!tag.has_value())
                return Status::CANCELLED;

            auto subs = subscribers[*tag]; // copy
            auto& mutex = mutexes[*tag];

            std::cout << "--------------------\n"
                      << "Broadcasting to tag " << message.tag() << " | "
                      << subs.size() << " subscribers\n"
                      << "publisher=" << context->peer() << "; "
                      << "messageid=" << message.id() << std::endl;

            std::unique_lock lk(mutex);
            for (auto& sub : subs) {
                std::cout << "Sending to " << sub.uri << std::endl;
                sub.writer->Write(message);
            }
            lk.unlock();

            db.write(*tag, message);
            std::cout << "There are " << db.size(*tag) << " messages for this tag in the database\n"
                      << "--------------------" << std::endl;
        }
        std::cout << "Publisher " << context->peer() << " disconnected" << std::endl;
        return Status::OK;
    }

    // --- Subscriber implementations

    Status RequestTags(ServerContext* context, const Empty* request, TagsReply* reply) override {
        std::cout << "Received request for tags from " << context->peer() << std::endl;
        reply->set_list(std::string(tags_reply_text));
        return Status::OK;
    }

    Subscriber register_subscriber(tag_t tag, std::string id, Writer* writer) {
        std::unique_lock lk(mutexes[tag]);
        return subscribers[tag].emplace_back(tag, id, writer);
    }

    void unregister_subscriber(tag_t tag, std::string id) {
        auto& subs = subscribers[tag];
        std::unique_lock lk(mutexes[tag]);
        subs.erase(std::remove_if(subs.begin(), subs.end(), 
            [id](const auto& sub) { return sub.uri == id; }));
    }

    void send_message_history(tag_t tag, Writer* writer) {
        auto old_messages = db.read(tag);
        std::cout << "There are " << old_messages.size() << " unread messages" << std::endl;
        for (auto& msg : old_messages)
            writer->Write(msg);
    }

    Status Subscribe(ServerContext* context, const SubscribeRequest* request, Writer* writer) override {
        auto tag = parse_tag(request->tag());
        if (!tag.has_value())
            return Status::CANCELLED;

        auto uri = context->peer();
        auto& subs = subscribers[*tag];
        auto& mutex = mutexes[*tag];

        register_subscriber(*tag, uri, writer);
        send_message_history(*tag, writer);

        // Loop until connection is closed
        while (context->IsCancelled() == false)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::cout << "Subscriber " << uri << " disconnected" << std::endl;

        unregister_subscriber(*tag, uri);

        return Status::OK;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: subscriber [ttl]\n"
                  << "\tttl: How long to keep messages stored for" << std::endl;
        return EXIT_FAILURE;
    }

    std::string server_address("0.0.0.0:50051");
    BrokerServiceImpl service((time_t) atoi(argv[1]));

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