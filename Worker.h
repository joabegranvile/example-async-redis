#pragma once

#include <atomic>
#include <string>
#include <thread>

class Worker {
  public:
    explicit Worker(const std::string &redis_url);
    void start();
    void stop();

  private:
    void loop();

    bool process(const std::string &job);
    std::atomic<bool> running_{true};
    std::thread thread_;
    std::string redis_url;
};
