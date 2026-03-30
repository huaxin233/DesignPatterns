#include <iostream>

class Singleton {

private:
    Singleton() {
        std::cout << "Singleton instance created\n";
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton& getInstance() {
        // C++11 保证线程安全， Magic static
        static Singleton instance;
        return instance;
    }

    void doSomething() {
        std::cout << "Singleton doing something\n";
    }
};

int main() {
    Singleton& s1 = Singleton::getInstance();
    Singleton& s2 = Singleton::getInstance();

    s1.doSomething();
    
    // s1和s2是同一个对象
    std::cout << "Same instance? " << (&s1 == &s2 ? "Yes" : "No") << "\n";
}
