// 课后练习 1：线程安全的 ConfigManager 单例
// g++ -std=c++11 -Wall -Wextra -o 01_config_manager 01_config_manager.cc

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>

class ConfigManager {
private:
    std::unordered_map<std::string, std::string> config_;
    mutable std::mutex mutex_;  // mutable: const 成员函数中也能加锁

    ConfigManager() {
        std::cout << "ConfigManager initialized\n";
    }

public:
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    static ConfigManager& getInstance() {
        static ConfigManager instance;  // Meyers' Singleton
        return instance;
    }

    void set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_[key] = value;
    }

    std::string get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(key);
        return it != config_.end() ? it->second : "";
    }
};

int main() {
    auto& config = ConfigManager::getInstance();

    config.set("db_host", "localhost");
    config.set("db_port", "5432");
    config.set("app_name", "MyApp");

    std::cout << "db_host: " << config.get("db_host") << "\n";
    std::cout << "db_port: " << config.get("db_port") << "\n";
    std::cout << "app_name: " << config.get("app_name") << "\n";
    std::cout << "missing: '" << config.get("missing") << "'\n";

    // 验证单例
    auto& config2 = ConfigManager::getInstance();
    std::cout << "Same instance: " << (&config == &config2) << "\n";
    // Output: Same instance: 1
}
