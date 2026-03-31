#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#include <string>

class Observer {

public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class ConcreteObserverA : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "ConcreteObserverA received: " << message << std::endl;
    }
};

class ConcreteObserverB : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "ConcreteObserverB received: " << message << std::endl;
    }
};

class Subject {

private:
    std::vector<std::weak_ptr<Observer>> observers_; // weak_ptr 防止循环引用
    std::string state_;

public:
    void subscribe(const std::shared_ptr<Observer>& observer) {
        observers_.push_back(observer);
    }

    void unsubscribe(const std::shared_ptr<Observer>& observer) {
        observers_.erase(
                         std::remove_if(observers_.begin(), observers_.end(), [&observer](const std::weak_ptr<Observer>& wptr) {
                                        return wptr.lock() == observer;
                                        }),
                         observers_.end()
                        );
    }

    void setState(const std::string& state) {
        state_ = state;
        notify();
    }

private:
    void notify() {
        for(auto it = observers_.begin(); it != observers_.end();) {
            if(auto obs = it->lock()) {
                obs->update(state_);
                ++it;
            } else {
                // 观察者已销毁，清理weak_ptr
                it = observers_.erase(it);
            }
        }
    }
};

int main()
{
    auto sub = std::make_shared<Subject>();

    auto phoneDisplay = std::make_shared<ConcreteObserverA>();
    auto webDisplay = std::make_shared<ConcreteObserverA>();

    sub->subscribe(phoneDisplay);
    sub->subscribe(webDisplay);

    sub->setState("Sunny, tem 25");

    sub->unsubscribe(phoneDisplay);

    sub->setState("Rainy, tem 18");
    return 0;
}

