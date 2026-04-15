#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// 前向声明
class SmartHomeMediator;

// Colleague 基类--所有智能设备的抽象
class SmartDevice {
protected:
  std::string name_;
  SmartHomeMediator *mediator_ = nullptr;

public:
  explicit SmartDevice(const std::string &name) : name_(name) {}
  virtual ~SmartDevice() = default;

  void setMediator(SmartHomeMediator* mediator) { mediator_ = mediator; }
  const std::string& getName() const { return name_; }

  virtual void receiveEvent(const std::string &event,
                            const std::string &from) = 0;
};

// Mediator 接口
class SmartHomeMediator {
public:
  virtual ~SmartHomeMediator() = default;
  virtual void notify(const std::string &event, SmartDevice *sender) = 0;
  virtual void registerDevice(std::shared_ptr<SmartDevice> device) = 0;
};

// Concrete Mediator--智能家居中控
class HomeController : public SmartHomeMediator {
private:
  std::unordered_map<std::string, std::shared_ptr<SmartDevice>> devices_;

  SmartDevice *getDevice(const std::string &name) {
    auto it = devices_.find(name);
    return (it != devices_.end()) ? it->second.get() : nullptr;
  }

public:
  void registerDevice(std::shared_ptr<SmartDevice> device) override {
    device->setMediator(this);
    devices_[device->getName()] = device;
    std::cout << "[HomeController] Registered: " << device->getName() << "\n";
  }

  void notify(const std::string &event, SmartDevice *sender) override {
    std::cout << "\n[HomeController] Event: \"" << event << "\" from "
              << sender->getName() << "\n";

    if (event == "motion_detected") {
      // 检测到运动 -> 开灯 + 关闭安防警报
      if (auto *light = getDevice("LivingRoomLight"))
        light->receiveEvent("turn_on", sender->getName());
      if (auto *alarm = getDevice("SecurityAlarm"))
        alarm->receiveEvent("disarm", sender->getName());
    } else if (event == "door_opened") {
      // 开门 -> 开空调 + 播放音乐
      if (auto *ac = getDevice("AirConditioner"))
        ac->receiveEvent("turn_on", sender->getName());
      if (auto *speaker = getDevice("Speaker"))
        speaker->receiveEvent("play_welcome", sender->getName());
    } else if (event == "temperature_high") {
      // 温度过高 -> 开空调 + 关窗帘（隔热）
      if (auto *ac = getDevice("AirConditioner"))
        ac->receiveEvent("cool_mode", sender->getName());
      if (auto *curtain = getDevice("Curtain"))
        curtain->receiveEvent("close", sender->getName());
    } else if (event == "leaving_home") {
      // 离家 -> 关灯 + 关空调 + 开安防 + 关窗帘
      for (auto &kv : devices_) {
        const std::string &name = kv.first;
        SmartDevice *device = kv.second.get();
        if (name != sender->getName()) {
          device->receiveEvent("owner_away", sender->getName());
        }
      }
    }
  }
};

// 具体设备
class MotionSensor : public SmartDevice {
public:
  MotionSensor() : SmartDevice("MotionSensor") {}

  void detectMotion() {
    std::cout << "[MotionSensor] Motion detected!\n";
    if(mediator_) mediator_->notify("motion_detected", this);
  }

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    std::cout << "  [MotionSensor] Received \"" << event << "\" from " << from
              << "\n";
  }
};

class SmartLight : public SmartDevice {
private:
  bool on_ = false;

public:
  explicit SmartLight(const std::string &name) : SmartDevice(name) {}

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "turn_on" && !on_) {
      on_ = true;
      std::cout << "  [" << name_ << "] Turned ON (triggered by " << from
                << ")\n";
    } else if (event == "owner_away" && on_) {
      on_ = false;
      std::cout << "  [" << name_ << "] Turned OFF (owner away)\n";
    }
  }
};

class AirConditioner : public SmartDevice {
private:
  bool on_ = false;
  std::string mode_ = "auto";

public:
  AirConditioner()
    : SmartDevice("AirCondtioner") {}

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "turn_on" && !on_) {
      on_ = true;
      std::cout << "  [AirConditioner] Turned ON (triggered by " << from
                << ")\n";
    } else if (event == "cool_mode") {
      on_ = true;
      mode_ = "cool";
      std::cout << "  [AirConditioner] Switched to COOL mode (triggered by "
                << from << ")\n";
    } else if (event == "owner_away" && on_) {
      on_ = false;
      std::cout << "  [AirConditioner] Turned OFF (owner away)\n";
    }
  }
};

class SecurityAlarm : public SmartDevice {
private:
  bool armed_ = true;

public:
  SecurityAlarm() : SmartDevice("SecurityAlarm") {}

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "disarm" && armed_) {
      armed_ = false;
      std::cout << "  [SecurityAlarm] Disarmed (triggered by " << from << ")\n";
    } else if (event == "owner_away") {
      armed_ = true;
      std::cout << "  [SecurityAlarm] Armed (owner away)\n";
    }
  }
};

class SmartSpeaker : public SmartDevice {
public:
  SmartSpeaker() : SmartDevice("Speaker") {}

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "play_welcome") {
      std::cout << "  [Speaker] Playing welcome music (triggered by " << from
                << ")\n";
    } else if (event == "owner_away") {
      std::cout << "  [Speaker] Stopped palyback (owner away)\n";
    }
  }
};

class SmartCurtain : public SmartDevice {
private:
  bool closed_ = false;

public:
  SmartCurtain() : SmartDevice("Curtain") {}

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "close" && !closed_) {
      closed_ = true;
      std::cout << "  [Curtain] Closed (triggered by " << from << ")\n";
    } else if (event == "owner_away" && !closed_) {
      closed_ = true;
      std::cout << "  [Curtain] Closed (owner away)\n";
    }
  }
};



class SmartDoorLock : public SmartDevice {
public:
  SmartDoorLock() : SmartDevice("DoorLock") {}

  void unlock() {
    std::cout << "[DoorLock] Door unlocked and opened!\n";
    if(mediator_) mediator_->notify("door_opened", this);
  }

  void ownerLeaving() {
    std::cout << "[DoorLock] Owner is leaving!\n";
    if(mediator_) mediator_->notify("leaving_home", this);
  }

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    if (event == "owner_away") {
      std::cout << "  [DoorLock] Locked (owner away)\n";
    }
  }
};

class TemperatureSensor : public SmartDevice {
public:
  TemperatureSensor() : SmartDevice("TempSensor") {}

  void reportHigh() {
    std::cout << "[TempSensor] Temperature is too high!\n";
    if (mediator_)
      mediator_->notify("temperature_high", this);
  }

  void receiveEvent(const std::string &event,
                    const std::string &from) override {
    std::cout << "  [TempSensor] Received \"" << event << "\" from " << from
              << "\n";
  }
};

int main() {
  // 创建中介者
  HomeController controller;

  // 创建设备
  auto motion = std::make_shared<MotionSensor>();
  auto light = std::make_shared<SmartLight>("LivingRoomLight");
  auto ac = std::make_shared<AirConditioner>();
  auto alarm = std::make_shared<SecurityAlarm>();
  auto speaker = std::make_shared<SmartSpeaker>();
  auto curtain = std::make_shared<SmartCurtain>();
  auto door = std::make_shared<SmartDoorLock>();
  auto temp = std::make_shared<TemperatureSensor>();

  // 注册到中介者--设备之间无需互相认识
  controller.registerDevice(motion);
  controller.registerDevice(light);
  controller.registerDevice(ac);
  controller.registerDevice(alarm);
  controller.registerDevice(speaker);
  controller.registerDevice(curtain);
  controller.registerDevice(door);
  controller.registerDevice(temp);

  // 场景1：传感器检测到运动
  std::cout << "\n=== Scenario 1: Notion Detected ===\n";
  motion->detectMotion();

  // 场景2：开门回家
  std::cout << "\n=== Scenario 2: Door Opened ===\n";
  door->unlock();

  // 场景3：温度过高
  std::cout << "\n=== Scenario 3: Temperature High ===\n";
  temp->reportHigh();

  // 场景4：离家
  std::cout << "\n=== Scenario 4: Leaving Home ===\n";
  door->ownerLeaving();

  return 0;
}
