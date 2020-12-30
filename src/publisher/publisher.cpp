/**
 * Publisher for a message oriented architecture
 *
 * Connects to the broker, registers itself as a provider of messages of a given
 * tag and continues to send messages to the broker
 * 
 * Takes message frequency as a parameter.
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <random>
#include <thread> 
#include <chrono>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include <moa.pb.h>
#include <moa.grpc.pb.h>

#include "../common.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientWriter;
using moa::RegisterRequest;
using moa::RegisterReply;
using moa::Broker;
using moa::tag_t;
using moa::Message;
using moa::SuccessReply;

// volatile sig_atomic_t stopFlag = 0;

// void handler(int){
//     stopFlag = 1;
// }

struct Generator {
    std::exponential_distribution<> dist;
    std::mt19937 gen;

    Generator(double frequence) {
        std::random_device rd;
        gen  = std::mt19937(rd());
        dist = std::exponential_distribution<>(frequence / 3600.0);  
        std::cout << "Generating messages with a frequence of " << frequence << std::endl;
    }

    int64_t next() {
        return dist(gen);
    }
};

struct BrokerClient {
    using Writer = std::unique_ptr<ClientWriter<Message>>;

    size_t message_id = 0;

    tag_t tag;
    std::unique_ptr<Broker::Stub> stub;
    Generator gen;

    Writer writer;

    BrokerClient(tag_t t, int f, std::shared_ptr<Channel> channel) 
        : tag(t)
        , stub(Broker::NewStub(channel))
        , gen(Generator(f)) {}

    std::optional<size_t> register_with_tag(const tag_t tag) {
        RegisterRequest request;
        request.set_tag(tag);

        RegisterReply reply;
        ClientContext context;
        Status status = stub->Register(&context, request, &reply);

        if (status.ok()) {
            return reply.id();
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return std::nullopt;
        }
    }

    Message generate_message() {
        using namespace std::chrono;
        int64_t current_time = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

        Message msg;
        msg.set_tag(tag); 
        msg.set_id(message_id++);
        msg.set_timestamp(current_time);
        msg.set_msg(message_texts[tag]);

        return msg;
    }

    void run() {
        ClientContext context;
        SuccessReply reply;
        writer = Writer(stub->Publish(&context, &reply));

        while(true) {
            auto msg = generate_message();

            if (!writer->Write(msg))
                break;

            auto t = gen.next();
            std::cout << "Next message will be sent after " << t << " seconds" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(t));

            // if (stopFlag) break;
        }

        writer->WritesDone();
        Status status = writer->Finish();
        if (status.ok())
            std::cout << "Finished publishing" << std::endl;
        else
            std::cout << "Exit failure" << std::endl;
    }

};

int main(int argc, char** argv) { 
    if (argc != 4) {
        std::cout << "Usage: publisher [tag] [frequence] [target]\n" 
                  <<    "\ttag: [TRIAL|LICENSE|SUPPORT|BUG]\n"
                  <<    "\tfrequence: How many messages to send per hour on average\n"
                  <<    "\ttarget: Target ip address and port" << std::endl;
        return EXIT_FAILURE;
    }

    auto tag = parse_tag(argv[1]);
    auto freq = atoi(argv[2]);
    auto target = argv[3];

    auto publisher = BrokerClient{tag, freq, grpc::CreateChannel(target, grpc::InsecureChannelCredentials())};
    auto reply = publisher.register_with_tag(tag);
    
    if (!reply.has_value()) {
        std::cout << "Failed to register with the broker" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Sucessfully registered with the broker\n"
              << "My tag is: " << argv[1] << '\n'
              << "My publisher id is: " << reply.value() << std::endl;

    // signal(SIGINT, &handler);

    publisher.run();
}