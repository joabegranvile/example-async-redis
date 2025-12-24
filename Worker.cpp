#include "Worker.h"
#include <chrono>
#include <iostream>
#include <iterator>
#include <string>
#include <sw/redis++/command_options.h>
#include <sw/redis++/errors.h>
#include <sw/redis++/redis++.h>
#include <thread>
using namespace sw::redis;

Worker::Worker(const std::string &redis_url) : redis_url(redis_url) {}

void Worker::start() { thread_ = std::thread(&Worker::loop, this); }

bool Worker::process(const std::string &job) {
    std::cout << "Processando job: " << job << std::endl;
    if (job.find("retry=") != std::string::npos)
        return false;

    return true;
}
void Worker::stop() {
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

void promote_delayed(Redis &redis) {
    auto now = std::chrono::system_clock::now();
    auto ts =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();

    std::vector<std::string> jobs;

    redis.zrangebyscore("jobs:delayed",
                        RightBoundedInterval<double>(static_cast<double>(ts),
                                                     BoundType::LEFT_OPEN),
                        std::back_inserter(jobs));

    for (const auto &job : jobs) {
        redis.zrem("jobs:delayed", job);
        redis.lpush("jobs", job);
    }
}

int get_retry(const std::string &job) {
    auto pos = job.find("retry=");
    return pos == std::string::npos ? 0 : std::stoi(job.substr(pos + 6));
}
int backoff_seconds(int retry) { return 1 << retry; }

void Worker::loop() {
    Redis redis(redis_url);

    while (running_) {
        try {
            promote_delayed(redis);

            auto job = redis.brpoplpush("jobs", "jobs:processing", 1);
            if (!job)
                continue;

            const std::string &payload = *job;

            if (process(payload)) {
                redis.lrem("jobs:processing", 1, payload);
            } else {
                throw std::runtime_error("job failed");
            }
        } catch (const sw::redis::Error &e) {
            std::cerr << "[Redis error] " << e.what() << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception &e) {
            std::cerr << "[Worker error] " << e.what() << '\n';
        } catch (...) {
            std::cerr << "[Unknown error]\n";
        }
    }
}
