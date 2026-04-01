# 第 2 课：结构型模式入门

> **前置知识**：第 1 课内容（类层次结构、继承、多态、智能指针）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Adapter、Decorator、Facade 三种结构型模式，理解"组合优于继承"在结构设计中的威力

---

## 2.1 课程导入

### 结构型模式的核心问题

第 1 课我们学了两种创建型模式（Singleton、Factory Method）和一种行为型模式（Observer）。它们分别解决"如何创建对象"和"对象之间如何通信"。

**结构型模式**解决的是第三个问题：

> **如何把已有的类和对象组合成更大、更有用的结构？**

想象你有一堆积木（已有的类），结构型模式教你如何拼搭它们：

| 组合手段 | 解决的问题 | 代表模式 |
|---------|---------|---------|
| **接口转换** | 已有的接口和期望的接口不一致 | Adapter |
| **动态叠加功能** | 用继承扩展功能会导致类爆炸 | Decorator |
| **提供统一入口** | 子系统太复杂，客户端不该直接操作细节 | Facade |

### 贯穿本课的核心原则

> **组合优于继承**——三种结构型模式都通过"持有另一个对象的引用"来实现功能扩展，而不是通过继承产生新子类。

---

## 2.2 Adapter（适配器模式）

### 定义

> 将一个类的接口转换成客户端期望的另一个接口，使原本不兼容的类可以一起工作。

### 现实类比

就像**电源转换插头**——你从中国带了一台笔记本去英国出差，中国的两脚插头插不进英国的三孔插座。你不可能改造插座（已有系统），也不可能改造笔记本的电源线（第三方设备），但一个转换插头就能让它们协同工作。

### 问题场景

- 集成第三方库（库的接口与项目约定不一致）
- 重构过渡期（新 API 上线，但大量旧代码还在用老接口）
- 多数据源统一访问（MySQL、MongoDB、Redis 各自接口不同，需要统一抽象）
- 跨平台兼容（不同操作系统的文件系统 API 不同）

### 实现

#### 场景：支付系统集成

项目需要一个统一的支付接口，但已有的第三方支付 SDK 接口各不相同：

```cpp
#include <iostream>
#include <memory>
#include <string>

// ========== 目标接口 ==========
// 项目内部统一的支付接口——所有支付方式都要符合这个接口
class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;
    virtual bool pay(const std::string& orderId, double amount) = 0;
    virtual std::string getName() const = 0;
};
```

```cpp
// ========== 第三方 SDK（无法修改） ==========

// 支付宝 SDK——接口是 sendPayment(订单号, 金额, 币种)
class AlipaySDK {
public:
    int sendPayment(const std::string& tradeNo, double yuan, const std::string& currency) {
        std::cout << "Alipay: Processing trade " << tradeNo
                  << ", amount=" << yuan << " " << currency << "\n";
        return 0;  // 0 表示成功
    }
};

// 微信支付 SDK——接口是 unifiedOrder(json 字符串)
class WechatPaySDK {
public:
    bool unifiedOrder(const std::string& jsonBody) {
        std::cout << "WechatPay: Unified order with body: " << jsonBody << "\n";
        return true;  // true 表示成功
    }
};

// Stripe SDK——接口是 createCharge(金额以"分"为单位, 币种, 描述)
class StripeSDK {
public:
    std::string createCharge(int amountInCents, const std::string& currency,
                             const std::string& description) {
        std::cout << "Stripe: Charging " << amountInCents << " cents ("
                  << currency << ") for: " << description << "\n";
        return "ch_success_123";  // 返回 charge ID
    }
};
```

```cpp
// ========== 适配器 ==========

// 支付宝适配器
class AlipayAdapter : public PaymentProcessor {
private:
    std::unique_ptr<AlipaySDK> sdk_;

public:
    AlipayAdapter() : sdk_(std::make_unique<AlipaySDK>()) {}

    bool pay(const std::string& orderId, double amount) override {
        // 将统一接口转换为支付宝 SDK 的接口
        int result = sdk_->sendPayment(orderId, amount, "CNY");
        return result == 0;
    }

    std::string getName() const override { return "Alipay"; }
};

// 微信支付适配器
class WechatPayAdapter : public PaymentProcessor {
private:
    std::unique_ptr<WechatPaySDK> sdk_;

public:
    WechatPayAdapter() : sdk_(std::make_unique<WechatPaySDK>()) {}

    bool pay(const std::string& orderId, double amount) override {
        // 构造微信支付需要的 JSON 格式
        std::string json = "{\"order_id\":\"" + orderId
                         + "\",\"amount\":" + std::to_string(amount) + "}";
        return sdk_->unifiedOrder(json);
    }

    std::string getName() const override { return "WechatPay"; }
};

// Stripe 适配器
class StripeAdapter : public PaymentProcessor {
private:
    std::unique_ptr<StripeSDK> sdk_;

public:
    StripeAdapter() : sdk_(std::make_unique<StripeSDK>()) {}

    bool pay(const std::string& orderId, double amount) override {
        // Stripe 要求金额以"分"为单位
        int cents = static_cast<int>(amount * 100);
        std::string chargeId = sdk_->createCharge(cents, "USD", "Order: " + orderId);
        return !chargeId.empty();
    }

    std::string getName() const override { return "Stripe"; }
};
```

**客户端代码**：

```cpp
// 客户端只依赖 PaymentProcessor 接口——不知道背后是哪个 SDK
void checkout(PaymentProcessor& processor,
              const std::string& orderId, double amount) {
    std::cout << "Using " << processor.getName() << " to pay...\n";
    bool success = processor.pay(orderId, amount);
    std::cout << "Result: " << (success ? "SUCCESS" : "FAILED") << "\n\n";
}

int main() {
    AlipayAdapter alipay;
    WechatPayAdapter wechat;
    StripeAdapter stripe;

    checkout(alipay, "ORD-001", 99.9);
    // Output:
    // Using Alipay to pay...
    // Alipay: Processing trade ORD-001, amount=99.9 CNY
    // Result: SUCCESS

    checkout(wechat, "ORD-002", 49.5);
    // Output:
    // Using WechatPay to pay...
    // WechatPay: Unified order with body: {"order_id":"ORD-002","amount":49.500000}
    // Result: SUCCESS

    checkout(stripe, "ORD-003", 29.99);
    // Output:
    // Using Stripe to pay...
    // Stripe: Charging 2999 cents (USD) for: Order: ORD-003
    // Result: SUCCESS
}
```

---

### 类适配器 vs 对象适配器

上面的实现是**对象适配器**（通过组合持有 SDK 对象）。还有一种**类适配器**（通过多继承）：

```cpp
// 类适配器：同时继承目标接口和被适配类
class AlipayClassAdapter : public PaymentProcessor, private AlipaySDK {
public:
    bool pay(const std::string& orderId, double amount) override {
        return sendPayment(orderId, amount, "CNY") == 0;  // 直接调用继承来的方法
    }
    std::string getName() const override { return "Alipay"; }
};
```

| 维度 | 类适配器（继承） | 对象适配器（组合） |
|------|----------------|------------------|
| **机制** | `public Target, private Adaptee` | 持有 `Adaptee` 的指针/引用 |
| **灵活性** | 只能适配一个 Adaptee 类 | 可以适配 Adaptee 及其所有子类 |
| **覆盖能力** | 可以直接覆盖 Adaptee 的方法 | 需要通过转发 |
| **推荐程度** | 适合 Adaptee 简单且稳定的场景 | **现代 C++ 推荐**，更符合"组合优于继承" |

---

### 错误用法

```cpp
// 错误 1：适配器暴露了被适配者的内部细节
class LeakyAdapter : public PaymentProcessor {
public:
    AlipaySDK sdk;  // public 成员！客户端可以直接操作 SDK
                    // 绕过适配器 → 适配层形同虚设

    bool pay(const std::string& orderId, double amount) override {
        return sdk.sendPayment(orderId, amount, "CNY") == 0;
    }
    std::string getName() const override { return "Alipay"; }
};

// 错误 2：适配器中添加了被适配接口没有的新功能
class OverreachingAdapter : public PaymentProcessor {
    std::unique_ptr<AlipaySDK> sdk_;
public:
    bool pay(const std::string& orderId, double amount) override { /*...*/ return true; }
    std::string getName() const override { return "Alipay"; }

    // 错误！适配器不该发明新功能，那是 Decorator 的职责
    void sendMarketingEmail(const std::string& userId) {
        std::cout << "Sending email to " << userId << "\n";
    }
};

// 错误 3：适配不完整——只适配了部分接口，留下隐患
class PartialAdapter : public PaymentProcessor {
public:
    bool pay(const std::string& orderId, double amount) override {
        // 忽略了 orderId，调用时 SDK 侧无法关联订单
        return true;  // 假装成功
    }
    std::string getName() const override { return "Partial"; }
};
```

---

### 面试热点

```
Q: 适配器模式与装饰器模式的区别？
A: 适配器是"改接口"——让不兼容变得兼容，不改变原有功能。
   装饰器是"加功能"——接口不变，在原有功能之上叠加新行为。
   一个改形状，一个加内容。

Q: 什么时候用适配器，什么时候直接改源码？
A: 当被适配的类无法修改时（第三方库、遗留代码、跨团队依赖），用适配器。
   如果是自己团队可控的代码，直接统一接口更简单。

Q: C++ 标准库中有哪些适配器的例子？
A: std::stack 和 std::queue 是容器适配器——它们不是独立的数据结构，
   而是将 deque/vector 的接口适配成栈/队列的接口。
   std::bind 和 lambda 也是函数适配器——将一种函数签名适配为另一种。
```

---

## 2.3 Decorator（装饰器模式）

### 定义

> 动态地给对象添加额外的职责。就增加功能而言，装饰器模式比继承更灵活。

### 现实类比

就像**手机壳**——你买了一个基础款手机，想要更多功能：

- 加个防摔壳 → 防摔能力
- 再套个充电壳 → 无线充电能力
- 再贴个指环扣 → 支架能力

你可以任意组合这些"壳"，不用改手机本身，也不用为每种组合去制造一款新手机。

### 问题场景

- IO 流处理（基础流 + 缓冲 + 加密 + 压缩）
- GUI 组件（文本框 + 滚动条 + 边框 + 阴影）
- 日志系统（基础写入 + 时间戳 + 加密 + 分级过滤）
- Web 中间件（请求处理 + 认证 + 日志 + 限流 + 缓存）

### 实现

#### 场景：消息发送系统

基础功能是发送纯文本消息，但不同场景需要叠加不同处理：加密、压缩、添加签名……

```cpp
#include <iostream>
#include <memory>
#include <string>

// ========== 组件接口 ==========
class MessageSender {
public:
    virtual ~MessageSender() = default;
    virtual void send(const std::string& message) = 0;
    virtual std::string describe() const = 0;
};

// ========== 具体组件：基础发送器 ==========
class PlainSender : public MessageSender {
public:
    void send(const std::string& message) override {
        std::cout << "  Sending: " << message << "\n";
    }

    std::string describe() const override { return "PlainSender"; }
};
```

```cpp
// ========== 装饰器基类 ==========
// 关键：装饰器本身也是 MessageSender（is-a），同时持有另一个 MessageSender（has-a）
class MessageDecorator : public MessageSender {
protected:
    std::unique_ptr<MessageSender> wrapped_;

public:
    explicit MessageDecorator(std::unique_ptr<MessageSender> sender)
        : wrapped_(std::move(sender)) {}

    void send(const std::string& message) override {
        wrapped_->send(message);  // 默认：委托给被包装对象
    }

    std::string describe() const override {
        return wrapped_->describe();
    }
};
```

```cpp
// ========== 具体装饰器 ==========

// 装饰器 A：加密
class EncryptionDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        // 在发送前加密（模拟：每个字符 +1）
        std::string encrypted;
        for (char c : message) encrypted += static_cast<char>(c + 1);
        std::cout << "  [Encrypt] " << message << " -> " << encrypted << "\n";
        wrapped_->send(encrypted);  // 将加密后的消息传给下一层
    }

    std::string describe() const override {
        return "Encrypted(" + wrapped_->describe() + ")";
    }
};

// 装饰器 B：压缩
class CompressionDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        // 模拟压缩：只保留前 10 个字符 + "..."
        std::string compressed = message;
        if (compressed.size() > 10) {
            compressed = compressed.substr(0, 10) + "...";
        }
        std::cout << "  [Compress] " << message.size()
                  << " bytes -> " << compressed.size() << " bytes\n";
        wrapped_->send(compressed);
    }

    std::string describe() const override {
        return "Compressed(" + wrapped_->describe() + ")";
    }
};

// 装饰器 C：签名
class SignatureDecorator : public MessageDecorator {
private:
    std::string signer_;

public:
    SignatureDecorator(std::unique_ptr<MessageSender> sender,
                      const std::string& signer)
        : MessageDecorator(std::move(sender)), signer_(signer) {}

    void send(const std::string& message) override {
        std::string signed_msg = message + "\n  -- Signed by " + signer_;
        std::cout << "  [Sign] Added signature of " << signer_ << "\n";
        wrapped_->send(signed_msg);
    }

    std::string describe() const override {
        return "Signed(" + wrapped_->describe() + ")";
    }
};

// 装饰器 D：日志
class LoggingDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        std::cout << "  [Log] Message length: " << message.size() << " chars\n";
        wrapped_->send(message);
        std::cout << "  [Log] Message sent successfully\n";
    }

    std::string describe() const override {
        return "Logged(" + wrapped_->describe() + ")";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // === 场景 1：最简单——纯文本发送 ===
    std::cout << "--- Plain sender ---\n";
    auto plain = std::make_unique<PlainSender>();
    plain->send("Hello World");
    // Output:
    //   Sending: Hello World

    // === 场景 2：加密 + 发送 ===
    std::cout << "\n--- Encrypted sender ---\n";
    auto encrypted = std::make_unique<EncryptionDecorator>(
        std::make_unique<PlainSender>());
    encrypted->send("Secret");
    // Output:
    //   [Encrypt] Secret -> Tfdsfu
    //   Sending: Tfdsfu

    // === 场景 3：日志 + 加密 + 签名 + 发送（多层装饰） ===
    std::cout << "\n--- Full pipeline ---\n";
    auto fullPipeline = std::make_unique<LoggingDecorator>(
        std::make_unique<EncryptionDecorator>(
            std::make_unique<SignatureDecorator>(
                std::make_unique<PlainSender>(),
                "Alice")));

    std::cout << "Pipeline: " << fullPipeline->describe() << "\n";
    // Output: Pipeline: Logged(Encrypted(Signed(PlainSender)))

    fullPipeline->send("Important data");
    // Output:
    //   [Log] Message length: 14 chars
    //   [Encrypt] Important data -> Jnqpsubou!ebub
    //   [Sign] Added signature of Alice
    //   Sending: Jnqpsubou!ebub
    //     -- Signed by Alice
    //   [Log] Message sent successfully
}
```

---

### 装饰顺序很重要

装饰器的叠加顺序不同，行为也不同：

```cpp
// 顺序 A：先加密，再压缩
// 效果：加密后的密文被压缩（密文通常不可压缩，效率低）
auto pipelineA = std::make_unique<CompressionDecorator>(
    std::make_unique<EncryptionDecorator>(
        std::make_unique<PlainSender>()));

// 顺序 B：先压缩，再加密
// 效果：明文先压缩（压缩率高），再对压缩结果加密（更合理）
auto pipelineB = std::make_unique<EncryptionDecorator>(
    std::make_unique<CompressionDecorator>(
        std::make_unique<PlainSender>()));
```

> **经验法则**：最外层的装饰器最先执行。构造时从内到外包装，执行时从外到内层层调用。

### 装饰器 vs 继承

| 维度 | 继承 | 装饰器 |
|------|------|--------|
| **绑定时机** | 编译时静态确定 | 运行时动态组合 |
| **类数量** | N 种功能组合 → 2^N 个子类 | N 种功能 → N 个装饰器类 |
| **灵活性** | 运行时无法增减功能 | 可以随时包装/拆除 |
| **对象类型** | `is-a` 子类类型 | `is-a` 依然成立（透明性） |

```
// 继承的灾难——3 种功能的所有组合 = 8 个类
PlainMessage
EncryptedMessage
CompressedMessage
SignedMessage
EncryptedCompressedMessage
EncryptedSignedMessage
CompressedSignedMessage
EncryptedCompressedSignedMessage  // 再加一种功能？16 个类！

// 装饰器——3 种功能 = 3 个装饰器类，自由组合
```

---

### 错误用法

```cpp
// 错误 1：装饰器不委托给 wrapped_，而是自己重新创建对象
class BrokenDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        // 错误！没有调用 wrapped_->send()，而是自己 new 了一个新对象
        auto fresh = std::make_unique<PlainSender>();
        fresh->send("[decorated] " + message);
        // 之前的装饰链全部被跳过！
    }
};

// 错误 2：装饰器改变了接口——破坏透明性
class IntrusiveDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    // 错误！新增了基类没有的方法，客户端必须知道具体装饰器类型才能调用
    void sendUrgent(const std::string& message) {
        wrapped_->send("[URGENT] " + message);
    }
};
// 这样 std::unique_ptr<MessageSender> 指针就无法调用 sendUrgent()

// 错误 3：装饰器层数过深——调试噩梦
// 超过 3-4 层装饰就该考虑 Pipeline 模式或责任链模式了
auto nightmare = std::make_unique<D1>(
    std::make_unique<D2>(
        std::make_unique<D3>(
            std::make_unique<D4>(
                std::make_unique<D5>(
                    std::make_unique<D6>(
                        std::make_unique<Base>()))))));
// 出 bug 了？祝你好运找到是哪一层的问题
```

---

### 面试热点

```
Q: 装饰器模式和代理模式的区别？
A: 装饰器：目的是"添加功能"，客户端知道自己可以叠加装饰器。
   代理：目的是"控制访问"，客户端通常不知道自己用的是代理。
   结构几乎一样（都是包装对象），但意图完全不同。

Q: 装饰器的 is-a 关系有什么好处？
A: 装饰器继承了组件接口，所以在客户端看来，装饰后的对象和原始对象"长得一样"。
   客户端代码不需要修改——这就是"透明性"。std::unique_ptr<Component> 可以指向
   原始对象，也可以指向任意层装饰后的对象。

Q: C++ IO 流如何体现装饰器模式？
A: std::ifstream 是基础文件流，std::istream 是抽象接口。
   std::buffered_streambuf 给流添加缓冲能力，
   整个 iostream 体系就是装饰器模式的经典应用。
```

---

## 2.4 Facade（外观模式）

### 定义

> 为子系统中的一组接口提供一个统一的高层接口，使子系统更易于使用。

### 现实类比

就像**一键启动按钮**——启动一辆现代汽车，你只需按一个按钮。但在引擎盖下面，这个按钮触发了一连串操作：检查电池 → 启动油泵 → 点火 → 怠速控制 → 启动仪表盘 → 解锁方向盘。驾驶员不需要知道这些细节，一个按钮就搞定一切。

### 问题场景

- 复杂库/框架的封装（游戏引擎的初始化、图形渲染管线的设置）
- 多系统集成（订单 + 库存 + 支付 + 物流 → 统一下单接口）
- 遗留系统改造（旧系统内部混乱，用 Facade 给外部提供干净接口）
- 微服务网关（多个后端服务 → 统一 API Gateway）

### 实现

#### 场景：家庭影院系统

看一部电影需要操作多个设备，每个设备都有自己的接口：

```cpp
#include <iostream>
#include <memory>
#include <string>

// ========== 子系统组件（各自独立，各自复杂） ==========

class Amplifier {
public:
    void on()  { std::cout << "  Amplifier: ON\n"; }
    void off() { std::cout << "  Amplifier: OFF\n"; }
    void setVolume(int level) {
        std::cout << "  Amplifier: Volume set to " << level << "\n";
    }
    void setSurroundSound() {
        std::cout << "  Amplifier: Surround sound enabled\n";
    }
};

class BluRayPlayer {
public:
    void on()  { std::cout << "  BluRay: ON\n"; }
    void off() { std::cout << "  BluRay: OFF\n"; }
    void play(const std::string& movie) {
        std::cout << "  BluRay: Playing '" << movie << "'\n";
    }
    void stop() { std::cout << "  BluRay: Stopped\n"; }
    void eject() { std::cout << "  BluRay: Disc ejected\n"; }
};

class Projector {
public:
    void on()  { std::cout << "  Projector: ON\n"; }
    void off() { std::cout << "  Projector: OFF\n"; }
    void setWideScreen() {
        std::cout << "  Projector: Widescreen mode (16:9)\n";
    }
    void setInput(const std::string& source) {
        std::cout << "  Projector: Input set to " << source << "\n";
    }
};

class Screen {
public:
    void down() { std::cout << "  Screen: Lowered\n"; }
    void up()   { std::cout << "  Screen: Raised\n"; }
};

class Lights {
public:
    void dim(int level) {
        std::cout << "  Lights: Dimmed to " << level << "%\n";
    }
    void on() { std::cout << "  Lights: ON (100%)\n"; }
};
```

```cpp
// ========== 外观类 ==========
class HomeTheaterFacade {
private:
    Amplifier    amp_;
    BluRayPlayer player_;
    Projector    projector_;
    Screen       screen_;
    Lights       lights_;

public:
    // 高层接口：看电影
    void watchMovie(const std::string& movie) {
        std::cout << "=== Preparing to watch: " << movie << " ===\n";
        lights_.dim(10);
        screen_.down();
        projector_.on();
        projector_.setWideScreen();
        projector_.setInput("BluRay");
        amp_.on();
        amp_.setSurroundSound();
        amp_.setVolume(7);
        player_.on();
        player_.play(movie);
        std::cout << "=== Enjoy your movie! ===\n";
    }

    // 高层接口：结束观影
    void endMovie() {
        std::cout << "\n=== Shutting down movie theater ===\n";
        player_.stop();
        player_.eject();
        player_.off();
        amp_.off();
        projector_.off();
        screen_.up();
        lights_.on();
        std::cout << "=== Goodnight! ===\n";
    }

    // 高层接口：听音乐（复用部分子系统）
    void listenToMusic() {
        std::cout << "=== Music mode ===\n";
        lights_.dim(50);
        amp_.on();
        amp_.setVolume(5);
        std::cout << "=== Ready for music! ===\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    HomeTheaterFacade theater;

    // 客户端只需调用一个方法——不关心 5 个子系统的操作顺序
    theater.watchMovie("The Matrix");
    // Output:
    // === Preparing to watch: The Matrix ===
    //   Lights: Dimmed to 10%
    //   Screen: Lowered
    //   Projector: ON
    //   Projector: Widescreen mode (16:9)
    //   Projector: Input set to BluRay
    //   Amplifier: ON
    //   Amplifier: Surround sound enabled
    //   Amplifier: Volume set to 7
    //   BluRay: ON
    //   BluRay: Playing 'The Matrix'
    // === Enjoy your movie! ===

    theater.endMovie();
    // === Shutting down movie theater ===
    //   BluRay: Stopped
    //   BluRay: Disc ejected
    //   BluRay: OFF
    //   Amplifier: OFF
    //   Projector: OFF
    //   Screen: Raised
    //   Lights: ON (100%)
    // === Goodnight! ===
}
```

---

### Facade 的本质

Facade **不隐藏**子系统——它只是提供一条"快捷通道"：

```
没有 Facade：                   有 Facade：
客户端 → Amplifier              客户端 → Facade → Amplifier
客户端 → BluRay                            ↘ → BluRay
客户端 → Projector                          ↘ → Projector
客户端 → Screen                             ↘ → Screen
客户端 → Lights                             ↘ → Lights
（客户端需要知道 5 个组件）      （客户端只需要知道 Facade）
                                （但仍然可以直接访问子系统）
```

> **重要区别**：Facade 不阻止客户端直接访问子系统。如果客户端需要细粒度控制（比如单独调节音量），可以直接操作 Amplifier。

### 三种模式的区分

| 模式 | 目的 | 做了什么 | 客户端视角 |
|------|------|---------|-----------|
| **Adapter** | 让不兼容的接口协同 | 转换接口 A → 接口 B | "我调用的就是接口 B" |
| **Decorator** | 给对象添加新功能 | 包装对象，叠加行为 | "还是同一个接口，但功能更强" |
| **Facade** | 简化复杂系统的使用 | 组合多个子系统调用 | "一个方法搞定所有事" |

---

### 错误用法

```cpp
// 错误 1：Facade 变成"上帝类"——什么都往里塞
class GodFacade {
    AudioSystem audio_;
    VideoSystem video_;
    NetworkSystem network_;
    FileSystem file_;
    DatabaseSystem db_;
    CacheSystem cache_;
    // ...还有 20 个子系统
    // 一个 Facade 管太多子系统 → 它自身变成了复杂系统
    // 应该按职责拆分成多个 Facade
};

// 错误 2：Facade 只是简单转发，没有简化
class PointlessFacade {
    Amplifier amp_;
public:
    void ampOn()  { amp_.on(); }   // 1:1 转发
    void ampOff() { amp_.off(); }  // 客户端还不如直接用 Amplifier
    void setVol(int v) { amp_.setVolume(v); }
    // 没有任何简化，只是多了一层无意义的封装
};

// 错误 3：Facade 禁止客户端访问子系统（过度封装）
class JailFacade {
private:
    Amplifier amp_;      // 全部 private
    BluRayPlayer player_; // 客户端完全无法访问子系统
    // 需要单独调音量？不行！只能通过 Facade 的方法
    // 这违背了 Facade 的初衷——它是"便捷入口"，不是"监狱围墙"
};
```

---

### 面试热点

```
Q: Facade 和 Adapter 都是"封装"，区别在哪里？
A: Adapter 是包装"一个"已有对象，将其接口转换为另一个接口。
   Facade 是包装"一组"子系统，提供一个新的简化接口。
   Adapter 的动机是"兼容"，Facade 的动机是"简化"。

Q: 什么时候不应该使用 Facade？
A: 1) 子系统本身就很简单，不需要额外封装
   2) 客户端确实需要细粒度控制每个组件
   3) 一个 Facade 试图统一过多不相关的子系统

Q: Facade 通常是单例的吗？
A: 经常是，但不是必须的。如果整个应用只需要一个家庭影院入口，
   单例是合理的。但大型应用可能为不同模块提供不同的 Facade。
```

---

## 2.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 一句话记忆 |
|------|---------|---------|-----------|
| **Adapter** | 接口不兼容 | 包装被适配者，翻译接口调用 | "改接口" |
| **Decorator** | 继承扩展导致类爆炸 | 动态包装，层层叠加功能 | "加功能" |
| **Facade** | 子系统太复杂 | 提供统一高层入口 | "做简化" |

### 统一思想：组合优于继承

三种模式都通过**持有另一个对象的引用**来工作，而不是继承：

```
Adapter:    持有 Adaptee 对象  → 转发并转换调用
Decorator:  持有 Component 对象 → 转发并叠加行为
Facade:     持有多个子系统对象 → 编排调用顺序
```

这就是"组合优于继承"在结构设计中的三种具体表现：

- Adapter：**让不一样的东西能协作**
- Decorator：**让简单的东西变丰富**
- Facade：**让复杂的东西变简单**

### 本课核心收获

1. **Adapter** 解决"接口不匹配"——在不修改已有代码的前提下让它们协同工作
2. **Decorator** 解决"功能扩展"——用组合代替继承，避免类爆炸
3. **Facade** 解决"使用复杂性"——为复杂子系统提供简洁的操作入口

### 下一课预告

第 3 课将学习三个进阶结构型模式：**Composite**、**Proxy**、**Bridge**，处理更复杂的对象组合结构。

---

## 2.6 课后练习

1. **Adapter**：实现一个"日志系统适配器"：
   - 目标接口：`Logger`，有 `log(level, message)` 方法
   - 待适配类 A：`FileWriter`，接口是 `write(filename, content)`
   - 待适配类 B：`ConsoleOutput`，接口是 `print(color, text)`
   - 为两者各写一个适配器，使客户端代码可以通过统一的 `Logger` 接口同时输出到文件和控制台

2. **Decorator**：实现一个"咖啡订单系统"：
   - 基础组件：`Coffee`，有 `cost()` 和 `description()` 方法
   - 基础咖啡：`Espresso`（$2.0）、`HouseBlend`（$1.5）
   - 装饰器：`MilkDecorator`（+$0.5）、`SugarDecorator`（+$0.3）、`WhipDecorator`（+$0.7）
   - 要求：可以任意组合，如"加双份奶的浓缩咖啡"（Espresso + Milk + Milk = $3.0）

3. **Facade**：实现一个"编译器 Facade"：
   - 子系统：`Lexer`（词法分析）、`Parser`（语法分析）、`Optimizer`（优化）、`CodeGenerator`（代码生成）
   - 提供 `compile(sourceCode)` 高层接口，按正确顺序调用四个子系统
   - 提供 `checkSyntax(sourceCode)` 接口，只执行词法分析和语法分析
