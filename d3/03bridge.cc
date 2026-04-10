#include <iostream>
#include <string>
#include <memory>

// 实现接口--设备驱动层
class Device {
public:
  virtual ~Device() = default;
  virtual void powerOn() = 0;
  virtual void powerOff() = 0;
  virtual void setVolume(int level) = 0;
  virtual int getVolume() const = 0;
};

// 具体实现 A：电视
class TV : public Device {
private:
  int volume_ = 0;
  bool pwered_ = false;

public:
  void powerOn() override {
    pwered_ = true;
    std::cout << "TV powered on\n";
  }

  void powerOff() override {
    pwered_ = false;
    std::cout << "TV powered off\n";
  }

  void setVolume(int level) override {
    volume_ = std::max(0, std::min(100, level));
    std::cout << "TV volume set to " << volume_ << "\n";
  }

  int getVolume() const override { return volume_; }
};

// 具体实现 B：音响
class SoundSystem : public Device {
private:
  int volume_ = 0;
  bool powered_ = false;

public:
  void powerOn() override {
    powered_ = true;
    std::cout << "Sound system powered on\n";
  }


  void powerOff() override {
    powered_ = false;
    std::cout << "Sound system powered off\n";
  }

  void setVolume(int level) override {
    volume_ = std::max(0, std::min(100, level));
    std::cout << "Sound system volume set to " << volume_ << "\n";
  }

  int getVolume() const override { return volume_; }
};

// 抽象层--遥控器
class RemoteControl {
protected:
  std::unique_ptr<Device> device_; // 桥接：指向实现

public:
  explicit RemoteControl(std::unique_ptr<Device> device)
      : device_(std::move(device)) {}

  virtual ~RemoteControl() = default;

  void power() {
    if (device_->getVolume() == 0) {
      device_->powerOn();
    } else {
      device_->powerOff();
    }
  }

  void volumeUp() {
    device_->setVolume(device_->getVolume() + 10);
  }

  void volumeDown() {
    device_->setVolume(device_->getVolume() - 10);
  }
};

// 扩展抽象--高级遥控器（带显示屏）
class AdvancedRemoteControl : public RemoteControl {
private:
  int displayBrightness_ = 50;

public:
  using RemoteControl::RemoteControl;

  void setDisplayBrightness(int level) {
    displayBrightness_ = level;
    std::cout << "Display brightness: " << displayBrightness_ << "\n";
  }

  void mute() {
    device_->setVolume(0);
    std::cout << "Muted\n";
  }
};

int main() {
  // 组合1：TV + 普通遥控器
  std::unique_ptr<RemoteControl> tvRemote = std::unique_ptr<RemoteControl>(
      new RemoteControl(std::unique_ptr<Device>(new TV())));
  tvRemote->power();
  tvRemote->volumeUp();

  // 组合2：SoundSystem + 高级遥控器
  std::unique_ptr<AdvancedRemoteControl> soundRemote =
      std::unique_ptr<AdvancedRemoteControl>(new AdvancedRemoteControl(
          std::unique_ptr<Device>(new SoundSystem())));
  soundRemote->power();
  soundRemote->volumeUp();
  soundRemote->mute();

  // 组合3：TV + 高级遥控器
  std::unique_ptr<AdvancedRemoteControl> tvAdvancedRemote =
      std::unique_ptr<AdvancedRemoteControl>(new AdvancedRemoteControl(
          std::unique_ptr<Device>(new TV())));
  tvAdvancedRemote->power();
  tvAdvancedRemote->setDisplayBrightness(80);

  return 0;
}
