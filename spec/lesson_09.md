# 第 9 课：终章——Flyweight、Mediator 与设计模式总复习

> **前置知识**：第 1-8 课全部内容
> **本课时长**：约 120 分钟
> **学习目标**：掌握 Flyweight 和 Mediator 两种最后的模式，系统复习 SOLID 原则，完成 23 种 GoF 模式的全景回顾与实战场景分析

---

## 9.1 课程导入

### 最后两块拼图

经过 8 课的学习，我们已经掌握了 21 种设计模式。第 9 课补齐最后两块拼图：

| 类别 | 模式 | 解决的问题 |
|------|------|---------|
| **结构型** | Flyweight（享元） | 大量细粒度对象的内存优化 |
| **行为型** | Mediator（中介者） | 多对象之间的交互解耦 |

然后我们将站在更高的视角，用 SOLID 原则串联所有模式，进行 23 种模式的总复习。

---

## 9.2 Flyweight（享元模式）

### 定义

> 运用共享技术有效地支持大量细粒度的对象。

### 现实类比

就像**活字印刷**——印一本书需要几万个汉字，但不需要为每个字都刻一个新字模。常用字（"的"、"了"、"是"）共享同一个字模，只是在不同位置使用。字模是共享的**内在状态**（intrinsic state），在页面上的位置是每次不同的**外在状态**（extrinsic state）。

### 问题场景

- 文本编辑器中的字符渲染（百万个字符共享 26 种字形）
- 游戏中的粒子系统（10 万个粒子共享几种纹理）
- 地图瓦片（大地图上重复使用的几十种地形块）
- 数据库连接池（共享连接对象）

### 核心思想：内在状态 vs 外在状态

| 状态类型 | 说明 | 示例（围棋） | 是否共享 |
|---------|------|-------------|---------|
| **内在状态**（Intrinsic） | 对象不变的、可共享的部分 | 棋子颜色（黑/白）、纹理 | 共享 |
| **外在状态**（Extrinsic） | 随上下文变化的部分 | 棋盘上的位置 (x, y) | 不共享 |

Flyweight 的核心就是**把内在状态提取出来共享，外在状态由客户端在使用时传入**。

### 实现

#### 游戏粒子系统

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

// Flyweight——共享的粒子类型（内在状态）
class ParticleType {
private:
    std::string name_;
    std::string color_;
    std::string texture_;  // 假设纹理数据很大（几 MB）
    double baseSize_;

public:
    ParticleType(const std::string& name, const std::string& color,
                 const std::string& texture, double baseSize)
        : name_(name), color_(color), texture_(texture), baseSize_(baseSize) {}

    const std::string& getName() const { return name_; }
    const std::string& getColor() const { return color_; }
    double getBaseSize() const { return baseSize_; }

    void render(double x, double y, double speed, double direction) const {
        std::cout << "  [" << name_ << "] color=" << color_
                  << " size=" << baseSize_
                  << " at (" << x << ", " << y << ")"
                  << " speed=" << speed
                  << " dir=" << direction << "°\n";
    }
};
```

#### Flyweight Factory

```cpp
// Flyweight Factory——管理共享对象的创建和缓存
class ParticleTypeFactory {
private:
    std::unordered_map<std::string, std::shared_ptr<ParticleType>> types_;

public:
    std::shared_ptr<ParticleType> getType(
            const std::string& name, const std::string& color,
            const std::string& texture, double baseSize) {
        std::string key = name + "_" + color;
        auto it = types_.find(key);
        if (it != types_.end()) {
            return it->second;
        }

        auto type = std::make_shared<ParticleType>(name, color, texture, baseSize);
        types_[key] = type;
        std::cout << "  [Factory] Created new ParticleType: " << key << "\n";
        return type;
    }

    size_t typeCount() const { return types_.size(); }
};
```

#### 上下文对象（外在状态）

```cpp
// 每个粒子实例——只保存外在状态 + 指向共享 Flyweight 的指针
struct Particle {
    std::shared_ptr<ParticleType> type;  // 指向共享的内在状态
    double x, y;         // 外在状态：位置
    double speed;        // 外在状态：速度
    double direction;    // 外在状态：方向（度）

    void render() const {
        type->render(x, y, speed, direction);
    }

    void update(double dt) {
        double rad = direction * 3.14159 / 180.0;
        x += speed * std::cos(rad) * dt;
        y += speed * std::sin(rad) * dt;
    }
};
```

#### 粒子系统

```cpp
class ParticleSystem {
private:
    ParticleTypeFactory factory_;
    std::vector<Particle> particles_;

public:
    void addExplosion(double x, double y, int count) {
        auto fireType = factory_.getType("fire", "orange",
                                          "fire_texture_2MB", 3.0);
        auto smokeType = factory_.getType("smoke", "gray",
                                           "smoke_texture_4MB", 5.0);
        auto sparkType = factory_.getType("spark", "yellow",
                                           "spark_texture_1MB", 1.0);

        for (int i = 0; i < count; ++i) {
            double angle = 360.0 * i / count;
            double spd = 50.0 + (i % 3) * 20.0;

            if (i % 3 == 0)
                particles_.push_back({fireType, x, y, spd, angle});
            else if (i % 3 == 1)
                particles_.push_back({smokeType, x, y, spd * 0.5, angle});
            else
                particles_.push_back({sparkType, x, y, spd * 1.5, angle});
        }
    }

    void render() const {
        std::cout << "Rendering " << particles_.size() << " particles:\n";
        for (const auto& p : particles_) {
            p.render();
        }
    }

    void printMemoryStats() const {
        std::cout << "\n=== Memory Statistics ===\n"
                  << "Particle instances: " << particles_.size() << "\n"
                  << "Unique ParticleTypes: " << factory_.typeCount() << "\n"
                  << "Per-particle cost: ~" << sizeof(Particle) << " bytes "
                  << "(without Flyweight: ~" << sizeof(Particle) + 200 << "+ bytes)\n"
                  << "Shared texture data: only " << factory_.typeCount()
                  << " copies (not " << particles_.size() << ")\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    ParticleSystem system;

    // 两次爆炸，共 24 个粒子，但只有 3 种 ParticleType
    system.addExplosion(100.0, 200.0, 12);
    system.addExplosion(500.0, 300.0, 12);

    system.render();
    system.printMemoryStats();
    // Particle instances: 24
    // Unique ParticleTypes: 3
    // 如果没有 Flyweight，24 个粒子各自持有纹理数据 ≈ 24 × 几 MB
    // 使用 Flyweight 后，纹理数据只有 3 份
}
```

---

### Flyweight 的内存优势

```
不使用 Flyweight：
  10,000 个粒子 × (外在状态 40B + 纹理数据 2MB) ≈ 20 GB

使用 Flyweight：
  10,000 个粒子 × 外在状态 40B + 3 种纹理 × 2MB ≈ 400 KB + 6 MB ≈ 6.4 MB

节省：99.97%
```

### Flyweight vs Singleton vs 对象池

| 模式 | 目的 | 共享方式 |
|------|------|---------|
| **Flyweight** | 减少内存——共享不可变的内在状态 | 多种共享对象，按类型缓存 |
| **Singleton** | 保证全局唯一实例 | 只有一个实例 |
| **对象池** | 减少创建/销毁开销——复用可变对象 | 对象可变，用完归还 |

---

### 错误用法

```cpp
// 错误 1：把外在状态放进 Flyweight——导致无法共享
class BrokenParticleType {
    std::string color_;
    std::string texture_;
    double x_, y_;        // 错误！位置是外在状态，不应该在这里
    double speed_;        // 错误！每个粒子速度不同
};

// 错误 2：Flyweight 是可变的——共享对象被修改，影响所有使用者
class MutableFlyweight {
public:
    std::string color_;
    void setColor(const std::string& c) { color_ = c; }
    // 如果一个粒子把颜色改成红色，所有共享这个 Flyweight 的粒子都变红了！
};

// 错误 3：不需要 Flyweight 时强行使用——过度设计
class OverEngineered {
    // 只有 5 个对象，每个对象 100 字节
    // 完全不需要 Flyweight——复杂性远大于收益
};
```

---

### 面试热点

```
Q: Flyweight 模式的核心是什么？
A: 将对象状态分为内在状态（不变、可共享）和外在状态（变化、不共享）。
   内在状态通过工厂缓存并共享，外在状态由客户端在使用时传入。

Q: 什么时候应该使用 Flyweight？
A: 同时满足：1) 大量相似对象 2) 对象的大部分状态可以外部化
   3) 内存确实是瓶颈。如果对象数量少或内存充裕，不需要使用。

Q: std::string 的短字符串优化（SSO）和 Flyweight 有关系吗？
A: 理念相似但不同。SSO 是把短字符串存在对象内部避免堆分配。
   Flyweight 是多个对象共享同一个不可变数据。
   C++ 早期的 std::string COW（Copy-On-Write）更接近 Flyweight 思想。
```

---

## 9.3 Mediator（中介者模式）

### 定义

> 用一个中介对象来封装一系列的对象交互。中介者使各对象不需要显式地相互引用，从而使其耦合松散，而且可以独立地改变它们之间的交互。

### 现实类比

就像**机场塔台**——飞机之间不直接通信（否则 100 架飞机需要 4950 条通信线路），而是都和塔台通信。塔台知道所有飞机的状态，负责协调起降、避免冲突。新增一架飞机只需要让它认识塔台，不需要认识其他所有飞机。

### 问题场景

- GUI 组件交互（按钮禁用、文本框联动、复选框互斥）
- 聊天室（用户发消息通过服务器中转）
- 航空管制系统（飞机通过塔台协调）
- 微服务编排（服务间通过消息总线通信）

### 核心思想：星形拓扑 vs 网状拓扑

```
网状拓扑（无 Mediator）：          星形拓扑（有 Mediator）：
  A --- B                           A
  |\ /|                              \
  | X  |                           Mediator
  |/ \|                              / | \
  C --- D                          B   C   D

  连接数：n(n-1)/2                  连接数：n
  4 个对象 = 6 条连接               4 个对象 = 4 条连接
```

### 实现

#### 智能家居控制系统

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// 前向声明
class SmartHomeMediator;

// Colleague 基类——所有智能设备的抽象
class SmartDevice {
protected:
    std::string name_;
    SmartHomeMediator* mediator_ = nullptr;

public:
    explicit SmartDevice(const std::string& name) : name_(name) {}
    virtual ~SmartDevice() = default;

    void setMediator(SmartHomeMediator* mediator) { mediator_ = mediator; }
    const std::string& getName() const { return name_; }

    virtual void receiveEvent(const std::string& event,
                              const std::string& from) = 0;
};
```

#### Mediator 接口与具体中介者

```cpp
// Mediator 接口
class SmartHomeMediator {
public:
    virtual ~SmartHomeMediator() = default;
    virtual void notify(const std::string& event,
                        SmartDevice* sender) = 0;
    virtual void registerDevice(std::shared_ptr<SmartDevice> device) = 0;
};

// Concrete Mediator——智能家居中控
class HomeController : public SmartHomeMediator {
private:
    std::unordered_map<std::string, std::shared_ptr<SmartDevice>> devices_;

    SmartDevice* getDevice(const std::string& name) {
        auto it = devices_.find(name);
        return (it != devices_.end()) ? it->second.get() : nullptr;
    }

public:
    void registerDevice(std::shared_ptr<SmartDevice> device) override {
        device->setMediator(this);
        devices_[device->getName()] = device;
        std::cout << "[HomeController] Registered: " << device->getName() << "\n";
    }

    void notify(const std::string& event, SmartDevice* sender) override {
        std::cout << "\n[HomeController] Event: \"" << event
                  << "\" from " << sender->getName() << "\n";

        if (event == "motion_detected") {
            // 检测到运动 → 开灯 + 关闭安防警报
            if (auto* light = getDevice("LivingRoomLight"))
                light->receiveEvent("turn_on", sender->getName());
            if (auto* alarm = getDevice("SecurityAlarm"))
                alarm->receiveEvent("disarm", sender->getName());
        }
        else if (event == "door_opened") {
            // 开门 → 开空调 + 播放音乐
            if (auto* ac = getDevice("AirConditioner"))
                ac->receiveEvent("turn_on", sender->getName());
            if (auto* speaker = getDevice("Speaker"))
                speaker->receiveEvent("play_welcome", sender->getName());
        }
        else if (event == "temperature_high") {
            // 温度过高 → 开空调 + 关窗帘（隔热）
            if (auto* ac = getDevice("AirConditioner"))
                ac->receiveEvent("cool_mode", sender->getName());
            if (auto* curtain = getDevice("Curtain"))
                curtain->receiveEvent("close", sender->getName());
        }
        else if (event == "leaving_home") {
            // 离家 → 关灯 + 关空调 + 开安防 + 关窗帘
            for (auto& [name, device] : devices_) {
                if (name != sender->getName()) {
                    device->receiveEvent("owner_away", sender->getName());
                }
            }
        }
    }
};
```

#### Concrete Colleagues（具体设备）

```cpp
class MotionSensor : public SmartDevice {
public:
    MotionSensor() : SmartDevice("MotionSensor") {}

    void detectMotion() {
        std::cout << "[MotionSensor] Motion detected!\n";
        if (mediator_) mediator_->notify("motion_detected", this);
    }

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        std::cout << "  [MotionSensor] Received \"" << event
                  << "\" from " << from << "\n";
    }
};

class SmartLight : public SmartDevice {
private:
    bool on_ = false;

public:
    explicit SmartLight(const std::string& name) : SmartDevice(name) {}

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        if (event == "turn_on" && !on_) {
            on_ = true;
            std::cout << "  [" << name_ << "] Turned ON (triggered by "
                      << from << ")\n";
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
    AirConditioner() : SmartDevice("AirConditioner") {}

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        if (event == "turn_on" && !on_) {
            on_ = true;
            std::cout << "  [AirConditioner] Turned ON (triggered by "
                      << from << ")\n";
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

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        if (event == "disarm" && armed_) {
            armed_ = false;
            std::cout << "  [SecurityAlarm] Disarmed (triggered by "
                      << from << ")\n";
        } else if (event == "owner_away") {
            armed_ = true;
            std::cout << "  [SecurityAlarm] Armed (owner away)\n";
        }
    }
};

class SmartSpeaker : public SmartDevice {
public:
    SmartSpeaker() : SmartDevice("Speaker") {}

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        if (event == "play_welcome") {
            std::cout << "  [Speaker] Playing welcome music (triggered by "
                      << from << ")\n";
        } else if (event == "owner_away") {
            std::cout << "  [Speaker] Stopped playback (owner away)\n";
        }
    }
};

class SmartCurtain : public SmartDevice {
private:
    bool closed_ = false;

public:
    SmartCurtain() : SmartDevice("Curtain") {}

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        if (event == "close" && !closed_) {
            closed_ = true;
            std::cout << "  [Curtain] Closed (triggered by "
                      << from << ")\n";
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
        if (mediator_) mediator_->notify("door_opened", this);
    }

    void ownerLeaving() {
        std::cout << "[DoorLock] Owner is leaving!\n";
        if (mediator_) mediator_->notify("leaving_home", this);
    }

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
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
        if (mediator_) mediator_->notify("temperature_high", this);
    }

    void receiveEvent(const std::string& event,
                      const std::string& from) override {
        std::cout << "  [TempSensor] Received \"" << event
                  << "\" from " << from << "\n";
    }
};
```

**客户端代码**：

```cpp
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

    // 注册到中介者——设备之间无需互相认识
    controller.registerDevice(motion);
    controller.registerDevice(light);
    controller.registerDevice(ac);
    controller.registerDevice(alarm);
    controller.registerDevice(speaker);
    controller.registerDevice(curtain);
    controller.registerDevice(door);
    controller.registerDevice(temp);

    // 场景 1：传感器检测到运动
    std::cout << "\n=== Scenario 1: Motion Detected ===\n";
    motion->detectMotion();

    // 场景 2：开门回家
    std::cout << "\n=== Scenario 2: Door Opened ===\n";
    door->unlock();

    // 场景 3：温度过高
    std::cout << "\n=== Scenario 3: Temperature High ===\n";
    temp->reportHigh();

    // 场景 4：离家
    std::cout << "\n=== Scenario 4: Leaving Home ===\n";
    door->ownerLeaving();
}
```

---

### Mediator vs Observer vs Facade

| 模式 | 通信方向 | 目的 |
|------|---------|------|
| **Mediator** | 双向：组件 ↔ 中介者 | 协调多个对等对象的交互，集中控制逻辑 |
| **Observer** | 单向：Subject → Observers | 一对多通知，观察者被动接收 |
| **Facade** | 单向：客户端 → 子系统 | 简化外部调用接口，不改变子系统间交互 |

> **关键区别**：Mediator 替代了组件之间的直接通信，Observer 是在现有对象上添加通知机制，Facade 是对外部提供简化接口。

---

### 错误用法

```cpp
// 错误 1：Mediator 变成了"上帝对象"——所有逻辑都堆在中介者里
class GodMediator {
    void notify(const std::string& event, SmartDevice* sender) override {
        // 500 行 if-else 处理所有设备的所有事件组合
        // 正确做法：将规则拆分为独立的策略或规则对象
    }
};

// 错误 2：Colleague 绕过 Mediator 直接通信
class BadDevice : public SmartDevice {
    SmartLight* light_;  // 直接引用另一个设备——违背了 Mediator 的目的
public:
    void onMotion() {
        light_->receiveEvent("turn_on", name_);  // 绕过中介者
    }
};

// 错误 3：只有两个对象也用 Mediator——过度设计
class OverKillMediator {
    // 如果只有 A 和 B 两个对象交互，直接通信就好
    // Mediator 适合 3 个以上对象形成复杂交互网的场景
};
```

---

### 面试热点

```
Q: Mediator 模式解决什么问题？
A: 当多个对象之间存在复杂的网状交互时，Mediator 将交互逻辑集中到
   中介者中，把 N×N 的网状依赖变成 N×1 的星形依赖，降低耦合度。

Q: Mediator 的缺点？
A: 中介者可能变成"上帝对象"——承载了过多的协调逻辑，变得臃肿难维护。
   解决方案：将规则拆分为独立的策略对象、使用事件总线等。

Q: MVC 架构中的 Controller 是 Mediator 吗？
A: 是的。Controller 充当 Model 和 View 之间的中介者，
   协调数据更新和界面刷新，Model 和 View 不直接通信。
```

---

## 9.4 SOLID 原则

在学完 23 种模式之后，我们来学习指导这些模式背后的**设计原则**——SOLID。它们不是具体的解决方案，而是判断设计好坏的准则。

### S — 单一职责原则（Single Responsibility Principle）

> 一个类应该只有一个引起它变化的原因。

```cpp
// 违反 SRP：一个类既处理业务逻辑又负责持久化
class Employee {
    std::string name_;
    double salary_;
public:
    double calculatePay() { return salary_ * 1.1; }  // 薪资计算
    void saveToDatabase() { /* SQL INSERT ... */ }     // 持久化
    std::string generateReport() { /* HTML生成 */ }    // 报表生成
};

// 遵循 SRP：拆分为三个类
class Employee { /* 只包含员工数据 */ };
class PayCalculator { double calculate(const Employee&); };
class EmployeeRepository { void save(const Employee&); };
class ReportGenerator { std::string generate(const Employee&); };
```

**相关模式**：Strategy（算法职责分离）、Visitor（操作职责分离）、Command（请求封装为独立对象）

### O — 开闭原则（Open-Closed Principle）

> 软件实体应该对扩展开放，对修改关闭。

```cpp
// 违反 OCP：添加新形状必须修改 AreaCalculator
class AreaCalculator {
    double calculate(const Shape& s) {
        if (s.type == "circle") return 3.14 * s.r * s.r;
        if (s.type == "rect") return s.w * s.h;
        // 每新增一种形状就要加 if-else
    }
};

// 遵循 OCP：通过多态扩展
class Shape {
public:
    virtual double area() const = 0;
    virtual ~Shape() = default;
};
class Circle : public Shape {
    double r_;
public:
    double area() const override { return 3.14159 * r_ * r_; }
};
// 新增 Triangle 不需要修改任何现有代码
```

**相关模式**：Strategy、Decorator、Factory Method、Observer——都通过多态或组合实现"不修改原有代码即可扩展"。

### L — 里氏替换原则（Liskov Substitution Principle）

> 子类型必须能够替换其基类型，而不破坏程序的正确性。

```cpp
// 违反 LSP：经典的正方形-长方形问题
class Rectangle {
public:
    virtual void setWidth(int w) { width_ = w; }
    virtual void setHeight(int h) { height_ = h; }
    int area() const { return width_ * height_; }
protected:
    int width_ = 0, height_ = 0;
};

class Square : public Rectangle {
public:
    void setWidth(int w) override { width_ = height_ = w; }   // 破坏了预期
    void setHeight(int h) override { width_ = height_ = h; }  // 破坏了预期
};

void resize(Rectangle& r) {
    r.setWidth(5);
    r.setHeight(10);
    // 期望 area() == 50，但如果 r 是 Square，area() == 100！
}
```

**相关模式**：Template Method（子类覆写不应改变算法骨架的语义）、Strategy（替换策略不应改变接口契约）

### I — 接口隔离原则（Interface Segregation Principle）

> 客户端不应该被迫依赖它不使用的接口。

```cpp
// 违反 ISP：一个臃肿的接口
class IWorker {
public:
    virtual void work() = 0;
    virtual void eat() = 0;     // 机器人不吃饭
    virtual void sleep() = 0;   // 机器人不睡觉
};

// 遵循 ISP：拆分为细粒度接口
class IWorkable { public: virtual void work() = 0; };
class IFeedable { public: virtual void eat() = 0; };
class ISleepable { public: virtual void sleep() = 0; };

class Human : public IWorkable, public IFeedable, public ISleepable { /* ... */ };
class Robot : public IWorkable { /* 只实现 work */ };
```

**相关模式**：Adapter（适配不匹配的接口）、Facade（暴露精简子集）

### D — 依赖倒置原则（Dependency Inversion Principle）

> 高层模块不应依赖低层模块。两者都应依赖抽象。

```cpp
// 违反 DIP：高层直接依赖低层
class MySQLDatabase { public: void query(const std::string&); };
class UserService {
    MySQLDatabase db_;  // 直接依赖具体实现——换数据库要改 UserService
};

// 遵循 DIP：都依赖抽象
class IDatabase {
public:
    virtual void query(const std::string&) = 0;
    virtual ~IDatabase() = default;
};
class UserService {
    std::unique_ptr<IDatabase> db_;  // 依赖抽象——可注入任何实现
public:
    explicit UserService(std::unique_ptr<IDatabase> db) : db_(std::move(db)) {}
};
```

**相关模式**：Factory Method（创建时解耦）、Abstract Factory（产品族解耦）、Bridge（实现维度解耦）、Strategy（算法注入）

---

### SOLID 与设计模式的关系总结

| 原则 | 核心思想 | 代表模式 |
|------|---------|---------|
| **SRP** | 一个类一个变化原因 | Strategy, Visitor, Command |
| **OCP** | 扩展开放，修改关闭 | Strategy, Decorator, Observer, Factory Method |
| **LSP** | 子类可替换父类 | Template Method, Strategy |
| **ISP** | 接口精简不臃肿 | Adapter, Facade |
| **DIP** | 依赖抽象不依赖具体 | Factory Method, Abstract Factory, Bridge, Strategy |

---

## 9.5 23 种 GoF 模式全景图

### 创建型模式（5 种）——对象怎么创建

| 模式 | 一句话总结 | 关键词 |
|------|---------|--------|
| **Singleton** | 全局唯一实例 | "唯一" |
| **Factory Method** | 子类决定创建哪种对象 | "延迟到子类" |
| **Abstract Factory** | 创建一族相关对象 | "产品族" |
| **Builder** | 分步构建复杂对象 | "分步" |
| **Prototype** | 克隆已有对象 | "拷贝" |

### 结构型模式（7 种）——对象怎么组合

| 模式 | 一句话总结 | 关键词 |
|------|---------|--------|
| **Adapter** | 转换不兼容的接口 | "转换" |
| **Decorator** | 动态添加职责 | "包装" |
| **Facade** | 提供简化的统一接口 | "门面" |
| **Composite** | 树形结构，整体与部分一致 | "树" |
| **Proxy** | 控制对象访问 | "代理" |
| **Bridge** | 分离抽象与实现两个维度 | "桥接" |
| **Flyweight** | 共享细粒度对象 | "共享" |

### 行为型模式（11 种）——对象怎么通信

| 模式 | 一句话总结 | 关键词 |
|------|---------|--------|
| **Observer** | 一对多通知 | "订阅" |
| **Strategy** | 可替换的算法 | "策略" |
| **State** | 状态驱动行为切换 | "状态机" |
| **Template Method** | 算法骨架，子类填充步骤 | "骨架" |
| **Command** | 请求封装为对象 | "命令" |
| **Iterator** | 统一遍历接口 | "遍历" |
| **Chain of Responsibility** | 请求沿链传递 | "传递" |
| **Mediator** | 中介协调多对象交互 | "中介" |
| **Memento** | 状态快照与恢复 | "快照" |
| **Interpreter** | 语法定义与解释执行 | "语法树" |
| **Visitor** | 在不修改类的前提下添加操作 | "双分派" |

---

## 9.6 实战场景：如何选择模式

### 按问题类型选择

```
你遇到了什么问题？

├── 需要创建对象
│   ├── 只需要一个实例？                    → Singleton
│   ├── 不确定具体创建哪种对象？              → Factory Method
│   ├── 需要创建一族相关对象？                → Abstract Factory
│   ├── 构建过程复杂、参数很多？              → Builder
│   └── 需要复制已有对象？                    → Prototype
│
├── 需要组合对象
│   ├── 接口不兼容需要转换？                  → Adapter
│   ├── 需要动态添加功能？                    → Decorator
│   ├── 想简化复杂子系统的调用？              → Facade
│   ├── 树形结构，整体和部分统一处理？         → Composite
│   ├── 需要控制访问或延迟加载？              → Proxy
│   ├── 两个维度独立变化？                    → Bridge
│   └── 大量相似对象占内存太多？              → Flyweight
│
└── 需要管理行为
    ├── 算法需要灵活切换？                    → Strategy
    ├── 对象行为随状态改变？                  → State
    ├── 算法骨架固定，步骤可定制？             → Template Method
    ├── 需要封装请求、支持撤销？              → Command
    ├── 需要统一的遍历方式？                  → Iterator
    ├── 请求要经过多个处理者？                → Chain of Responsibility
    ├── 多个对象复杂交互需要协调？             → Mediator
    ├── 一个对象变化需要通知多个对象？          → Observer
    ├── 需要保存和恢复对象状态？              → Memento
    ├── 需要解释执行简单语法？                → Interpreter
    └── 稳定的数据结构需要添加新操作？         → Visitor
```

### 常见模式组合

| 场景 | 模式组合 | 说明 |
|------|---------|------|
| **文本编辑器** | Command + Memento + Observer | Command 封装操作，Memento 存快照，Observer 通知 UI 更新 |
| **游戏引擎** | State + Strategy + Flyweight + Observer | State 管理游戏状态，Strategy 切换 AI 行为，Flyweight 共享资源，Observer 触发事件 |
| **编译器** | Interpreter + Composite + Visitor | Composite 构建 AST，Interpreter 解释执行，Visitor 实现多种分析 |
| **Web 框架** | Chain of Responsibility + Template Method + Strategy | 中间件链处理请求，模板方法定义请求生命周期，Strategy 切换路由策略 |
| **GUI 框架** | Mediator + Observer + Command + Composite | Mediator 协调组件，Observer 处理事件，Command 封装操作，Composite 构建控件树 |

---

## 9.7 本课总结

### 学习路径回顾

```
第 1 课：Singleton、Factory Method、Observer          ← 入门三件套
第 2 课：Adapter、Decorator、Facade                    ← 基础结构型
第 3 课：Composite、Proxy、Bridge                      ← 进阶结构型
第 4 课：Strategy、State、Template Method              ← 行为型入门
第 5 课：Command、Iterator                             ← 行为封装
第 6 课：State、Chain of Responsibility、Memento       ← 状态与请求
第 7 课：Builder、Prototype、Abstract Factory          ← 高级创建型
第 8 课：Memento、Interpreter、Visitor                 ← 高级行为型
第 9 课：Flyweight、Mediator + SOLID + 总复习          ← 终章
```

### 最终箴言

> **模式不是目的，是手段。**

1. **先理解问题，再选择模式**——不要拿着锤子找钉子
2. **从简单开始，需要时再引入模式**——过度设计比没有设计更糟
3. **组合优于继承**——这是几乎所有模式的共同智慧
4. **针对接口编程，不针对实现编程**——SOLID 的核心精神
5. **模式会演化**——23 种 GoF 模式是 1994 年的智慧，现代语言特性（lambda、泛型、协程）可以更优雅地实现许多模式的意图

---

## 9.8 课后练习

1. **Flyweight**：实现一个"文本编辑器字符渲染"系统：
   - `CharacterStyle` 是 Flyweight，包含字体、大小、颜色（内在状态）
   - `Character` 包含字符值和在文档中的位置（外在状态）+ 指向 `CharacterStyle` 的指针
   - `StyleFactory` 管理样式缓存
   - 创建一段 1000 个字符的文本，统计实际创建了多少个 `CharacterStyle` 对象

2. **Mediator**：实现一个"聊天室"系统：
   - `ChatRoom` 作为 Mediator
   - `User` 作为 Colleague，支持发送消息和接收消息
   - 支持公共消息（广播给所有人）和私聊消息（发给指定用户）
   - 支持创建多个聊天室，用户可以加入/离开

3. **综合设计**：选择一个你熟悉的系统（如外卖平台、在线考试系统、图书管理系统），分析其中可以用到哪些设计模式，画出类图并说明选择理由。要求：
   - 至少使用 5 种不同的设计模式
   - 说明每种模式解决了什么具体问题
   - 分析是否有"过度设计"的风险
