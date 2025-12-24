#pragma once
#include <string>
#include <sw/redis++/redis++.h>

class RedisClient {
  public:
    explicit RedisClient(const std::string &url);

    void ping();
    void set(const std::string &k, const std::string &v);
    std::string get(const std::string &key);

  private:
    sw::redis::Redis redis_;
};
