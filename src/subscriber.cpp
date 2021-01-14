/**
 * Subscriber client for a message oriented architecture
 *
 * Usage: subscriber [target] [tag]
 * Where: 'tag' is a string denoting which type of messages to receive and is optional
 *        'target' is the adress of the server
 * 
 * This client connects to the server and receives a stream of messages of a single type.
 * If no tag is given then the server will be queried for a list of tags and one will be picked at random.
 * 
 */

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <moa.grpc.pb.h>
#include <moa.pb.h>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "common.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using moa::Broker;
using moa::Empty;
using moa::Message;
using moa::SubscribeRequest;
using moa::TagsReply;

struct SubscriberClient {
    using Reader = std::unique_ptr<ClientReader<Message>>;
    using Stub   = std::unique_ptr<Broker::Stub>;

    Stub stub;
    std::string tag = "";

    SubscriberClient() {}

    SubscriberClient(std::shared_ptr<Channel> channel)
        : stub(Broker::NewStub(channel)) {}

    SubscriberClient(std::string t, std::shared_ptr<Channel> channel)
        : tag(t), stub(Broker::NewStub(channel)) {}

    std::string pick_tag() {
        if (tag != "") return tag;

        std::cout << "Requesting list of tags..." << std::endl;

        ClientContext context;
        Empty request;
        TagsReply reply;
        auto status = stub->RequestTags(&context, request, &reply);

        if (!status.ok()) {
            std::cout << "Failed to connect to the server" << std::endl;    
            exit(EXIT_FAILURE);
        }

        std::cout << "List of tags received\n"
                  << reply.list() << std::endl;

        auto tags = split(reply.list(), ',');
        auto t = tags[rand() % tags.size()];
        std::cout << "Chose tag: " << t << std::endl;
        return t;
    }

    void run() {
        tag = pick_tag();
        
        std::cout << "Subscribing..." << std::endl;
        
        ClientContext context;
        SubscribeRequest sub_request;
        sub_request.set_tag(tag);

        auto reader = Reader(stub->Subscribe(&context, sub_request));

        Message message;
        while (reader->Read(&message)) {
            std::cout << "--------------------\n"
                      << "Received message:\n"
                      << message << std::endl;
        }

        auto status = reader->Finish();
        if (status.ok())
            std::cout << "Finished receiving stream" << std::endl;
        else
            std::cout << "No connection to server" << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        std::cout << "Usage: subscriber [tag] [target]\n"
                  << "\ttarget: Target ip address and port" 
                  << "\ttag: Optional tag" << std::endl;
        return EXIT_FAILURE;
    }

    srand(time(0));

    if (argc == 2)
        SubscriberClient(grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials())).run();
    else 
        SubscriberClient(argv[1], grpc::CreateChannel(argv[2], grpc::InsecureChannelCredentials())).run();
}