#include <cstdio>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>
#include <string>
#include <algorithm>

// 观察者接口
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(float temperature, float humidity) = 0;
    virtual std::string getName() const = 0;
};

// 被观察者（Subject）
class WeatherStation {
private:
    std::vector<Observer*> observers_; // 不拥有观察者的生命周期
    float temperature_ = 0.0f;
    float humidity_ = 0.0f;

public:
    // 注册观察者
    void subscribe(Observer* observer) {
        observers_.push_back(observer);
        std::cout << observer->getName() << " subscribed\n";
    }

    // 取消注册
    void unsubscribe(Observer* observer) {
        observers_.erase(
                         std::remove(observers_.begin(), observers_.end(), observer),
                         observers_.end()
                        );
        std::cout << observer->getName() << " unscribed\n";
    }

    // 通知所有观察者
    void notifyAll() {
        for(auto* obs : observers_) {
            obs->update(temperature_, humidity_);
        }
    }

    // 状态变化时自动通知
    void setMeasurements(float temp, float humidity) {
        std::cout << "\n--- Weather update ---\n";
        temperature_ = temp;
        humidity_ = humidity;
        notifyAll(); // 状态变化->自动通知
    }
};

// 具体观察者
// 观察者 A：手机App显示
class PhoneDisplay : public Observer {
private:
    std::string userName_;

public:
    explicit PhoneDisplay(const std::string& user) : userName_(user) {}

    void update(float temperature, float humidity) override {
        std::cout << "[Phone:" << userName_ << "] "
            << temperature << "°C, " << humidity << "% humidity\n";
    }

    std::string getName() const override { return "Phone:" + userName_; }
};

// 观察者 B：窗口显示屏
class WindowDisplay : public Observer {
private:
    std::string location_;

public:
    explicit WindowDisplay(const std::string& loc) : location_(loc) {}

    void update(float temperature, float humidity) override {
        std::cout << "[Window:" << location_ << "] "
            << "Temp=" << temperature << "°C\n";
    }

    std::string getName() const override { return "Window:" + location_; }
};

// 观察者C：温度预警系统
class AlertSystem : public Observer {
private:
    float threshold_;

public:
    explicit AlertSystem(float threshold) : threshold_(threshold) {}

    void update(float temperature, float humidity) override {
        if(temperature > threshold_) {
            std::cout << "[ALERT] Temperature " << temperature
                << "°C exceeds threshold " << threshold_ << "°C\n";
        }
    }

    std::string getName() const override {return "AlertSystem";}
};

int main() {
    WeatherStation station;

    // 创建观察者
    PhoneDisplay alice("Alice");
    PhoneDisplay bob("Bob");
    WindowDisplay lobby("Lobby");
    AlertSystem alert(35.0f);

    // 订阅
    station.subscribe(&alice);
    station.subscribe(&bob);
    station.subscribe(&lobby);
    station.subscribe(&alert);

    // 气象站更新数据 -> 所有观察者自动收到通知
    station.setMeasurements(25.0f, 65.0f);

    // bob取消订阅
    station.unsubscribe(&bob);

    // 再次更新--Bob 不再收到通知
    station.setMeasurements(38.0f, 40.0f);

    return 0;
}
