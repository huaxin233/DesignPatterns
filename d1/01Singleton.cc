#include <iostream>
#include <mutex>

class Singleton {

private:
    static Singleton* instance; // 单例指针
    static std::mutex mutex_;   // 互斥锁
    
    // 私有构造函数，防止外部new
    Singleton() {
        std::cout << "Singleton instance created\n";
    }

    // 删除拷贝构造函数和赋值运算符（防止副本）
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if(instance == nullptr) {
            instance = new Singleton();
        }
        return instance;
    }

    void doSometing() {
        std::cout << "Singleton doing something\n";
    }
};

// 静态成员初始化
Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mutex_;
