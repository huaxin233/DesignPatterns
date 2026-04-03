# 第 1 课：开篇与三大基础模式

> **前置知识**：C++ 基础语法、类与继承、多态（虚函数）、智能指针基本概念
> **本课时长**：约 90 分钟
> **学习目标**：理解设计模式的本质，掌握 Singleton、Factory Method、Observer 三种基础模式，建立"对象创建"与"对象通信"的基本范式

---

## 1.1 什么是设计模式？

### 一句话定义

> 设计模式是**反复出现的设计问题的经典解法**——不是发明，而是发现。

### GoF 与 23 种模式

1994 年，四位作者（Erich Gamma、Richard Helm、Ralph Johnson、John Vlissides，合称 **Gang of Four**）出版了《Design Patterns: Elements of Reusable Object-Oriented Software》，系统整理了 23 种面向对象的设计模式，分为三大类：

| 类别 | 关注点 | 数量 |
|------|--------|------|
| **创建型** (Creational) | 如何创建对象 | 5 |
| **结构型** (Structural) | 如何组合对象 | 7 |
| **行为型** (Behavioral) | 对象之间如何通信 | 11 |

### 为什么要学设计模式？

- **共同语言**：团队成员说"这里用工厂模式"，所有人立即理解意图
- **避免重复踩坑**：前人总结的经验，避免从零开始设计
- **提升可扩展性**：模式鼓励"面向接口编程"和"组合优于继承"

### 两条核心原则

在学习具体模式之前，先记住两条贯穿所有模式的核心原则：

1. **面向接口编程，而非面向实现编程** —— 依赖抽象，不依赖具体类
2. **组合优于继承** —— 优先使用对象组合来复用行为，而不是类继承

本课的三个模式分别展示了这两条原则在不同场景下的应用。

---

## 1.2 Singleton（单例模式）

### 定义

> 确保一个类只有一个实例，并提供一个全局访问点。

### 现实类比

就像一个国家的**总统**——同一时间只能有一位总统，所有人通过"总统办公室"这个统一入口联系到他。你不能自己"new 一个总统"。

### 问题场景

- 数据库连接池（全局唯一的连接管理器）
- 日志系统（所有模块写入同一个日志实例）
- 配置管理器（全局读取同一份配置）
- 线程池（共享同一组工作线程）

### 实现

#### 版本 1：Meyers' Singleton（推荐）

C++11 保证局部静态变量的初始化是线程安全的，这是最简洁的实现：

**`singleton_meyers.cpp`**

```cpp
#include <iostream>
#include <string>

class Logger {
private:
    std::string logFile_;

    // 私有构造函数——外部无法创建实例
    Logger() : logFile_("app.log") {
        std::cout << "Logger initialized with file: " << logFile_ << "\n";
    }

public:
    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 全局访问点——C++11 保证线程安全
    static Logger& getInstance() {
        static Logger instance;  // 局部静态变量，首次调用时初始化
        return instance;
    }

    void log(const std::string& message) {
        std::cout << "[LOG] " << message << "\n";
    }
};
```

**`singleton_meyers.cpp`** — 客户端代码：

```cpp
int main() {
    // 通过全局访问点获取实例
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();

    // 两个引用指向同一个对象
    std::cout << "Same instance: " << (&logger1 == &logger2) << "\n";
    // Output: Same instance: 1

    logger1.log("Application started");
    logger2.log("Processing request");

    // 以下代码编译失败：
    // Logger logger3;                     // 错误：构造函数是 private
    // Logger logger4 = logger1;           // 错误：拷贝构造函数被 delete
    // Logger logger5 = std::move(logger1); // 错误：移动构造函数未定义
}
```

#### 版本 2：双重检查锁定（经典面试考点）

**`singleton_dclp.cpp`**

```cpp
#include <iostream>
#include <mutex>
#include <memory>

class DatabaseConnection {
private:
    static std::unique_ptr<DatabaseConnection> instance_;
    static std::mutex mutex_;

    DatabaseConnection() {
        std::cout << "Database connection established\n";
    }

public:
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    static DatabaseConnection& getInstance() {
        // 第一次检查：避免每次都加锁（性能优化）
        if (!instance_) {
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：防止多线程同时通过第一次检查
            if (!instance_) {
                instance_.reset(new DatabaseConnection());
            }
        }
        return *instance_;
    }

    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << "\n";
    }
};

// 静态成员定义
std::unique_ptr<DatabaseConnection> DatabaseConnection::instance_ = nullptr;
std::mutex DatabaseConnection::mutex_;
```

**`singleton_dclp.cpp`** — 客户端代码：

```cpp
int main() {
    auto& db = DatabaseConnection::getInstance();
    db.query("SELECT * FROM users");

    auto& db2 = DatabaseConnection::getInstance();
    db2.query("INSERT INTO logs VALUES(...)");

    std::cout << "Same instance: " << (&db == &db2) << "\n";
    // Output: Same instance: 1
}
```

> **选择建议**：优先使用 Meyers' Singleton。双重检查锁定主要出现在面试和 C++11 之前的代码中。

---

### 错误用法

**`singleton_bad.cpp`**

```cpp
// 错误 1：非线程安全的懒汉式——多线程下会创建多个实例
class UnsafeSingleton {
    static UnsafeSingleton* instance_;
    UnsafeSingleton() {}
public:
    static UnsafeSingleton* getInstance() {
        if (instance_ == nullptr) {   // 竞态条件！线程 A 和 B 同时到达这里
            instance_ = new UnsafeSingleton();  // 可能创建两个实例
        }
        return instance_;
    }
};

// 错误 2：返回指针而不是引用——调用者可能 delete 它
class DeletableSingleton {
    static DeletableSingleton* instance_;
    DeletableSingleton() {}
public:
    static DeletableSingleton* getInstance() {
        static DeletableSingleton instance;
        return &instance;  // 返回指针！
    }
};
// 调用者可能写出：delete DeletableSingleton::getInstance(); // 灾难！

// 错误 3：忘记禁止拷贝——绕过单例约束
class CopyableSingleton {
    CopyableSingleton() {}
public:
    static CopyableSingleton& getInstance() {
        static CopyableSingleton instance;
        return instance;
    }
    // 没有 delete 拷贝构造函数！
};
// 调用者可以：CopyableSingleton copy = CopyableSingleton::getInstance();
// 现在有两个对象了！违反了"唯一实例"的约束
```

---

### 面试热点

```
Q: Meyers' Singleton 为什么是线程安全的？
A: C++11 标准（§6.7）规定：局部静态变量的初始化是线程安全的。
   如果多线程同时首次进入 getInstance()，编译器保证只有一个线程执行初始化，
   其余线程等待。

Q: 单例模式的主要缺点？
A: 1) 全局状态——难以测试（无法 mock）
   2) 隐藏依赖——调用者看不出函数内部依赖了全局单例
   3) 违反单一职责——既管理自身生命周期，又承载业务逻辑
   现代实践中常用依赖注入替代。

Q: 饿汉式 vs 懒汉式？
A: 饿汉式：程序启动时就创建，简单但可能浪费资源
   懒汉式：首次使用时创建（Meyers' Singleton 就是懒汉式）
   一般推荐懒汉式，除非初始化顺序有严格要求。
```

---

## 1.3 Factory Method（工厂方法模式）

### 定义

> 定义一个创建对象的接口，但由子类决定实例化哪一个类。工厂方法使一个类的实例化延迟到其子类。

### 现实类比

就像**物流公司**——客户只说"帮我发货"，不关心具体用卡车还是轮船还是飞机。物流公司（工厂）根据目的地和货物类型，决定用哪种运输工具（产品）。

### 问题场景

- 跨平台 UI 组件（同一个 `createButton()` 在 Windows 创建 WinButton，在 Mac 创建 MacButton）
- 文档解析器（根据文件扩展名创建 PDF/Word/Excel 解析器）
- 游戏角色创建（根据选择创建战士/法师/盗贼）
- 数据库驱动（根据配置创建 MySQL/PostgreSQL/SQLite 连接）

### 实现

#### 经典工厂方法（子类决定创建类型）

**`factory_method.cpp`**

```cpp
#include <iostream>
#include <memory>
#include <string>

// ========== 产品层次结构 ==========

// 抽象产品——所有运输工具的统一接口
class Transport {
public:
    virtual ~Transport() = default;
    virtual void deliver(const std::string& cargo) const = 0;
    virtual double getCostPerKg() const = 0;
};

// 具体产品 A：卡车
class Truck : public Transport {
public:
    void deliver(const std::string& cargo) const override {
        std::cout << "Truck delivering [" << cargo << "] by road\n";
    }

    double getCostPerKg() const override { return 1.5; }
};

// 具体产品 B：轮船
class Ship : public Transport {
public:
    void deliver(const std::string& cargo) const override {
        std::cout << "Ship delivering [" << cargo << "] by sea\n";
    }

    double getCostPerKg() const override { return 0.8; }
};

// 具体产品 C：飞机
class Airplane : public Transport {
public:
    void deliver(const std::string& cargo) const override {
        std::cout << "Airplane delivering [" << cargo << "] by air\n";
    }

    double getCostPerKg() const override { return 5.0; }
};
```

**`factory_method.cpp`（续）**

```cpp
// ========== 工厂层次结构 ==========

// 抽象工厂（Creator）——定义工厂方法
class LogisticsCompany {
public:
    virtual ~LogisticsCompany() = default;

    // 工厂方法——由子类实现，决定创建哪种运输工具
    virtual std::unique_ptr<Transport> createTransport() const = 0;

    // 业务逻辑——使用工厂方法创建的产品，不依赖具体产品类型
    void planDelivery(const std::string& cargo, double weight) const {
        auto transport = createTransport();  // 调用工厂方法
        double cost = transport->getCostPerKg() * weight;
        std::cout << "Cost estimate: $" << cost << "\n";
        transport->deliver(cargo);
    }
};

// 具体工厂 A：陆路物流
class RoadLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Truck());
    }
};

// 具体工厂 B：海运物流
class SeaLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Ship());
    }
};

// 具体工厂 C：空运物流
class AirLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Airplane());
    }
};
```

**`factory_method.cpp`** — 客户端代码：

```cpp
// 客户端代码——只依赖抽象接口，不知道具体产品类型
void processOrder(const LogisticsCompany& company,
                  const std::string& cargo, double weight) {
    company.planDelivery(cargo, weight);
}

int main() {
    RoadLogistics road;
    SeaLogistics sea;
    AirLogistics air;

    std::cout << "=== Road delivery ===\n";
    processOrder(road, "Electronics", 100);
    // Output:
    // Cost estimate: $150
    // Truck delivering [Electronics] by road

    std::cout << "\n=== Sea delivery ===\n";
    processOrder(sea, "Furniture", 500);
    // Output:
    // Cost estimate: $400
    // Ship delivering [Furniture] by sea

    std::cout << "\n=== Air delivery ===\n";
    processOrder(air, "Medicine", 10);
    // Output:
    // Cost estimate: $50
    // Airplane delivering [Medicine] by air
}
```

---

### 简单工厂 vs 工厂方法

| 维度 | 简单工厂 | 工厂方法 |
|------|---------|---------|
| **结构** | 一个工厂类 + switch/if | 抽象工厂 + 多个具体工厂子类 |
| **扩展方式** | 修改工厂代码（违反开闭原则） | 新增工厂子类（符合开闭原则） |
| **适用场景** | 产品种类少且稳定 | 产品种类多且可能扩展 |

简单工厂（也叫静态工厂）不是 GoF 模式，但实际开发中使用非常广泛：

**`simple_factory.cpp`**

```cpp
// 简单工厂——通过参数决定创建哪种产品
class SimpleTransportFactory {
public:
    enum Type { ROAD, SEA, AIR };

    static std::unique_ptr<Transport> create(Type type) {
        switch (type) {
            case ROAD: return std::unique_ptr<Transport>(new Truck());
            case SEA:  return std::unique_ptr<Transport>(new Ship());
            case AIR:  return std::unique_ptr<Transport>(new Airplane());
        }
        return nullptr;
    }
};

// 使用：
auto t = SimpleTransportFactory::create(SimpleTransportFactory::SEA);
t->deliver("Cargo");
```

---

### 错误用法

**`factory_bad.cpp`**

```cpp
// 错误 1：工厂方法返回裸指针——谁负责 delete？
class BadFactory : public LogisticsCompany {
public:
    Transport* createTransport() const {  // 裸指针！
        return new Truck();  // 调用者容易忘记 delete → 内存泄漏
    }
};

// 错误 2：工厂方法内部依赖具体类——失去了抽象的意义
class PointlessFactory : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Truck());  // 永远返回 Truck
    }

    void deliver() {
        Truck truck;          // 直接使用具体类！
        truck.deliver("...");  // 那要工厂干什么？
    }
};

// 错误 3：把所有产品塞进一个工厂——违反单一职责
class GodFactory {
public:
    std::unique_ptr<Transport> createTransport(int type) { /*...*/ }
    std::unique_ptr<Animal> createAnimal(int type) { /*...*/ }
    std::unique_ptr<Weapon> createWeapon(int type) { /*...*/ }
    // 运输工具、动物、武器全塞一起——这不是工厂，是杂货铺
};
```

---

### 面试热点

```
Q: 工厂方法的核心价值是什么？
A: 符合开闭原则——新增产品类型时，只需新增一个工厂子类，
   不需要修改已有代码。客户端代码只依赖抽象接口。

Q: 工厂方法和简单工厂的区别？
A: 简单工厂：一个类中用 switch/if 决定创建哪个对象——扩展需修改工厂
   工厂方法：用继承决定创建哪个对象——扩展只需新增子类
   简单工厂适合产品少的场景，工厂方法适合产品种类会增长的场景。

Q: 什么时候不该用工厂方法？
A: 当产品类型固定且不会变化时，简单工厂甚至直接 new 就够了。
   不要为了"设计模式"而使用模式，过度设计比没有设计更糟。
```

---

## 1.4 Observer（观察者模式）

### 定义

> 定义对象间的一对多依赖关系，当一个对象（Subject）的状态发生改变时，所有依赖于它的对象（Observer）都得到通知并自动更新。

### 现实类比

就像**微信公众号**——你关注（subscribe）了一个公众号，当它发布新文章时，所有关注者自动收到推送。你不需要反复去刷新查看，公众号也不需要知道每个读者的具体信息。

### 问题场景

- GUI 事件系统（按钮点击 → 通知所有监听器）
- 股票价格变动（价格变化 → 通知所有订阅的交易者）
- MVC 架构（Model 变化 → 通知所有 View 更新）
- 消息队列（发布者发送消息 → 所有订阅者接收）

### 实现

#### 基础版：温度监控系统

**`observer.cpp`**

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

// ========== 观察者接口 ==========

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(float temperature, float humidity) = 0;
    virtual std::string getName() const = 0;
};

// ========== 被观察者（Subject） ==========

class WeatherStation {
private:
    std::vector<Observer*> observers_;  // 不拥有观察者的生命周期
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
        std::cout << observer->getName() << " unsubscribed\n";
    }

    // 通知所有观察者
    void notifyAll() {
        for (auto* obs : observers_) {
            obs->update(temperature_, humidity_);
        }
    }

    // 状态变化时自动通知
    void setMeasurements(float temp, float humidity) {
        std::cout << "\n--- Weather update ---\n";
        temperature_ = temp;
        humidity_ = humidity;
        notifyAll();  // 状态变化 → 自动通知
    }
};
```

**`observer.cpp`（续）**

```cpp
// ========== 具体观察者 ==========

// 观察者 A：手机 App 显示
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

// 观察者 B：窗户显示屏
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

// 观察者 C：温度预警系统
class AlertSystem : public Observer {
private:
    float threshold_;

public:
    explicit AlertSystem(float threshold) : threshold_(threshold) {}

    void update(float temperature, float humidity) override {
        if (temperature > threshold_) {
            std::cout << "[ALERT] Temperature " << temperature
                      << "°C exceeds threshold " << threshold_ << "°C!\n";
        }
    }

    std::string getName() const override { return "AlertSystem"; }
};
```

**`observer.cpp`** — 客户端代码：

```cpp
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

    // 气象站更新数据 → 所有观察者自动收到通知
    station.setMeasurements(25.0f, 65.0f);
    // Output:
    // --- Weather update ---
    // [Phone:Alice] 25°C, 65% humidity
    // [Phone:Bob] 25°C, 65% humidity
    // [Window:Lobby] Temp=25°C
    // （AlertSystem 不输出，因为未超阈值）

    // Bob 取消订阅
    station.unsubscribe(&bob);

    // 再次更新——Bob 不再收到通知
    station.setMeasurements(38.0f, 40.0f);
    // Output:
    // --- Weather update ---
    // [Phone:Alice] 38°C, 40% humidity
    // [Window:Lobby] Temp=38°C
    // [ALERT] Temperature 38°C exceeds threshold 35°C!
}
```

---

### 推模型 vs 拉模型

| 模型 | 通知方式 | 优缺点 |
|------|---------|--------|
| **推模型 (Push)** | Subject 将数据直接推送给 Observer | 简单直接，但 Observer 可能收到不需要的数据 |
| **拉模型 (Pull)** | Subject 只通知"有变化"，Observer 自行查询需要的数据 | 灵活，但增加了耦合（Observer 需要知道 Subject） |

上面的实现是**推模型**——直接把 temperature 和 humidity 推给观察者。

拉模型的变体：

**`pull_observer.cpp`**

```cpp
// 拉模型——Observer 只收到"有变化"的通知，自己去拉数据
class PullObserver {
public:
    virtual ~PullObserver() = default;
    virtual void update(const WeatherStation& station) = 0;  // 传 Subject 引用
};

// 观察者自己决定拉取什么
class SmartDisplay : public PullObserver {
    void update(const WeatherStation& station) override {
        // 只关心温度，不关心湿度
        float temp = station.getTemperature();
        std::cout << "Temperature: " << temp << "\n";
    }
};
```

---

### 错误用法

**`observer_bad.cpp`**

```cpp
// 错误 1：观察者析构后未取消订阅——悬空指针
class DanglingObserver {
    WeatherStation* station_;
public:
    DanglingObserver(WeatherStation* s) : station_(s) {
        station_->subscribe(this);
    }
    ~DanglingObserver() {
        // 忘记 station_->unsubscribe(this);
        // 析构后 station_ 仍持有 this 指针 → 下次通知时访问已释放内存！
    }
};

// 错误 2：通知过程中修改观察者列表——迭代器失效
class UnsafeStation {
    std::vector<Observer*> observers_;
public:
    void notifyAll() {
        for (auto* obs : observers_) {
            obs->update(0, 0);
            // 如果 update() 内部调用了 unsubscribe()，
            // observers_ 被修改，迭代器失效 → 崩溃！
        }
    }
};

// 错误 3：循环通知——A 通知 B，B 又通知 A → 死循环
class LoopingObserver : public Observer {
    WeatherStation* station_;
public:
    void update(float temp, float humidity) override {
        // 收到通知后又去修改 station → 又触发通知 → 无限循环
        station_->setMeasurements(temp + 1, humidity);
    }
};
```

---

### 面试热点

```
Q: 观察者模式和发布-订阅模式的区别？
A: 观察者模式：Subject 直接知道 Observer，紧耦合
   发布-订阅：中间有一个 EventBus/Broker，Publisher 和 Subscriber 互不知道
   观察者模式是发布-订阅的简化版本。

Q: 如何避免观察者析构后的悬空指针？
A: 1) 在析构函数中调用 unsubscribe()
   2) 使用 weak_ptr 存储观察者引用，通知前检查是否有效
   3) RAII：用一个 Subscription 对象管理订阅生命周期

Q: 通知顺序有保证吗？
A: GoF 没有规定通知顺序。通常按注册顺序通知，
   但不应依赖这个顺序——如果需要顺序，使用责任链模式。
```

---

## 1.5 本课总结

### 三种模式的对比

| 模式 | 类别 | 核心问题 | 解决方案 | 关键词 |
|------|------|---------|---------|--------|
| **Singleton** | 创建型 | 全局唯一实例 | 私有构造 + 静态访问点 | "唯一" |
| **Factory Method** | 创建型 | 创建逻辑与使用逻辑耦合 | 子类决定创建哪种对象 | "延迟" |
| **Observer** | 行为型 | 对象间一对多通知 | 发布-订阅机制 | "通知" |

### 三种模式体现的设计原则

| 原则 | Singleton | Factory Method | Observer |
|------|-----------|---------------|----------|
| 面向接口编程 | — | 客户端依赖抽象产品和抽象工厂 | 客户端依赖 Observer 接口 |
| 开闭原则 | — | 新增产品不修改已有代码 | 新增观察者不修改 Subject |
| 单一职责 | 管理自身唯一性 | 将创建逻辑从业务逻辑中分离 | Subject 只负责通知，Observer 自行处理 |

### 本课核心收获

1. **Singleton** 解决"创建几个"的问题——答案是"只创建一个"
2. **Factory Method** 解决"谁来创建"的问题——答案是"让子类决定"
3. **Observer** 解决"如何通知"的问题——答案是"自动推送"

---

## 1.6 课后练习

1. **Singleton**：实现一个线程安全的 `ConfigManager` 单例：
   - 使用 Meyers' Singleton
   - 支持 `set(key, value)` 和 `get(key)` 操作
   - 内部使用 `std::unordered_map<std::string, std::string>` 存储配置
   - 加分：使用 `std::shared_mutex` 实现读写锁，允许多线程并发读取

2. **Factory Method**：实现一个"文档解析器"工厂：
   - 抽象产品：`Document`，有 `parse()` 和 `getContent()` 方法
   - 具体产品：`PDFDocument`、`WordDocument`、`MarkdownDocument`
   - 抽象工厂：`DocumentFactory`，有 `createDocument(filename)` 工厂方法
   - 具体工厂：根据文件扩展名决定创建哪种 Document

3. **Observer**：实现一个"股票价格监控系统"：
   - Subject：`StockMarket`，维护多只股票的价格
   - Observer A：`PriceDisplay`——显示股票当前价格
   - Observer B：`TradeBot`——价格低于阈值时自动买入，输出"BUY"
   - Observer C：`EmailAlert`——价格变化超过 5% 时发送邮件通知
   - 支持按股票代码订阅（不是所有股票变化都通知所有观察者）
