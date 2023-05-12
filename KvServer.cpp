#include "include/HashTable.hpp"
#include "include/IpcEndpoint.hpp"
#include "include/ComProtocol.hpp"

#include "third_party/cxxopts.hpp"

#include <thread>
#include <list>
#include <iostream>
#include <cstring>
#include <cassert>


int main(int argc, const char* argv[]) {

    cxxopts::Options options(argv[0], "Key Value server");

	options
		.allow_unrecognised_options()
		.add_options()
			("s, size", "size of HashTable", cxxopts::value<size_t>())
			("h, help", "Help");

	auto args = options.parse(argc, argv);

	if (args.count("help")) {
		std::cout << options.help() << std::endl;
		return 0;
	}

    if (!args.count("size"))
	{
		std::cout << "-s arg is required" << std::endl;
		return 1;
	}

    size_t hashSize = args["size"].as<size_t>();

    cskv::HashTable table(hashSize);

    auto workerFunc = [&table](std::string_view clientEndpoint) {

        auto endpoint = cskv::IpcEndpoint(clientEndpoint);

        bool exit = false;

        while(not exit) {

            cskv::protocol::KvRequest req;
            cskv::protocol::KvResponse res;
            auto readLen = endpoint.readReq(&req, sizeof(req));

            if (not readLen or readLen != sizeof(req)) {
                std::cerr << "error reading req" << std::endl;
                continue;
            }

            switch (req.cmd)
            {
            case cskv::protocol::KvCommand::FINISH:
                res.success = true;
                exit = true;
                break;

            case cskv::protocol::KvCommand::INSERT:
                res.success = table.insert(req.key, std::string(req.value, req.len));
                break;

            
            case cskv::protocol::KvCommand::DEL:
                res.success = true;
                table.del(req.key);
                break;

            
            case cskv::protocol::KvCommand::GET:
            {
                auto value = table.get(req.key);
                if (not value) {
                    res.success = false;
                    break;
                }

                res.len = value.value().length();
                assert(res.len < cskv::protocol::VALUE_SIZE);
                std::memcpy(res.value, value.value().c_str(), res.len);
                res.success = true;
            }
                break;

            
            default:
                std::cerr << "invalid cmd" << std::endl;
                break;
            }

            auto writtenLen = endpoint.writeRes(&res, sizeof(res));
            writtenLen = 0;
        }
    };

    std::list<std::thread> workerThreads;
    auto listenServer = cskv::IpcEndpoint(cskv::protocol::LISTEN_SERVER, true);

    bool exit = false;
    while (not exit) {

        cskv::protocol::ConnectionRequest req;
        auto readLen = listenServer.readReq(&req, sizeof(req));

        if (not readLen or readLen != sizeof(req)) {
            std::cerr << "error reading req" << std::endl;
            continue;
        }

        std::string endpointName = std::string(req.name, req.len);

        if (endpointName.compare(cskv::protocol::SPECIAL_EXIT_SEQUENCE) == 0) {
            exit = true;
        }
        else {
            workerThreads.emplace_back(workerFunc, endpointName);
        }

        cskv::protocol::ConnectionResponse res;
        res.success = true;

        auto writtenLen = listenServer.writeRes(&res, sizeof(res));


    }

    for (auto& t : workerThreads) {
        t.join();
    }

    return 0;
}