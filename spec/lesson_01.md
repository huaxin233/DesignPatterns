# 第 1 课：开篇与三大基础模式

> **前置知识**：C++ 基础（类、继承、virtual 函数、智能指针、std::mutex）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Singleton、Factory Method、Observer 三种最常用的基础模式

---

## 1.1 课程导入

### 什么是设计模式？

设计模式是**针对特定问题的通用解决方案**，由 Gang of Four（四人帮）在 1994 年提出。模式不是代码，而是**解决问题的思想**。

### 三类模式总览

| 类别 | 关注点 | 代表模式 |
|------|--------|---------|
| **创建型** | 对象如何创建 | Singleton、Factory Method、Builder… |
| **结构型** | 对象如何组合 | Adapter、Decorator、Facade… |
| **行为型** | 对象如何通信 | Observer、Strategy、Command… |

---

## 1.2 Singleton（单例模式）

### 定义

> 确保一个类只有一个实例，并提供一个全局访问点。

### 现实类比

就像**联合国**——世界只有一个，任何国家想联系联合国都通过同一个入口，不能存在两个"联合国"。

### 问题场景

- 数据库连接池（全系统共用一个）
- 日志记录器（所有模块写同一份日志）
- 配置管理器（全局配置只加载一次）

### 正确实现

#### 版本 1：基础线程安全版

```cpp
#include <iostream>
#include <mutex>

class Singleton {
private:
    static Singleton* instance;  // 单例指针
    static std::mutex mutex_;   // 互斥锁

    // 私有构造函数，防止外部 new
    Singleton() {
        std::cout << "Singleton instance created\n";
    }

    // 删除拷贝构造函数和赋值运算符（防止副本）
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance == nullptr) {
            instance = new Singleton();
        }
        return instance;
    }

    void doSomething() {
        std::cout << "Singleton doing something\n";
    }
};

// 静态成员初始化
Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mutex_;
```

**问题**：每次调用都要加锁，开销大。

---

#### 版本 2：双重检查锁定（Double-Checked Locking）

```cpp
#include <iostream>
#include <mutex>

class Singleton {
private:
    static Singleton* instance;
    static std::mutex mutex_;

    Singleton() {
        std::cout << "Singleton instance created\n";
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton* getInstance() {
        // 第一次检查：不加锁快速判断
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            // 第二次检查：加锁后再确认（防止多线程重复创建）
            if (instance == nullptr) {
                instance = new Singleton();
            }
        }
        return instance;
    }

    void doSomething() {
        std::cout << "Singleton doing something\n";
    }
};

Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mutex_;
```

**原理**：
1. 第一次 `instance == nullptr` 快速路径——实例已创建时直接返回，不加锁
2. 第二次 `instance == nullptr` 在加锁后确认，防止多个线程同时通过第一层检查

---

#### 版本 3：C++11 局部静态变量（推荐）

```cpp
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
        // C++11 保证线程安全，Magic Static
        static Singleton instance;
        return instance;
    }

    void doSomething() {
        std::cout << "Singleton doing something\n";
    }
};
```

**优势**：最简洁、C++11 保证线程安全、无需手动管理生命周期。

> **注意**：返回引用而非指针，调用者无需手动 `delete`。

---

### 使用示例

```cpp
int main() {
    Singleton& s1 = Singleton::getInstance();
    Singleton& s2 = Singleton::getInstance();

    s1.doSomething();

    // s1 和 s2 是同一个对象
    std::cout << "Same instance? " << (&s1 == &s2 ? "Yes" : "No") << "\n";
    // Output: Same instance? Yes
}
```

---

### 错误用法

```cpp
// 错误 1：直接调用构造函数（编译错误被 delete 挡住）
Singleton s;  // 编译错误！构造函数是 private

// 错误 2：在多线程环境下不使用双检锁
class BadSingleton {
    static BadSingleton* instance;
    static std::mutex mutex_;
public:
    static BadSingleton* getInstance() {
        // 危险！多线程可能创建多个实例
        if (instance == nullptr) {
            instance = new BadSingleton();
        }
        return instance;
    }
};

// 错误 3：返回指针且调用者可能 delete
Singleton* s = Singleton::getInstance();
delete s;  // 危险！下次 getInstance 会创建悬挂指针
```

---

### 面试热点

```
Q: Singleton 的缺点是什么？
A:
   1. 违反单一职责原则（控制创建 + 业务逻辑）
   2. 全局状态，隐藏依赖，难以测试
   3. 在多线程环境下需要小心处理
   4. 难以扩展为多个实例（如 Redis 主从集群）
```

---

## 1.3 Factory Method（工厂方法模式）

### 定义

> 定义一个创建对象的接口，让子类决定实例化哪个类。工厂方法使一个类的实例化延迟到其子类。

### 现实类比

就像**手机工厂**——总厂只定义"生产手机"的流程，具体生产哪款手机（iPhone / 小米 / 华为）由各分厂决定。消费者不需要知道手机是谁造的。

### 问题场景

- UI 框架创建按钮（WindowsButton / MacButton）
- 数据库驱动创建连接（MySQLConnection / PostgreSQLConnection）
- 日志框架创建 Logger（FileLogger / ConsoleLogger）

### 实现

#### 框架代码

```cpp
#include <iostream>
#include <memory>
#include <string>

// 产品抽象基类
class Product {
public:
    virtual ~Product() = default;
    virtual void use() const = 0;
};

// 具体产品 A
class ConcreteProductA : public Product {
public:
    void use() const override {
        std::cout << "Using ConcreteProductA\n";
    }
};

// 具体产品 B
class ConcreteProductB : public Product {
public:
    void use() const override {
        std::cout << "Using ConcreteProductB\n";
    }
};

// 创建者抽象类
class Creator {
public:
    virtual ~Creator() = default;

    // 工厂方法——子类决定创建什么产品
    virtual std::unique_ptr<Product> factoryMethod() const = 0;

    // 使用产品的模板逻辑
    void doSomething() const {
        auto product = factoryMethod();
        product->use();
    }
};

// 具体创建者 A——创建产品 A
class ConcreteCreatorA : public Creator {
public:
    std::unique_ptr<Product> factoryMethod() const override {
        return std::make_unique<ConcreteProductA>();
    }
};

// 具体创建者 B——创建产品 B
class ConcreteCreatorB : public Creator {
public:
    std::unique_ptr<Product> factoryMethod() const override {
        return std::make_unique<ConcreteProductB>();
    }
};
```

#### 客户端代码

```cpp
int main() {
    std::unique_ptr<Creator> creator;

    // 根据配置或条件选择创建者
    std::string type = "A";
    if (type == "A") {
        creator = std::make_unique<ConcreteCreatorA>();
    } else {
        creator = std::make_unique<ConcreteCreatorB>();
    }

    creator->doSomething();
}
```

**核心思想**：把"创建哪个类的实例"推迟到子类决定。

---

### 简单工厂 vs 工厂方法 vs 抽象工厂

| 模式 | 复杂度 | 创建范围 | 扩展方式 |
|------|--------|---------|---------|
| **Simple Factory** | 一个工厂类 | 一种产品（多类型分支） | 修改工厂类（违反开闭原则） |
| **Factory Method** | 每种产品一个创建者类 | 一种产品的多个变体 | 添加新创建者子类 |
| **Abstract Factory** | 多个产品族工厂 | 多个产品的家族 | 添加新产品族（复杂） |

---

### 错误用法

```cpp
// 错误 1：在工厂方法中违反里氏替换原则
class BadCreator : public Creator {
public:
    std::unique_ptr<Product> factoryMethod() const override {
        // 返回了错误类型，违反 LSP
        return std::make_unique<ConcreteProductB>(); // A 工厂返回 B 产品！
    }
};

// 错误 2：工厂方法返回裸指针且不管理生命周期
class BadCreator2 : public Creator {
public:
    Product* factoryMethod() const override {
        return new ConcreteProductA();  // 调用者可能忘记 delete
    }
};

// 错误 3：创建逻辑暴露给客户端，而不是子类
class ExposedCreator : public Creator {
public:
    // 工厂方法不纯——客户端可以跳过它直接 new
};
```

---

### 面试热点

```
Q: 工厂方法模式的核心作用是什么？
A: 解耦客户端代码与具体产品类。将"对象创建"延迟到子类，
   客户端只依赖抽象 Product，不依赖具体实现。
```

---

## 1.4 Observer（观察者模式）

### 定义

> 定义对象间的一种一对多依赖关系，当一个对象状态改变时，所有依赖它的对象都会收到通知并自动更新。

### 现实类比

就像**微信公众号订阅**——你关注一个公众号后，公众号发文章时所有订阅者都会收到通知。你不需要每次去检查有没有新文章，公众号会自动推送给所有订阅者。

### 问题场景

- GUI 事件系统（按钮点击 → 多个监听器响应）
- MVC 架构（Model 变化 → View 自动更新）
- 消息推送系统（用户关注 → 新消息通知）
- 股票行情系统（价格变动 → 所有订阅者收到更新）

### 实现

```cpp
#include <iostream>
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

// 具体观察者 A
class ConcreteObserverA : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "ConcreteObserverA received: " << message << "\n";
    }
};

// 具体观察者 B
class ConcreteObserverB : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "ConcreteObserverB received: " << message << "\n";
    }
};

// 主题（被观察者）
class Subject {
private:
    std::vector<Observer*> observers_;  // 观察者列表
    std::string state_;                 // 内部状态

public:
    // 订阅
    void attach(Observer* observer) {
        observers_.push_back(observer);
    }

    // 取消订阅
    void detach(Observer* observer) {
        // 简化：移除第一个匹配的观察者
        for (auto it = observers_.begin(); it != observers_.end(); ++it) {
            if (*it == observer) {
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
        for (Observer* observer : observers_) {
            observer->update(state_);
        }
    }
};
```

**客户端代码**：

```cpp
int main() {
    Subject subject;

    ConcreteObserverA observerA;
    ConcreteObserverB observerB;

    subject.attach(&observerA);
    subject.attach(&observerB);

    subject.setState("Hello World");
    // Output:
    // ConcreteObserverA received: Hello World
    // ConcreteObserverB received: Hello World

    subject.detach(&observerA);

    subject.setState("Goodbye");
    // Output:
    // ConcreteObserverB received: Goodbye
}
```

---

### C++ 智能指针版本（更安全）

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class Subject {
private:
    std::vector<std::weak_ptr<Observer>> observers_;  // weak_ptr 防止循环引用
    std::string state_;

public:
    void subscribe(const std::shared_ptr<Observer>& observer) {
        observers_.push_back(observer);
    }

    void unsubscribe(const std::shared_ptr<Observer>& observer) {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const std::weak_ptr<Observer>& wptr) {
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
        for (auto it = observers_.begin(); it != observers_.end(); ) {
            if (auto obs = it->lock()) {
                obs->update(state_);
                ++it;
            } else {
                // 观察者已销毁，清理 weak_ptr
                it = observers_.erase(it);
            }
        }
    }
};
```

> **为什么用 `weak_ptr`**：Subject 持有 Observer 的 `shared_ptr` 会阻止 Observer 被销毁；Observer 持有 Subject 的引用也不合理。`weak_ptr` 打破循环引用，且能检测对象是否已销毁。

---

### 错误用法

```cpp
// 错误 1：观察者在通知过程中被销毁（悬挂指针）
class BadObserver : public Observer {
    Subject* subject_;
public:
    BadObserver(Subject* s) : subject_(s) {}
    void update(const std::string& message) override {
        subject_->detach(this);  // 从通知中删除自己！危险
    }
};

// 错误 2：通知时修改观察者列表（迭代器失效）
class BadSubject : public Subject {
public:
    void badNotify() {
        for (auto& observer : observers_) {
            observers_.push_back(new ConcreteObserverA);  // 迭代器失效！
        }
    }
};

// 错误 3：违反开闭原则——每次新观察者类型都要修改 Subject
class RigidSubject {
    std::vector<ConcreteObserverA*> observersA_;  // 类型绑死
    std::vector<ConcreteObserverB*> observersB_;
    // 每次加新观察者类型都要改这个类
};
```

---

### 面试热点

```
Q: 观察者模式的优点和缺点？
A:
   优点：一对多解耦、动态订阅/退订、自动广播通知
   缺点：观察者过多时通知耗时、可能产生循环引用、
        观察者顺序不确定、调试困难

Q: Observable（拉模型）vs Push 模型？
A:
   Push: Subject 主动推送给 Observer，Observer 被动接收
   Pull: Observer 按需从 Subject 拉取数据
   混合：推送变更摘要，Observer 按需拉取完整数据
```

---

## 1.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Singleton** | 全局唯一实例 | 私有构造函数 + static 实例 | 全局访问 |
| **Factory Method** | 对象创建解耦 | 子类决定创建类型 | 延迟到子类 |
| **Observer** | 一对多自动通知 | 发布-订阅 + 回调 | 状态同步 |

### 统一思想

三种模式都在解决**"对象与对象之间的关系应该由谁控制"**：

- Singleton：**控制实例数量**——全局唯一
- Factory Method：**控制谁创建**——从"谁需要谁创建"变成"谁决定谁创建"
- Observer：**控制通知时机和方式**——从"主动查询"变成"自动推送"

### 下一课预告

第 2 课我们将学习三个结构型模式：**Adapter**、**Decorator**、**Facade**，理解如何通过对象组合来改变对象结构和添加职责。

---

## 1.6 课后练习

1. **Singleton**：实现一个线程安全的配置管理器 `ConfigManager`，读取一次配置文件后全局复用。

2. **Factory Method**：实现一个物流系统，`RoadLogistics` 创建卡车，`SeaLogistics` 创建轮船，客户端只调用 `createTransport()` 不需要知道具体类型。

3. **Observer**：实现一个气象站系统，`WeatherStation`（Subject）推送温度、湿度、气压，`PhoneDisplay` 和 `WebDisplay`（Observer）分别显示。

---
