#include <iostream>
#include <fstream>
#include <mutex>
#include <unordered_map>

class ConfigManager {
private:
    static ConfigManager* instance;
    static std::mutex mutex_;

    std::unordered_map<std::string, std::string> config_;

    ConfigManager() {
        std::cout << "ConfigManager instance created\n";
        loadConfig("config.txt");
    }

    void loadConfig(const std::string& filename) {
        (void)filename;
        config_["host"] = "localhost";
        config_["port"] = "3306";
        config_["database"] = "mydb";
        config_["username"] = "admin";
        config_["password"] = "secret";
    }

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    static ConfigManager* getInstance() {
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance == nullptr) {
                instance = new ConfigManager();
            }
        }
        return instance;
    }

    std::string get(const std::string& key) {
        return config_[key];
    }

    void printAll() const {
        for (const auto& [key, value] : config_) {
            std::cout << key << " = " << value << "\n";
        }
    }
};

ConfigManager* ConfigManager::instance = nullptr;
std::mutex ConfigManager::mutex_;

int main() {
    ConfigManager* conf1 = ConfigManager::getInstance();
    ConfigManager* conf2 = ConfigManager::getInstance();

    std::cout << "Same instance? " << (conf1 == conf2 ? "yes" : "no") << "\n\n";

    std::cout << "Config values:\n";
    conf1->printAll();

    std::cout << "\nGet 'host': " << conf1->get("host") << "\n";
    std::cout << "Get 'port': " << conf2->get("port") << "\n";
}
