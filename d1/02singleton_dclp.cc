#include <algorithm>
#include <iostream>
#include <mutex>
#include <memory>

class DatabaseConnection {
private:
    static std::unique_ptr<DatabaseConnection> instance_;
    static std::mutex mutex_;

    DatabaseConnection() {
        std::cout << "Database connection established\n";
    }

public:
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    static DatabaseConnection& getInstance() {
        // 第一次检查：避免每次加锁（性能优化）
        if(!instance_) {
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：防止多线程同时通过第一次检查
            if(!instance_) {
                instance_.reset(new DatabaseConnection());
            }
        }
        return *instance_;
    }

    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << "\n";
    }
};

// 静态成员定义
std::unique_ptr<DatabaseConnection> DatabaseConnection::instance_ = nullptr;
std::mutex DatabaseConnection::mutex_;

int main() {
    auto &db = DatabaseConnection::getInstance();
    db.query("SELECT * FROM users");

    auto &db2 = DatabaseConnection::getInstance();
    db2.query("INSERT INTO logs VALUES(...)");

    std::cout << "Same instance: " << (&db == &db2) << "\n";

    return 0;
}
