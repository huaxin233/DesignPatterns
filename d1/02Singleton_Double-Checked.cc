#include <iostream>
#include <mutex>

class Singleton {

private:
    static Singleton* instance;
    static std::mutex mutex_;

    Singleton() {
        std::cout << "Singleton instance created\n";
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton* getInstance() {
        // 第一次检查：不加锁快速判断
        if(instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：加锁后再确认（防止多线程重复创建）
            if(instance == nullptr) {
                instance = new Singleton();
            }
        }
        return instance;
    }

    void doSometing() {
        std::cout << "Singleton doing something\n";
    }
};

Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mutex_;
