#include <iostream>
#include <string>

class Logger {
private:
    std::string logFile_;

    // 私有构造函数--外部无法创建实例
    Logger() : logFile_("app.log") {
        std::cout << "Logger initialized with file: " << logFile_ << "\n";
    }

public:
    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 全局访问点--C++11保证线程安全
    static Logger& getInstance() {
        static Logger instance; // 局部静态变量，保证首次调用时初始化
        return instance;
    }

    void log(const std::string& message) {
        std::cout << "[LOG] " << message << "\n";
    }
};

int main() {
    // 通过全局访问点获取实例
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();

    // 两个引用指向同一个对象
    std::cout << "Same instance: " << (&logger1 == &logger2) << "\n";

    logger1.log("Application started");
    logger2.log("Processing request");

    return 0;
}
