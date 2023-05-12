#include "include/IpcEndpoint.hpp"
#include "include/ComProtocol.hpp"

#include "third_party/cxxopts.hpp"

#include <unistd.h>
#include <cstring>
#include <random>
#include <functional>
#include <cassert>


int main(int argc, const char* argv[]) {

    cxxopts::Options options(argv[0], "Key Value client");

    options
        .allow_unrecognised_options()
        .add_options()
            ("n, numRound", "number of rounds the client will run", cxxopts::value<unsigned>())
            ("k, key", "key that this client will operate on", cxxopts::value<unsigned>())
            ("e, exit", "signal the server to exit. This proram will terminate immediately.", cxxopts::value<bool>())
            ("h, help", "Help");


    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    bool exitRequested = false;
    unsigned nRounds = 0;
    unsigned key = 0;

    if(args.count("exit")) {
        exitRequested = true;
    }
    else {
        if (!args.count("numRound"))
        {
            std::cout << "-n arg is required" << std::endl;
            return 1;
        }

        if (!args.count("key"))
        {
            std::cout << "-k arg is required" << std::endl;
            return 1;
        }

        nRounds = args["numRound"].as<unsigned>();
        key = args["key"].as<unsigned>();
    }

    unsigned idx = 0;

    std::string THIS_ENDPOINT_NAME;
    
    if (exitRequested) {
        THIS_ENDPOINT_NAME = cskv::protocol::SPECIAL_EXIT_SEQUENCE;
    }
    else {
        THIS_ENDPOINT_NAME = std::to_string(getpid()) + "_KV_CLIENT";
    }

    auto endpoint = cskv::IpcEndpoint(THIS_ENDPOINT_NAME, true);

    auto listenServer = cskv::IpcEndpoint(cskv::protocol::LISTEN_SERVER);
    cskv::protocol::ConnectionRequest conReq;
    cskv::protocol::ConnectionResponse conRes;
    
    conReq.len = THIS_ENDPOINT_NAME.length();
    assert(conReq.len < cskv::protocol::VALUE_SIZE);
    std::memcpy(conReq.name, THIS_ENDPOINT_NAME.c_str(), conReq.len);

    auto conWrittenLen = listenServer.writeReq(&conReq, sizeof(conReq));
    assert(conWrittenLen == sizeof(conReq));

    auto conReadLen = listenServer.readRes(&conRes, sizeof(conRes));
    assert(conReadLen == sizeof(conRes));
    assert(conRes.success);

    if (exitRequested) {
        return 0;
    }

    std::random_device rd;
    std::uniform_int_distribution<int> dist(1, 1000);
    std::mt19937 engine(rd());
    auto diceRoller = std::bind(dist, engine);


    while (idx++ < nRounds) {

        std::string value = std::to_string(diceRoller());
        cskv::protocol::KvRequest req;
        cskv::protocol::KvResponse res;

        auto doInsert = [&req, &res, key, &value, &endpoint]() {
            req.cmd = cskv::protocol::KvCommand::INSERT;
            req.key = key;
            req.len = value.length();
            assert(req.len < cskv::protocol::VALUE_SIZE);
            std::memcpy(req.value, value.c_str(), req.len);

            auto writtenLen = endpoint.writeReq(&req, sizeof(req));
            assert(writtenLen == sizeof(req));

            auto readLen = endpoint.readRes(&res, sizeof(res));
            assert(readLen == sizeof(res));
            assert(res.success);
        };

        auto doGet = [&req, &res, key, &value, &endpoint]() {
            req.cmd = cskv::protocol::KvCommand::GET;
            req.key = key;

            auto writtenLen = endpoint.writeReq(&req, sizeof(req));
            assert(writtenLen == sizeof(req));

            auto readLen = endpoint.readRes(&res, sizeof(res));
            assert(readLen == sizeof(res));
            assert(res.success);
            std::string receivedValue = std::string(res.value, res.len);
            assert(receivedValue == value);
        };

        // ------- insert operation --------
        doInsert();


        // ------- get operation --------
        doGet();


        // replace
        value = value + "_replaced";
        doInsert();


        // get
        doGet();


        // del
        req.cmd = cskv::protocol::KvCommand::DEL;
        req.key = key;

        auto writtenLen = endpoint.writeReq(&req, sizeof(req));
        assert(writtenLen == sizeof(req));

        auto readLen = endpoint.readRes(&res, sizeof(res));
        assert(readLen == sizeof(res));
        assert(res.success);
        std::string receivedValue = std::string(res.value, res.len);
        assert(receivedValue == value);


        // get
        req.cmd = cskv::protocol::KvCommand::GET;
        req.key = key;

        writtenLen = endpoint.writeReq(&req, sizeof(req));
        assert(writtenLen == sizeof(req));

        readLen = endpoint.readRes(&res, sizeof(res));
        assert(readLen == sizeof(res));
        assert(not res.success);
    }

    // signal finish.
    cskv::protocol::KvRequest req;
    cskv::protocol::KvResponse res;
    req.cmd = cskv::protocol::KvCommand::FINISH;

    auto writtenLen = endpoint.writeReq(&req, sizeof(req));
    assert(writtenLen == sizeof(req));

    auto readLen = endpoint.readRes(&res, sizeof(res));
    assert(readLen == sizeof(res));
    assert(res.success);

}