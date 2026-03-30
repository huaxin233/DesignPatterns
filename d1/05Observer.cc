#include <iostream>
#include <type_traits>
#include <vector>
#include <memory>
#include <string>

// 前向声明
class Subject;

// 观察者抽象基类
class Observer {

public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

// 具体观察者A
class ConcreteObserverA : public Observer {
public:
    void update(const std::string& message) override {
        std::cout <<"ConcreteObserverA received: " << message << "\n";
    }
};

// 具体观察者B
class ConcreteObserverB : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "ConcreteObserverB received: " << message << "\n";
    }
};

// 主题 （被观察者）
class Subject {

private:
    std::vector<Observer*> observers_; // 观察者列表
    std::string state_;                // 内部状态

public:
    // 订阅
    void attach(Observer* observer) {
        observers_.push_back(observer);
    }

    // 取消订阅
    void detach(Observer* observer) {
        // 简化：移除第一个匹配的观察者
        for(auto it = observers_.begin(); it != observers_.end(); ++it) {
            if(*it == observer) {
                observers_.erase(it);
                break;
            }
        }
    }

    // 状态变更时，通知所有观察者
    void setState(const std::string& state) {
        state_ = state;
        notify();
    }

    const std::string& getState() const {
        return state_;
    }

private:
    void notify() {
        for(Observer* observer : observers_) {
            observer->update(state_);
        }
    }
};


int main()
{
    Subject subject;

    ConcreteObserverA observerA;
    ConcreteObserverB observerB;

    subject.attach(&observerA);
    subject.attach(&observerB);

    subject.setState("Hello World");

    subject.detach(&observerA);

    subject.setState("GoodBye");

    return 0;
}

