#include "catch2/catch.hpp"

#include "include/HashTable.hpp"

#include <condition_variable>
#include <thread>


TEST_CASE( "initially there's no key/value pairs", "[HashTable]" ) {
    
    auto table = cskv::HashTable(10);

    REQUIRE(table.get(1) == std::nullopt);
}


TEST_CASE( "HashTable should return the value that was inserted", "[HashTable]" ) {
    
    const unsigned key = 0;
    const std::string expectedValue = "test-value";

    auto table = cskv::HashTable(10);
    table.insert(key, expectedValue);

    REQUIRE(table.get(key) == expectedValue);
}


TEST_CASE( "HashTable should be able to delete an entry", "[HashTable]" ) {
    
    const unsigned key = 0;
    const std::string expectedValue = "test-value";

    auto table = cskv::HashTable(10);
    
    table.insert(key, expectedValue);
    table.del(key);

    REQUIRE(table.get(key) == std::nullopt);
}


TEST_CASE( "In case of Hash collision, HashTable should be able to give back the correct value", "[HashTable]" ) {
    
    // these 2 keys have same hash for Table size of 10.
    const unsigned key1 = 0;
    const unsigned key2 = 10;

    const std::string expectedValue1 = "test-value-1";
    const std::string expectedValue2 = "test-value-2";

    auto table = cskv::HashTable(10);
    
    table.insert(key1, expectedValue1);

    REQUIRE(table.get(key1) == expectedValue1);
    REQUIRE(table.get(key2) == std::nullopt);

    table.insert(key2, expectedValue2);

    REQUIRE(table.get(key1) == expectedValue1);
    REQUIRE(table.get(key2) == expectedValue2);
}


TEST_CASE( "In case of Hash collision, HashTable should be able to delete the correct entry", "[HashTable]" ) {
    
    // these 2 keys have same hash for Table size of 10.
    const unsigned key1 = 0;
    const unsigned key2 = 10;

    const std::string expectedValue1 = "test-value-1";
    const std::string expectedValue2 = "test-value-2";

    auto table = cskv::HashTable(10);
    
    table.insert(key1, expectedValue1);
    table.insert(key2, expectedValue2);
    table.del(key1);

    REQUIRE(table.get(key1) == std::nullopt);
    REQUIRE(table.get(key2) == expectedValue2);
}


TEST_CASE( "Test multithreaded operations on HashTable", "[HashTable]" ) {
    
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;

    cskv::HashTable table (100);


    auto threadFuncs = [&m, &cv, &ready, &table](int startKey, int stopKey) {

        // wait for signal to start
        std::unique_lock lk(m);
        cv.wait(lk, [&ready]{return ready;});
        lk.unlock();
        
        for (int key=startKey; key<stopKey; ++key) {
            table.insert(key, std::to_string(key));
        }


        for (int key=startKey; key<stopKey; ++key) {
            REQUIRE(table.get(key) == std::to_string(key));
        }

        for (int key=startKey; key<stopKey; ++key) {
            table.del(key);
        }

        for (int key=startKey; key<stopKey; ++key) {
            REQUIRE(table.get(key) == std::nullopt);
        }

    };


    // create threads
    std::list<std::thread> threads;
    
    threads.emplace_back( std::thread(threadFuncs, 0, 1000) );
    threads.emplace_back( std::thread(threadFuncs, 1000, 2000) );

    threads.emplace_back( std::thread(threadFuncs, 2000, 4000) );
    threads.emplace_back( std::thread(threadFuncs, 4000, 6000) );

    threads.emplace_back( std::thread(threadFuncs, 6000, 10000) );
    threads.emplace_back( std::thread(threadFuncs, 10000, 14000) );

    // signal run
    {
        std::scoped_lock lk(m);
        ready = true;
    }
    cv.notify_all();


    // wait for threads
    for (auto& t : threads) {
        t.join();
    }
}