# 第 2 课：结构型模式入门

> **前置知识**：第 1 课内容（类层次结构、继承、多态、智能指针）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Adapter、Decorator、Facade 三种结构型模式，理解"组合优于继承"的核心思想

---

## 2.1 课程导入

### 结构型模式的核心问题

创建型模式解决"对象如何创建"，行为型模式解决"对象如何通信"，而**结构型模式**解决的是：

> **已有的对象和类如何组合成更大的结构？**

三种主要的组合手段：

| 手段 | 解决的问题 | 代表模式 |
|------|---------|---------|
| **接口转换** | 已有接口不兼容 | Adapter |
| **动态添加职责** | 继承导致类爆炸 | Decorator |
| **统一入口** | 子系统太复杂 | Facade |

---

## 2.2 Adapter（适配器模式）

### 定义

> 将一个类的接口转换成客户期望的另一个接口，使原本接口不兼容的类可以一起工作。

### 现实类比

就像**电源适配器**——笔记本电脑需要 220V 交流电，但手机需要 5V 直流电。适配器把 220V 转换成 5V，让不兼容的两者能够合作。你不需要知道手机内部是怎么工作的，适配器帮你做好转换。

### 问题场景

- 集成第三方库（库接口与项目接口不一致）
- 重构时保留旧接口（新版 API 变了，但旧代码还要用）
- 多数据源统一访问（MySQL、MongoDB、Redis 接口不同）

### 实现

#### 类适配器（通过继承）

```cpp
#include <iostream>
#include <string>

// 目标接口：客户端期望的接口
class Target {
public:
    virtual ~Target() = default;
    virtual void request(const std::string& data) const = 0;
};

// 待适配类：已经存在的类，接口不兼容
class Adaptee {
public:
    void specificRequest(const std::string& data) {
        std::cout << "Adaptee: Received '" << data << "'\n";
    }
};

// 适配器：通过继承 Adaptee，实现 Target 接口
class Adapter : public Target, private Adaptee {
public:
    void request(const std::string& data) const override {
        // 将目标接口转换为被适配接口
        std::string transformed = "Adapted: " + data;
        specificRequest(transformed);
    }
};
```

#### 对象适配器（通过组合）

```cpp
#include <iostream>
#include <string>
#include <memory>

class Target {
public:
    virtual ~Target() = default;
    virtual void request(const std::string& data) const = 0;
};

class Adaptee {
public:
    void specificRequest(const std::string& data) {
        std::cout << "Adaptee: Received '" << data << "'\n";
    }
};

// 适配器：通过组合持有 Adaptee 对象
class ObjectAdapter : public Target {
private:
    std::unique_ptr<Adaptee> adaptee_;

public:
    ObjectAdapter() : adaptee_(std::make_unique<Adaptee>()) {}

    void request(const std::string& data) const override {
        std::string transformed = "Adapted: " + data;
        adaptee_->specificRequest(transformed);
    }
};
```

**客户端代码**：

```cpp
int main() {
    std::unique_ptr<Target> target = std::make_unique<Adapter>();
    target->request("Hello");
    // Output: Adaptee: Received 'Adapted: Hello'
}
```

---

### 类适配器 vs 对象适配器

| 维度 | 类适配器 | 对象适配器 |
|------|---------|----------|
| **机制** | 多继承（public Target + private Adaptee） | 组合（持有 Adaptee 指针） |
| **灵活性** | 只能适配一个 Adaptee | 可以适配多个 Adaptee |
| **覆盖行为** | 可以直接覆盖 Adaptee 方法 | 需要通过转发 |
| **推荐场景** | Adaptee 接口较稳定 | Adaptee 需要被复用或扩展 |

> **现代 C++ 推荐**：对象适配器更灵活，且避免多继承的复杂性。

---

### 错误用法

```cpp
// 错误 1：适配器直接暴露 Adaptee，违反封装
class BadAdapter : public Target {
public:
    Adaptee* adaptee;  // 直接暴露！客户端可以直接操作 Adaptee
};

// 错误 2：适配器添加了被适配接口没有的功能（职责混乱）
class OverloadedAdapter : public Target {
    // 错误！适配器不应该添加新功能
    void extraFunction() {
        // 这不是适配，是扩展
    }
};

// 错误 3：双向适配器没有清晰定义两组接口
class AmbiguousAdapter : public Target, public Adaptee {
    // 双向适配容易产生歧义，难以维护
};
```

---

### 面试热点

```
Q: 适配器模式与装饰器模式的区别？
A: 适配器是"改接口"，让不兼容变得兼容。
   装饰器是"加功能"，在原有功能基础上动态添加新职责。
   两者都是为了扩展，但动机不同。

Q: 适配器模式在标准库中的应用？
A: std::unique_ptr<T[]> 可以当作 T* 使用（接口兼容）。
   C++20 的 std::ranges 与传统迭代器之间的适配。
```

---

## 2.3 Decorator（装饰器模式）

### 定义

> 动态地给对象添加一些额外的职责，就增加功能来说，装饰器比继承更灵活。

### 现实类比

就像**手机壳**——你买了一个基础款手机，但想要更多功能：
- 加个防水壳 → 防水功能
- 加个充电壳 → 充电功能
- 加个指环扣 → 支架功能

你可以任意组合这些装饰，而且不用修改手机本身（不用"继承"出一个"防水充电指环手机"）。

### 问题场景

- IO 流体系（`std::istream` + `std::buf_isstream` + `std::cin`）
- GUI 组件装饰（窗口 + 滚动条 + 边框）
- 日志增强（基础日志 + 时间戳 + 加密）
- Web 中间件（请求 + 认证 + 缓存 + 日志）

### 实现

#### 基础框架

```cpp
#include <iostream>
#include <string>
#include <memory>

// 组件抽象基类
class Component {
public:
    virtual ~Component() = default;
    virtual std::string operation() const = 0;
};

// 具体组件——最基本的对象
class ConcreteComponent : public Component {
public:
    std::string operation() const override {
        return "ConcreteComponent";
    }
};

// 装饰器基类——持有组件引用
class Decorator : public Component {
protected:
    std::unique_ptr<Component> component_;

public:
    explicit Decorator(std::unique_ptr<Component> comp)
        : component_(std::move(comp)) {}

    std::string operation() const override {
        return component_->operation();
    }
};

// 具体装饰器 A——添加职责 A
class ConcreteDecoratorA : public Decorator {
public:
    using Decorator::Decorator;  // 继承构造函数

    std::string operation() const override {
        return "DecoratorA(" + Decorator::operation() + ")";
    }
};

// 具体装饰器 B——添加职责 B
class ConcreteDecoratorB : public Decorator {
public:
    using Decorator::Decorator;

    std::string operation() const override {
        return "DecoratorB(" + Decorator::operation() + ")";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 基础组件
    std::unique_ptr<Component> base = std::make_unique<ConcreteComponent>();
    std::cout << base->operation() << "\n";
    // Output: ConcreteComponent

    // 装饰一层 A
    std::unique_ptr<Component> withA = std::make_unique<ConcreteDecoratorA>(
        std::make_unique<ConcreteComponent>());
    std::cout << withA->operation() << "\n";
    // Output: DecoratorA(ConcreteComponent)

    // 装饰两层 A + B
    std::unique_ptr<Component> withAB = std::make_unique<ConcreteDecoratorB>(
        std::make_unique<ConcreteDecoratorA>(
            std::make_unique<ConcreteComponent>()));
    std::cout << withAB->operation() << "\n";
    // Output: DecoratorB(DecoratorA(ConcreteComponent))
}
```

---

### 装饰器的叠加顺序

```
原对象 → DecoratorA → DecoratorB → DecoratorC
```

装饰顺序不同，结果可能不同：

```cpp
// 顺序 1：A 装饰 B
DecoratorA(DecoratorB(base))  // A 包裹 B

// 顺序 2：B 装饰 A
DecoratorB(DecoratorA(base))  // B 包裹 A
```

**注意**：如果 A 是"加密"，B 是"压缩"，顺序不同会导致结果不同！

---

### 装饰器 vs 继承

| 维度 | 继承 | 装饰器 |
|------|------|--------|
| **编译时 vs 运行时** | 编译时静态绑定 | 运行时动态组合 |
| **类数量** | 每个功能组合需要一个子类 | 只需 N 个装饰器类 |
| **灵活性** | 运行时无法改变 | 可以随时添加/移除装饰器 |
| **对象身份** | 始终是子类类型 | 仍是原始类型（is-a 成立） |

```cpp
// 继承的问题：类爆炸
class TextFieldWithBorder {};
class TextFieldWithScroll {};
class TextFieldWithBorderAndScroll {};  // 又一个类！
class TextFieldWithBorderAndScrollAndColor {};  // 爆炸！

// 装饰器的解决方案：自由组合
auto textField = std::make_unique<ColorDecorator>(
    std::make_unique<ScrollDecorator>(
        std::make_unique<BorderDecorator>(
            std::make_unique<TextField>())));
```

---

### 错误用法

```cpp
// 错误 1：装饰器类自己添加成员变量但未在构造时初始化
class ForgetfulDecorator : public Decorator {
public:
    using Decorator::Decorator;
    // 错误！假设某个成员变量由被装饰对象提供，但未初始化
    void badOperation() {
        // component_->may not be properly initialized
    }
};

// 错误 2：在装饰器中创建新对象而非持有引用（失去装饰意义）
class WrongDecorator : public Decorator {
public:
    std::string operation() const override {
        // 错误！应该委托给 component_，而不是自己 new
        auto newComp = std::make_unique<ConcreteComponent>();
        return "Decorated: " + newComp->operation();
    }
};

// 错误 3：装饰器链过长，导致调试困难
auto chain = std::make_unique<D1>(
    std::make_unique<D2>(
        std::make_unique<D3>(
            std::make_unique<D4>(
                std::make_unique<D5>(
                    std::make_unique<Base>()  // 太长了！难以调试
                ))))));
```

---

### 面试热点

```
Q: 装饰器模式和代理模式的区别？
A: 代理模式侧重"控制访问"（是否允许访问原对象）。
   装饰器模式侧重"添加功能"（在访问时增加新行为）。
   两者结构相似，但目的不同。

Q: 装饰器在 C++ iostream 中的应用？
A: std::cin 是基础流，std::getline 是装饰器（按行读取），
   std::ostream 是基础，std::cout 是全局对象。
   实际是装饰器思想的体现。
```

---

## 2.4 Facade（外观模式）

### 定义

> 为子系统中的一组接口提供一个统一的高层接口，使子系统更易于使用。

### 现实类比

就像**汽车仪表盘**——汽车内部有几百个零件（发动机、变速箱、ABS、空调、音响...），但驾驶员只需要看仪表盘上的几个旋钮和按钮就能操作汽车。仪表盘就是"外观"，隐藏了内部复杂性。

### 问题场景

- 复杂库或框架的封装（游戏引擎、图形库、音频引擎）
- 多系统集成（订单系统 + 库存系统 + 支付系统 → 统一下单接口）
- 遗留系统改造（旧系统接口混乱，用 Facade 统一新接口）

### 实现

#### 复杂子系统（模拟）

```cpp
#include <iostream>
#include <string>
#include <memory>

// 子系统组件 1：CPU
class CPU {
public:
    void freeze() {
        std::cout << "CPU: Freezing...\n";
    }

    void execute(long address) {
        std::cout << "CPU: Executing at address " << address << "\n";
    }
};

// 子系统组件 2：内存
class Memory {
public:
    void load(long address, const std::string& data) {
        std::cout << "Memory: Loading '" << data << "' at " << address << "\n";
    }
};

// 子系统组件 3：硬盘
class HardDrive {
public:
    std::string read(long lba, int size) {
        std::cout << "HardDrive: Reading " << size << " bytes from sector " << lba << "\n";
        return "boot_data";
    }
};
```

#### 外观类（统一入口）

```cpp
// 外观类——提供统一的高层接口
class ComputerFacade {
private:
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<Memory> memory_;
    std::unique_ptr<HardDrive> hardDrive_;

public:
    ComputerFacade() {
        cpu_ = std::make_unique<CPU>();
        memory_ = std::make_unique<Memory>();
        hardDrive_ = std::make_unique<HardDrive>();
    }

    // 高层接口：启动计算机
    void start() {
        std::cout << "=== ComputerFacade: Starting computer ===\n";
        cpu_->freeze();
        memory_->load(0, hardDrive_->read(0, 1024));
        cpu_->execute(0);
        std::cout << "=== ComputerFacade: Boot complete ===\n";
    }

    // 高层接口：关闭计算机
    void shutdown() {
        std::cout << "=== ComputerFacade: Shutting down ===\n";
        // 简化：按正确顺序关闭各组件
        std::cout << "CPU: Power off\n";
        std::cout << "Memory: Power off\n";
        std::cout << "HardDrive: Power off\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    ComputerFacade computer;

    // 客户端只需要调用外观，不需要了解内部子系统
    computer.start();
    // Output:
    // === ComputerFacade: Starting computer ===
    // CPU: Freezing...
    // HardDrive: Reading 1024 bytes from sector 0
    // Memory: Loading 'boot_data' at 0
    // CPU: Executing at address 0
    // === ComputerFacade: Boot complete ===

    computer.shutdown();
}
```

---

### 外观 vs 适配器 vs 装饰器

| 模式 | 目的 | 手段 | 客户端感知 |
|------|------|------|-----------|
| **Facade** | 简化接口 | 组合多个子系统组件 | 不知道子系统存在 |
| **Adapter** | 转换接口 | 包装一个类 | 以为直接用了目标接口 |
| **Decorator** | 添加功能 | 包装一个对象 | 以为用的是原对象 |

---

### 错误用法

```cpp
// 错误 1：Facade 变成"上帝类"——所有子系统都放进去
class GodFacade {
    SubsystemA a;
    SubsystemB b;
    SubsystemC c;
    // ... 50 个子系统！
    // 错误！Facade 应该是一组相关子系统的入口，不是全局对象容器
};

// 错误 2：Facade 只是简单转发，没有真正简化
class UselessFacade {
    SubsystemA a;
public:
    void doA() { a.doA(); }  // 错误！没有任何简化，客户端还不如直接用 a
};

// 错误 3：Facade 阻止客户端访问子系统（过度封装）
class OverFacaded {
    SubsystemA a;
public:
    // sealed 方法阻止客户端使用底层接口
    // 错误！Facade 应该让子系统可访问，只是提供便利
};
```

---

### 面试热点

```
Q: 外观模式的优点和缺点？
A:
   优点：简化高层接口、降低耦合、隐藏复杂性、提供默认实现
   缺点：可能引入不必要的层次、过度封装导致灵活性下降

Q: 外观模式与单例模式的关系？
A: Facade 通常以单例形式存在（整个应用只需要一个外观）。
   但不是绝对的——大型应用可能有多个 Facade 服务不同模块。

Q: 什么情况下不应该用外观模式？
A: 当客户端确实需要细粒度控制子系统组件时，
   或者子系统非常简单不需要简化时。
```

---

## 2.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Adapter** | 接口不兼容 | 包装被适配者，转换接口 | "改接口" |
| **Decorator** | 继承导致类爆炸 | 动态包装，层层装饰 | "加功能" |
| **Facade** | 子系统太复杂 | 提供统一高层入口 | "简化" |

### 统一思想

三种模式都在解决**"如何让已有的东西更好地协同工作"**：

- Adapter：**让不一样的东西能够合作**——接口翻译
- Decorator：**让简单的东西变得丰富**——层层叠加
- Facade：**让复杂的东西变得简单**——统一入口

### 组合优于继承

这三种模式共同体现了面向对象设计的核心原则：

```
继承（静态）  →  组合（动态）
|                    |
└──── 类爆炸          └──── 灵活组合
     编译时绑定             运行时可拆换
```

### 下一课预告

第 3 课我们将学习三个进阶结构型模式：**Composite**、**Proxy**、**Bridge**，继续深入理解对象组合的威力。

---

## 2.6 课后练习

1. **Adapter**：实现一个"文件加密适配器"，将 `LegacyEncryptor`（接口是 `encrypt(std::string)`）适配到 `NewCrypto`（接口是 `protect(File)`）。

2. **Decorator**：实现一个"咖啡订单系统"：
   - `Coffee` 是基础组件
   - `MilkDecorator`、`SugarDecorator`、`WhipDecorator` 各自添加配料
   - 支持任意组合，如"加奶加糖的咖啡"或"加奶加奶油的咖啡"

3. **Facade**：实现一个"家庭影院Facade"：
   - 子系统：`Amplifier`、`DvdPlayer`、`Projector`、`Screen`
   - 提供 `watchMovie()` 和 `endMovie()` 两个高层操作
   - 客户端调用一个方法即可开启/关闭整个系统

---
