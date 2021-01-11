/**
 * Subscriber for a message oriented architecture
 *
 * Connects to the broker, asks for a list of message tags, subscribes to a
 * given tag and continues to receive messages with that tag sent by the broker
 */

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <moa.grpc.pb.h>
#include <moa.pb.h>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

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

    SubscriberClient(std::shared_ptr<Channel> channel)
        : stub(Broker::NewStub(channel)) {}

    std::string pick_tag() {
        std::cout << "Requesting list of tags" << std::endl;

        ClientContext context;
        Empty request;
        TagsReply reply;

        stub->RequestTags(&context, request, &reply);

        std::cout << "List of tags received\n"
                  << reply.list() << std::endl;

        auto tags = split(reply.list(), ',');
        auto tag = tags[rand() % tags.size()];

        std::cout << "Chose tag: " << tag << std::endl;

        return tag;
    }

    void run() {
        auto tag = pick_tag();

        std::cout << "Subscribing..." << std::endl;

        ClientContext context;
        SubscribeRequest sub_request;
        sub_request.set_tag(tag);

        auto reader = Reader(stub->Subscribe(&context, sub_request));

        std::cout << "Receiving stream of messages" << std::endl;

        Message message;
        while (reader->Read(&message)) {
            std::cout << "--------------------\n"
                      << "Received message:\n";
            print_message(message);
        }

        auto status = reader->Finish();
        if (status.ok())
            std::cout << "Finished receiving stream" << std::endl;
        else
            std::cout << "Server closed connection" << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: subscriber [target]\n"
                  << "\ttarget: Target ip address and port" << std::endl;
        return EXIT_FAILURE;
    }

    auto target = argv[1];

    auto subscriber = SubscriberClient(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    subscriber.run();
}