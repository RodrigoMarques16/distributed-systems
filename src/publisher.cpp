/**
 * Publisher client for a Message Oriented Architecture
 * 
 * Usage: publisher [tag] [frequence] [target]
 * Where: 'tag' is the string messages will be tagged with
 *        'frequence' is how many messages should be sent per hour, on average
 *        'target' is the server's address
 * 
 * This client starts by registering with the server as a publisher of its given tag
 * Then it enters an infinite loop, generating messages and sending them to the server,
 * until the server itself closes the connection.
 * 
 * The message distribution is generated by poisson proccess with the helper Generator class.
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

#include <chrono>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "common.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientWriter;
using grpc::Status;
using moa::Broker;
using moa::Empty;
using moa::Message;
using moa::RegisterRequest;

struct Generator {
    std::exponential_distribution<> dist;
    std::mt19937 gen;

    Generator(double frequence) {
        std::random_device rd;
        gen = std::mt19937(rd());
        dist = std::exponential_distribution<>(frequence / 3600.0);
        std::cout << "Generating messages with a frequence of " << frequence << std::endl;
    }

    long next() {
        return dist(gen);
    }
};

struct PublisherClient {
    using Writer = std::unique_ptr<ClientWriter<Message>>;
    using Stub = std::unique_ptr<Broker::Stub>;

    std::string tag;
    size_t message_id = 0;

    Generator gen;
    Stub stub;

    PublisherClient(std::string t, double f, std::shared_ptr<Channel> channel)
        : tag(t), stub(Broker::NewStub(channel)), gen(Generator(f)) {}

    bool try_register() {
        Empty reply;
        ClientContext context;
        RegisterRequest request;
        request.set_tag(tag);
        return (stub->Register(&context, request, &reply)).ok();
    }

    Message generate_message() {
        Message msg;
        msg.set_id(message_id++);
        msg.set_timestamp(get_current_time());
        msg.set_tag(tag);
        msg.set_msg("example");
        return msg;
    }

    void run() {
        ClientContext context;
        Empty reply;
        auto writer = Writer(stub->Publish(&context, &reply));

        std::cout << "Registering..." << std::endl;
        if (!try_register()) {
            std::cout << "Failed to register with the broker" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Sucessfully registered with the broker\n";

        while (true) {
            auto msg = generate_message();
            std::cout << "--------------------\n"
                      << "Sending message:\n"
                      << msg << std::endl;

            if (!writer->Write(msg))
                break;

            auto t = gen.next();
            std::cout << "Next message will be sent after " << t << " seconds" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(t));
        }

        writer->WritesDone();

        auto status = writer->Finish();
        if (status.ok())
            std::cout << "Finished publishing" << std::endl;
        else
            std::cout << "Server closed connection" << std::endl;
    }
};

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: publisher [tag] [frequence] [target]\n"
                  << "\ttag: [TRIAL|LICENSE|SUPPORT|BUG]\n"
                  << "\tfrequence: How many messages to send per hour on average\n"
                  << "\ttarget: Target ip address and port" << std::endl;
        return EXIT_FAILURE;
    }

    auto tag = argv[1];
    auto freq = atoi(argv[2]);
    auto target = argv[3];

    auto publisher = PublisherClient(tag, (double)freq, grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    publisher.run();
}