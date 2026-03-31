#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

class Observer {
public:
  virtual ~Observer() = default;
  virtual void update(float temp, float humidity, float pressure) = 0;
};

class Subject {
private:
  std::vector<std::weak_ptr<Observer>> observers_;
  float temprature_ = 0;
  float humidity_ = 0;
  float pressure_ = 0;

public:
  void subscribe(const std::shared_ptr<Observer> &observer) {
    observers_.push_back(observer);
  }

  void unsubscribe(const std::shared_ptr<Observer> &observer) {
    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
                       [&observer](const std::weak_ptr<Observer> &wptr) {
                         return wptr.lock() == observer;
                       }),
        observers_.end());
  }

  void setMeasurements(float temp, float humidity, float pressure) {
    temprature_ = temp;
    humidity_ = humidity;
    pressure_ = pressure;
    notify();
  }

  float getTemperature() const { return temprature_; }
  float getHumidity() const { return humidity_; }
  float getPressure() const { return pressure_; }

private:
  void notify() {
    for (auto it = observers_.begin(); it != observers_.end();) {
      if (auto obs = it->lock()) {
        obs->update(temprature_, humidity_, pressure_);
        ++it;
      } else {
        it = observers_.erase(it);
      }
    }
  }
};

class PhoneDisplay : public Observer {
public:
  void update(float temp, float humidity, float pressure) override {
    std::cout << "[Phone] Temp: " << temp << ", Humidity: " << humidity
              << ", Pressure: " << pressure << std::endl;
  }
};

class WebDisplay : public Observer {
public:
  void update(float temp, float humidity, float pressure) override {
    std::cout << "[Web] Now: " << temp << "C, " << humidity << "% humidity, "
              << pressure << "hPa\n";
  }
};

int main() {
  auto weatherStation = std::make_shared<Subject>();

  auto phone = std::make_shared<PhoneDisplay>();
  auto web = std::make_shared<WebDisplay>();

  weatherStation->subscribe(phone);
  weatherStation->subscribe(web);

  std::cout << "=== weather Update 1 ===\n";
  weatherStation->setMeasurements(25.5f, 60.0f, 1013.25f);
  weatherStation->unsubscribe(phone);

  std::cout << "=== weather Update 2 (phone unsubscribe) ===\n";
  weatherStation->setMeasurements(18.0f, 85.0f, 1008.50f);

  web.reset();
  weatherStation->setMeasurements(10.0f, 50.0f, 1020.00f);

  return 0;
}
