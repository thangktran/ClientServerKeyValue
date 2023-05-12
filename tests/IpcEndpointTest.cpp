#include "catch2/catch.hpp"

#include "include/IpcEndpoint.hpp"

#include <thread>
#include <vector>


TEST_CASE( "simple client server communication. 1 thread runs client and 1 thread runs server", "[IpcEndpoint]" ) {
    
    const std::string ENDPOINT_NAME = "TestEndPoint";
    const int BUFFER_SIZE = 1024;

    auto serverFunc = [&ENDPOINT_NAME, BUFFER_SIZE](int nRounds) {

        auto server = cskv::IpcEndpoint(ENDPOINT_NAME, true);
        char buff[BUFFER_SIZE];

        int count = 0;

        while (count++ < nRounds) {

            auto readLen = server.readReq(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);

            std::string req = std::string(buff, readLen.value());
            std::string res = req + "_serverRes";
            auto writeLen = server.writeRes(res.c_str(), res.length());
            REQUIRE(writeLen == res.length());


            std::string serverReq = req + "_serverReq";

            readLen = server.writeReq(serverReq.c_str(), serverReq.length());
            REQUIRE(readLen == serverReq.length());

            readLen = server.readRes(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);
            
            std::string clientRes = std::string(buff, readLen.value());
            const std::string expectedClientRes = serverReq + "_clientRes";
            REQUIRE(clientRes == expectedClientRes);
        }
    };

    auto clientFunc = [&ENDPOINT_NAME, BUFFER_SIZE](int nRounds) {

        auto client = cskv::IpcEndpoint(ENDPOINT_NAME);
        char buff[BUFFER_SIZE];

        std::uniform_int_distribution<int> dist(1, 1000);
        std::mt19937 engine;
        auto diceRoller = std::bind(dist, engine);

        int count = 0;

        while (count++ < nRounds) {
            
            std::string req = std::to_string(diceRoller());

            auto writeLen = client.writeReq(req.c_str(), req.length());
            REQUIRE(writeLen == req.length());

            auto readLen = client.readRes(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);

            std::string res = std::string(buff, readLen.value());
            std::string expectedRes = req + "_serverRes";
            REQUIRE(res == expectedRes);

            readLen = client.readReq(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);

            std::string serverReq = std::string(buff, readLen.value());
            std::string clientRes = serverReq + "_clientRes";
            writeLen = client.writeRes(clientRes.c_str(), clientRes.length());
            REQUIRE(writeLen == clientRes.length());
        }
    };

    const int nRounds = 1000;
    std::thread serverThread(serverFunc, nRounds);
    std::thread clientThread(clientFunc, nRounds);

    serverThread.join();
    clientThread.join();
}


TEST_CASE( "simple client server communication. 1000 threads run client and 1 thread runs server", "[IpcEndpoint]" ) {
    
    const std::string ENDPOINT_NAME = "TestEndPoint";
    const int BUFFER_SIZE = 1024;

    auto serverFunc = [&ENDPOINT_NAME, BUFFER_SIZE](int nRounds) {

        auto server = cskv::IpcEndpoint(ENDPOINT_NAME, true);
        char buff[BUFFER_SIZE];

        int count = 0;

        while (count++ < nRounds) {

            auto readLen = server.readReq(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);

            std::string req = std::string(buff, readLen.value());
            std::string res = req + "_serverRes";
            auto writeLen = server.writeRes(res.c_str(), res.length());
            REQUIRE(writeLen == res.length());

        }
    };


    auto clientFunc = [&ENDPOINT_NAME, BUFFER_SIZE](int nRounds) {

        auto client = cskv::IpcEndpoint(ENDPOINT_NAME);
        char buff[BUFFER_SIZE];

        std::uniform_int_distribution<int> dist(1, 1000);
        std::mt19937 engine;
        auto diceRoller = std::bind(dist, engine);

        int count = 0;

        while (count++ < nRounds) {
            
            std::string req = std::to_string(diceRoller());

            auto writeLen = client.writeReq(req.c_str(), req.length());
            REQUIRE(writeLen == req.length());

            auto readLen = client.readRes(buff, BUFFER_SIZE);
            REQUIRE(readLen != std::nullopt);

            std::string res = std::string(buff, readLen.value());
            std::string expectedRes = req + "_serverRes";
            REQUIRE(res == expectedRes);
        }
    };

    const int nRounds = 1000;
    const int nClientThreads = 1000;
    
    std::thread serverThread(serverFunc, nRounds*nClientThreads);
    std::vector<std::thread> clientThreads;

    for (int i=0; i<nClientThreads; ++i) {
        clientThreads.emplace_back(clientFunc, nRounds);
    }

    serverThread.join();
    for (auto& t : clientThreads) {
        t.join();
    }
}