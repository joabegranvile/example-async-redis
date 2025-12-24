#include "RedisClient.h"
#include "Worker.h"
#include <chrono>
#include <iostream>
#include <sw/redis++/errors.h>
#include <vector>

void redisConnect() {
    try {
        RedisClient redis("tcp://localhost:6379");
        redis.ping();
        std::cout << "Connected" << std::endl;

        redis.set("mari", "Mariana Granvile");

        std::cout << redis.get("mari") << std::endl;
        auto v = redis.get("mari");

    } catch (const sw::redis::Error &e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    redisConnect();
    constexpr int WORKERS = 4;
    std::vector<std::unique_ptr<Worker>> workers;

    for (int i = 0; i < WORKERS; i++) {
        workers.push_back(std::make_unique<Worker>("tcp://localhost:6379"));
        workers.back()->start();
    }

    std::this_thread::sleep_for(std::chrono::hours(24));
    return 0;
}
