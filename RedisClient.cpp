
#include "RedisClient.h"
#include <sw/redis++/redis++.h>

using namespace sw::redis;

RedisClient::RedisClient(const std::string &url) : redis_(url) {}

void RedisClient::ping() { redis_.ping(); }

void RedisClient::set(const std::string &k, const std::string &v) {
    redis_.set(k, v);
}

std::string RedisClient::get(const std::string &k) {
    auto v = redis_.get(k);
    return v ? *v : "";
}
