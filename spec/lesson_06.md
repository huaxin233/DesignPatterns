# 第 6 课：行为型模式进阶——状态管理与请求流转

> **前置知识**：第 1-5 课内容（创建型模式、结构型模式、Strategy、Command、Template Method）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 State、Chain of Responsibility、Memento 三种行为型模式，理解对象状态管理与请求处理链的设计思想

---

## 6.1 课程导入

### 从算法封装到状态管理

第 5 课我们学习了三种"封装行为"的模式：

- Strategy：封装**可替换的算法**
- Command：封装**请求为对象**
- Template Method：封装**算法骨架**

第 6 课我们将学习三种"管理状态与流转"的模式：

| 模式 | 解决的问题 | 核心思想 |
|------|---------|---------|
| **State** | 对象行为随状态改变 | 用类替代条件分支 |
| **Chain of Responsibility** | 请求需要多个处理者尝试处理 | 链式传递，职责分离 |
| **Memento** | 需要保存和恢复对象状态 | 快照封装，不破坏封装性 |

---

## 6.2 State（状态模式）

### 定义

> 允许对象在内部状态改变时改变它的行为，对象看起来好像修改了它的类。

### 现实类比

就像**自动售货机**——同一台机器在不同状态下对"投币"和"按按钮"的反应完全不同：
- **待机状态**：投币 → 进入已投币状态；按按钮 → 提示"请先投币"
- **已投币状态**：投币 → 退回多余硬币；按按钮 → 出货，进入出货状态
- **出货状态**：投币 → 等待；按按钮 → 无反应；出货完成 → 回到待机

机器没变，但行为随状态完全改变。

### 问题场景

- TCP 连接状态（LISTEN → SYN_SENT → ESTABLISHED → CLOSE_WAIT…）
- 游戏角色状态（站立 → 跑步 → 跳跃 → 攻击）
- 订单流程（待支付 → 已支付 → 已发货 → 已签收）
- 文档审批（草稿 → 审核中 → 已发布 → 已归档）

### 实现

#### 不用状态模式的糟糕写法

```cpp
// 反面教材：用 if-else / switch 管理状态
class VendingMachine {
    enum State { IDLE, HAS_COIN, DISPENSING };
    State state_ = IDLE;

public:
    void insertCoin() {
        if (state_ == IDLE) {
            state_ = HAS_COIN;
            std::cout << "Coin accepted\n";
        } else if (state_ == HAS_COIN) {
            std::cout << "Coin already inserted\n";
        } else if (state_ == DISPENSING) {
            std::cout << "Please wait\n";
        }
        // 每新增一个状态，所有方法都要改！
    }

    void pressButton() {
        // 又是一堆 if-else...
    }
};
```

**问题**：每新增一个状态，所有方法里的 if-else 都要修改——违反开闭原则。

---

#### 使用状态模式

```cpp
#include <iostream>
#include <memory>
#include <string>

// 前向声明
class VendingMachine;

// 状态抽象基类
class State {
public:
    virtual ~State() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void pressButton(VendingMachine& machine) = 0;
    virtual std::string getName() const = 0;
};

// 售货机——持有当前状态
class VendingMachine {
private:
    std::unique_ptr<State> state_;
    int itemCount_;

public:
    explicit VendingMachine(int items);  // 定义在所有 State 之后

    void setState(std::unique_ptr<State> state) {
        std::cout << "State: " << state_->getName()
                  << " -> " << state->getName() << "\n";
        state_ = std::move(state);
    }

    void insertCoin() { state_->insertCoin(*this); }
    void pressButton() { state_->pressButton(*this); }

    int getItemCount() const { return itemCount_; }
    void dispenseItem() { --itemCount_; }
};
```

#### 具体状态类

```cpp
// 前向声明具体状态
class IdleState;
class HasCoinState;
class DispensingState;
class SoldOutState;

// 待机状态
class IdleState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Coin accepted.\n";
        machine.setState(std::unique_ptr<HasCoinState>(new HasCoinState()));
    }

    void pressButton(VendingMachine& machine) override {
        std::cout << "Please insert a coin first.\n";
    }

    std::string getName() const override { return "Idle"; }
};

// 已投币状态
class HasCoinState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Coin already inserted, returning extra coin.\n";
    }

    void pressButton(VendingMachine& machine) override {
        if (machine.getItemCount() > 0) {
            std::cout << "Dispensing item...\n";
            machine.setState(std::unique_ptr<DispensingState>(new DispensingState()));
        } else {
            std::cout << "Sold out! Returning coin.\n";
            machine.setState(std::unique_ptr<SoldOutState>(new SoldOutState()));
        }
    }

    std::string getName() const override { return "HasCoin"; }
};

// 出货状态
class DispensingState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Please wait, dispensing...\n";
    }

    void pressButton(VendingMachine& machine) override {
        std::cout << "Already dispensing, please wait.\n";
    }

    std::string getName() const override { return "Dispensing"; }
};

// 售罄状态
class SoldOutState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Sold out. Returning coin.\n";
    }

    void pressButton(VendingMachine& machine) override {
        std::cout << "Sold out. No items available.\n";
    }

    std::string getName() const override { return "SoldOut"; }
};

// VendingMachine 构造函数（放在所有 State 定义之后）
VendingMachine::VendingMachine(int items)
    : state_(std::unique_ptr<IdleState>(new IdleState())), itemCount_(items) {}
```

**客户端代码**：

```cpp
int main() {
    VendingMachine machine(2);

    machine.pressButton();   // Please insert a coin first.
    machine.insertCoin();    // Coin accepted. State: Idle -> HasCoin
    machine.insertCoin();    // Coin already inserted, returning extra coin.
    machine.pressButton();   // Dispensing item... State: HasCoin -> Dispensing
}
```

---

### State vs Strategy

| 维度 | State | Strategy |
|------|-------|----------|
| **目的** | 状态驱动行为变化 | 客户端选择算法 |
| **切换方式** | 状态自己决定下一个状态 | 客户端从外部设置 |
| **状态类之间** | 互相知道（知道如何转换） | 互不关心 |
| **生命周期** | 频繁切换 | 通常设置一次 |

> **口诀**：Strategy 是"你选哪个算法"，State 是"我现在是什么状态"。

---

### 错误用法

```cpp
// 错误 1：状态类持有 VendingMachine 的 shared_ptr（循环引用）
class BadState : public State {
    std::shared_ptr<VendingMachine> machine_;  // 循环引用！内存泄漏
};

// 错误 2：在状态类外部直接 setState（破坏状态封装）
void clientCode(VendingMachine& m) {
    m.setState(std::unique_ptr<DispensingState>(new DispensingState()));  // 跳过投币！
    // 状态转换应该由状态类内部驱动，不是外部随意切换
}

// 错误 3：状态类中包含业务数据（状态应该是无状态的行为封装）
class StatefulState : public State {
    int coinCount_ = 0;   // 错误！业务数据应该在 Context 中
    std::string userId_;  // 状态类只封装行为，不存储数据
};
```

---

### 面试热点

```
Q: State 模式和 if-else / switch 的区别？
A:
   if-else：所有状态逻辑集中在一个类，新增状态要改所有方法（违反 OCP）
   State：每个状态独立一个类，新增状态只需加新类（符合 OCP）
   代价是类数量增多，但换来了可维护性和可扩展性

Q: 状态转换应该由谁驱动？
A:
   由状态类自己驱动（推荐）：状态类知道自己在什么条件下转到哪个状态
   由 Context 驱动：当转换逻辑简单或依赖外部信息时
   由外部驱动：不推荐，破坏封装性
```

---

## 6.3 Chain of Responsibility（责任链模式）

### 定义

> 使多个对象都有机会处理请求，从而避免请求的发送者和接收者之间的耦合关系。将这些对象连成一条链，沿着链传递请求，直到有一个对象处理它为止。

### 现实类比

就像**公司报销审批**——你提交一笔报销单：
- 金额 < 500：组长直接审批
- 金额 < 5000：经理审批
- 金额 < 50000：总监审批
- 金额 >= 50000：CEO 审批

你不需要知道谁来审批，只需提交申请，系统会沿着审批链自动找到合适的审批人。

### 问题场景

- 日志级别过滤（DEBUG → INFO → WARN → ERROR）
- Web 中间件链（认证 → 限流 → 日志 → 业务处理）
- 异常处理链（子类 catch → 父类 catch → 全局 handler）
- 事件冒泡（子组件 → 父组件 → 根组件）

### 实现

#### 处理者基类

```cpp
#include <iostream>
#include <memory>
#include <string>

// 请求对象
struct ExpenseRequest {
    std::string description;
    double amount;
    std::string requester;
};

// 处理者抽象基类
class Approver {
protected:
    std::unique_ptr<Approver> next_;  // 下一个处理者
    std::string name_;

public:
    explicit Approver(const std::string& name) : name_(name) {}
    virtual ~Approver() = default;

    // 设置下一个处理者，返回下一个处理者的引用（支持链式调用）
    Approver& setNext(std::unique_ptr<Approver> next) {
        next_ = std::move(next);
        return *next_;
    }

    // 处理请求——子类实现具体逻辑
    virtual void handle(const ExpenseRequest& request) = 0;

protected:
    // 传递给下一个处理者
    void passToNext(const ExpenseRequest& request) {
        if (next_) {
            next_->handle(request);
        } else {
            std::cout << "No one can approve: " << request.description
                      << " ($" << request.amount << ")\n";
        }
    }
};
```

#### 具体处理者

```cpp
// 组长：审批 < 500
class TeamLead : public Approver {
public:
    using Approver::Approver;

    void handle(const ExpenseRequest& request) override {
        if (request.amount < 500) {
            std::cout << name_ << " approved: " << request.description
                      << " ($" << request.amount << ")\n";
        } else {
            passToNext(request);
        }
    }
};

// 经理：审批 < 5000
class Manager : public Approver {
public:
    using Approver::Approver;

    void handle(const ExpenseRequest& request) override {
        if (request.amount < 5000) {
            std::cout << name_ << " approved: " << request.description
                      << " ($" << request.amount << ")\n";
        } else {
            passToNext(request);
        }
    }
};

// 总监：审批 < 50000
class Director : public Approver {
public:
    using Approver::Approver;

    void handle(const ExpenseRequest& request) override {
        if (request.amount < 50000) {
            std::cout << name_ << " approved: " << request.description
                      << " ($" << request.amount << ")\n";
        } else {
            passToNext(request);
        }
    }
};

// CEO：审批任意金额
class CEO : public Approver {
public:
    using Approver::Approver;

    void handle(const ExpenseRequest& request) override {
        std::cout << name_ << " approved: " << request.description
                  << " ($" << request.amount << ")\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 构建责任链：组长 → 经理 → 总监 → CEO
    auto chain = std::unique_ptr<TeamLead>(new TeamLead("TeamLead Zhang"));
    auto& manager = chain->setNext(std::unique_ptr<Manager>(new Manager("Manager Li")));
    auto& director = manager.setNext(std::unique_ptr<Director>(new Director("Director Wang")));
    director.setNext(std::unique_ptr<CEO>(new CEO("CEO Chen")));

    // 提交不同金额的报销
    chain->handle({"Office supplies",   200,   "Alice"});
    // Output: TeamLead Zhang approved: Office supplies ($200)

    chain->handle({"Team dinner",       3000,  "Bob"});
    // Output: Manager Li approved: Team dinner ($3000)

    chain->handle({"Conference travel",  15000, "Carol"});
    // Output: Director Wang approved: Conference travel ($15000)

    chain->handle({"New server cluster", 80000, "Dave"});
    // Output: CEO Chen approved: New server cluster ($80000)
}
```

---

### 变体：中间件链（每个节点都处理）

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// HTTP 请求（简化）
struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    bool authenticated = false;
    bool rateLimited = false;
};

// 中间件基类——每个节点处理后继续传递
class Middleware {
protected:
    std::unique_ptr<Middleware> next_;

public:
    virtual ~Middleware() = default;

    Middleware& setNext(std::unique_ptr<Middleware> next) {
        next_ = std::move(next);
        return *next_;
    }

    virtual bool handle(HttpRequest& request) {
        if (next_) {
            return next_->handle(request);
        }
        return true;  // 链末端，默认通过
    }
};

// 认证中间件
class AuthMiddleware : public Middleware {
public:
    bool handle(HttpRequest& request) override {
        auto it = request.headers.find("Authorization");
        if (it == request.headers.end() || it->second.empty()) {
            std::cout << "[Auth] Rejected: No authorization header\n";
            return false;
        }
        request.authenticated = true;
        std::cout << "[Auth] Passed\n";
        return Middleware::handle(request);  // 传递给下一个
    }
};

// 限流中间件
class RateLimitMiddleware : public Middleware {
    int requestCount_ = 0;
    int limit_;

public:
    explicit RateLimitMiddleware(int limit) : limit_(limit) {}

    bool handle(HttpRequest& request) override {
        if (++requestCount_ > limit_) {
            std::cout << "[RateLimit] Rejected: Too many requests\n";
            return false;
        }
        std::cout << "[RateLimit] Passed (" << requestCount_
                  << "/" << limit_ << ")\n";
        return Middleware::handle(request);
    }
};

// 日志中间件
class LogMiddleware : public Middleware {
public:
    bool handle(HttpRequest& request) override {
        std::cout << "[Log] " << request.method << " " << request.path << "\n";
        return Middleware::handle(request);
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 构建中间件链：日志 → 限流 → 认证
    auto pipeline = std::unique_ptr<LogMiddleware>(new LogMiddleware());
    auto& rateLimit = pipeline->setNext(
        std::unique_ptr<RateLimitMiddleware>(new RateLimitMiddleware(3)));
    rateLimit.setNext(std::unique_ptr<AuthMiddleware>(new AuthMiddleware()));

    HttpRequest req{"GET", "/api/data", {{"Authorization", "Bearer token123"}}};
    pipeline->handle(req);
    // Output:
    // [Log] GET /api/data
    // [RateLimit] Passed (1/3)
    // [Auth] Passed
}
```

---

### 纯责任链 vs 中间件链

| 类型 | 特点 | 场景 |
|------|------|------|
| **纯责任链** | 只有一个节点处理请求，其余跳过 | 审批流程、异常处理 |
| **中间件链** | 每个节点都处理，可中断也可继续传递 | Web 中间件、过滤器 |

---

### 错误用法

```cpp
// 错误 1：链中形成环路（无限循环）
class LoopHandler : public Approver {
public:
    using Approver::Approver;
    void handle(const ExpenseRequest& request) override {
        // 把自己设为下一个处理者
        next_ = std::unique_ptr<LoopHandler>(new LoopHandler("Loop"));  // 死循环！
        passToNext(request);
    }
};

// 错误 2：处理者直接依赖具体类型（失去灵活性）
class TightlyCoupled : public Approver {
    Manager* specificManager_;  // 绑死到具体类！应该只依赖 Approver 基类
public:
    using Approver::Approver;
    void handle(const ExpenseRequest& request) override {
        specificManager_->handle(request);
    }
};

// 错误 3：链末端没有兜底处理（请求丢失）
class NoFallback : public Approver {
public:
    using Approver::Approver;
    void handle(const ExpenseRequest& request) override {
        if (request.amount < 100) {
            std::cout << "Approved\n";
        }
        // 不满足条件且没有 next_，请求直接丢失，无任何反馈
    }
};
```

---

### 面试热点

```
Q: 责任链模式的优缺点？
A:
   优点：解耦发送者和接收者、动态组合处理链、符合 SRP 和 OCP
   缺点：请求可能没被处理、调试困难（不知道谁处理了）、链太长影响性能

Q: 责任链模式在哪些框架中有应用？
A:
   Java Servlet Filter 链
   Spring Security 过滤器链
   Netty ChannelPipeline
   Express.js 中间件
   C++ 异常处理机制本身就是责任链（catch 块链）

Q: 责任链和 Observer 的区别？
A:
   责任链：请求沿链传递，只有一个（或部分）处理者处理
   Observer：通知广播给所有观察者，每个都会收到
```

---

## 6.4 Memento（备忘录模式）

### 定义

> 在不破坏封装性的前提下，捕获一个对象的内部状态，并在该对象之外保存这个状态，以便以后当需要时能将该对象恢复到原先保存的状态。

### 现实类比

就像**游戏存档**——你在打 Boss 前存一个档。如果打输了，可以读档回到存档时的状态（血量、装备、位置全部恢复）。存档文件不暴露游戏内部数据结构，你只能"存"和"读"，不能手动修改存档里的血量（否则就是作弊）。

### 问题场景

- 文本编辑器撤销（Ctrl+Z）
- 游戏存档/读档
- 数据库事务回滚
- 浏览器后退按钮
- 绘图软件的历史记录

### 实现

#### 三个角色

```
Originator（原发器）  ─── 创建 ───>  Memento（备忘录）
                                         │
Caretaker（管理者）   <── 保管 ────────┘
```

- **Originator**：需要保存/恢复状态的对象
- **Memento**：状态快照，对外不可修改
- **Caretaker**：管理备忘录的存取，不关心内容

#### 文本编辑器实现

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Memento——不可变的状态快照
class EditorMemento {
private:
    std::string content_;
    int cursorPosition_;

    // 只有 Editor 可以访问内部状态
    friend class Editor;

    EditorMemento(const std::string& content, int cursorPos)
        : content_(content), cursorPosition_(cursorPos) {}

public:
    // 对外只暴露描述信息，不暴露内部状态
    std::string getDescription() const {
        return "Snapshot [" + std::to_string(content_.size()) + " chars]";
    }
};

// Originator——文本编辑器
class Editor {
private:
    std::string content_;
    int cursorPosition_ = 0;

public:
    void type(const std::string& text) {
        content_.insert(cursorPosition_, text);
        cursorPosition_ += text.size();
    }

    void moveCursor(int position) {
        cursorPosition_ = std::max(0, std::min(position,
            static_cast<int>(content_.size())));
    }

    void deleteBack(int count) {
        int start = std::max(0, cursorPosition_ - count);
        content_.erase(start, cursorPosition_ - start);
        cursorPosition_ = start;
    }

    // 创建备忘录（存档）
    std::unique_ptr<EditorMemento> save() const {
        return std::unique_ptr<EditorMemento>(
            new EditorMemento(content_, cursorPosition_));
    }

    // 从备忘录恢复（读档）
    void restore(const EditorMemento& memento) {
        content_ = memento.content_;
        cursorPosition_ = memento.cursorPosition_;
    }

    void display() const {
        std::cout << "Content: \"" << content_
                  << "\" | Cursor: " << cursorPosition_ << "\n";
    }
};
```

#### Caretaker——撤销管理器

```cpp
// Caretaker——管理备忘录栈
class UndoManager {
private:
    std::vector<std::unique_ptr<EditorMemento>> history_;
    int maxHistory_;

public:
    explicit UndoManager(int maxHistory = 50) : maxHistory_(maxHistory) {}

    void save(std::unique_ptr<EditorMemento> memento) {
        if (static_cast<int>(history_.size()) >= maxHistory_) {
            history_.erase(history_.begin());  // 超出上限，丢弃最早的
        }
        history_.push_back(std::move(memento));
    }

    // 弹出最近一次备忘录
    std::unique_ptr<EditorMemento> undo() {
        if (history_.empty()) {
            std::cout << "Nothing to undo.\n";
            return nullptr;
        }
        auto memento = std::move(history_.back());
        history_.pop_back();
        return memento;
    }

    bool canUndo() const { return !history_.empty(); }

    void showHistory() const {
        std::cout << "History (" << history_.size() << " snapshots):\n";
        for (size_t i = 0; i < history_.size(); ++i) {
            std::cout << "  [" << i << "] " << history_[i]->getDescription() << "\n";
        }
    }
};
```

**客户端代码**：

```cpp
int main() {
    Editor editor;
    UndoManager undoMgr;

    // 输入文本并保存快照
    undoMgr.save(editor.save());
    editor.type("Hello");
    editor.display();  // Content: "Hello" | Cursor: 5

    undoMgr.save(editor.save());
    editor.type(" World");
    editor.display();  // Content: "Hello World" | Cursor: 11

    undoMgr.save(editor.save());
    editor.type("!!!");
    editor.display();  // Content: "Hello World!!!" | Cursor: 14

    // 撤销操作
    undoMgr.showHistory();
    // History (3 snapshots):
    //   [0] Snapshot [0 chars]
    //   [1] Snapshot [5 chars]
    //   [2] Snapshot [11 chars]

    if (auto m = undoMgr.undo()) {
        editor.restore(*m);
        editor.display();  // Content: "Hello World" | Cursor: 11
    }

    if (auto m = undoMgr.undo()) {
        editor.restore(*m);
        editor.display();  // Content: "Hello" | Cursor: 5
    }
}
```

---

### Memento + Command = 完整撤销系统

在实际项目中，Memento 常与 Command 模式配合：

```
用户操作
   │
   ▼
Command.execute()  ──→  保存 Memento 到 UndoManager
   │
   ▼
Command.undo()     ──→  从 UndoManager 取出 Memento 恢复
```

- **Command** 负责"做什么"和"怎么撤销"
- **Memento** 负责"保存什么"和"恢复什么"

---

### 宽接口 vs 窄接口

| 接口类型 | 可见者 | 暴露内容 | C++ 实现方式 |
|---------|--------|---------|-------------|
| **宽接口** | Originator | 全部内部状态 | `friend class` |
| **窄接口** | Caretaker | 仅描述信息 | public 方法 |

> **关键**：Caretaker 不能查看或修改 Memento 内部状态，只能存取。这保证了封装性。

---

### 错误用法

```cpp
// 错误 1：Memento 暴露了内部状态（破坏封装性）
class LeakyMemento {
public:
    std::string content;     // public！任何人都能修改
    int cursorPosition;      // Caretaker 可以篡改状态
};

// 错误 2：每次保存整个对象的深拷贝（内存爆炸）
class HeavyOriginator {
    std::vector<char> hugeBuffer_;  // 100MB 的数据
public:
    auto save() {
        // 每次撤销都拷贝 100MB！应该用增量快照
        return std::unique_ptr<HeavyMemento>(new HeavyMemento(hugeBuffer_));
    }
};

// 错误 3：Caretaker 保存了 Originator 的引用而非快照
class BadCaretaker {
    Editor* editorRef_;  // 错误！保存的是引用，不是快照
public:
    void save(Editor* editor) {
        editorRef_ = editor;  // editor 状态变了，"快照"也跟着变
    }
};
```

---

### 面试热点

```
Q: Memento 模式如何保证封装性？
A: 通过"宽接口"和"窄接口"分离：
   Originator 通过 friend 访问 Memento 全部状态（宽接口）
   Caretaker 只能通过 public 方法存取（窄接口）
   外部无法查看或修改快照内容

Q: 大对象的 Memento 如何优化？
A:
   1. 增量快照：只保存变化的部分（diff）
   2. 写时复制（Copy-on-Write）：共享未修改的数据
   3. 限制历史长度：只保留最近 N 个快照
   4. 序列化到磁盘：不常用的快照持久化

Q: Memento 和 Command 的关系？
A: Command 封装"操作"，Memento 封装"状态"。
   结合使用：Command.execute() 前保存 Memento，
   Command.undo() 时从 Memento 恢复。
```

---

## 6.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **State** | 行为随状态改变 | 每个状态一个类，自动切换 | "状态驱动" |
| **Chain of Responsibility** | 请求的处理者不确定 | 链式传递，逐个尝试 | "链式传递" |
| **Memento** | 需要保存/恢复状态 | 快照封装，不破坏封装性 | "快照回滚" |

### 统一思想

三种模式都在解决**"对象状态如何被管理和流转"**：

- State：**状态决定行为**——用类封装每个状态的行为，状态转换自动驱动
- Chain of Responsibility：**请求寻找处理者**——请求沿链流转，找到合适的处理者
- Memento：**状态的时间旅行**——保存历史状态快照，支持撤销和恢复

### 与第 5 课的联系

```
第 5 课（封装行为）          第 6 课（管理状态）
Strategy  ←───对比───→  State      （都是用类替代条件分支）
Command   ←───配合───→  Memento    （Command + Memento = 撤销系统）
Template  ←───对比───→  Chain      （都定义了处理流程）
```

### 下一课预告

第 7 课我们将学习三个高级行为型模式：**Iterator**、**Mediator**、**Visitor**，理解如何遍历复杂结构、协调多对象交互、以及在不修改类的前提下添加新操作。

---

## 6.6 课后练习

1. **State**：实现一个"音乐播放器"状态机：
   - 状态：`StoppedState`、`PlayingState`、`PausedState`
   - 操作：`play()`、`pause()`、`stop()`
   - 在 Playing 状态下调用 `play()` 应该无反应，调用 `pause()` 进入 Paused，调用 `stop()` 进入 Stopped

2. **Chain of Responsibility**：实现一个"技术支持系统"：
   - Level 1：处理密码重置、账号解锁（简单问题）
   - Level 2：处理软件 bug、系统错误（中等问题）
   - Level 3：处理安全事件、数据恢复（复杂问题）
   - 每个级别无法处理的问题自动升级到下一级

3. **Memento**：实现一个"绘图板撤销系统"：
   - `Canvas` 持有一组 `Shape`（圆、矩形）
   - 支持 `addShape()`、`removeShape()`
   - 支持 `undo()` 回到上一步状态
   - 支持 `redo()` 重做被撤销的操作（提示：用两个栈）

---
