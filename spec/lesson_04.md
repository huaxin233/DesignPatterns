# 第 4 课：行为型模式（面向对象通信）

> **前置知识**：第 1–3 课内容（创建型模式、结构型模式、多态、智能指针、组合）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Strategy、State、Template Method 三种行为型模式，理解对象间职责分配与算法封装

---

## 4.1 课程导入

### 从"结构"到"行为"

前三课我们学习了对象如何创建（创建型）和对象如何组合（结构型）。第 4 课开始进入**行为型模式**——解决的核心问题是：

> **对象之间如何分配职责？算法如何灵活切换？**

| 模式 | 解决的问题 | 核心思想 |
|------|---------|---------|
| **Strategy** | 算法可替换 | 把算法封装成对象，运行时切换 |
| **State** | 状态决定行为 | 不同状态下执行不同逻辑，消除 if-else |
| **Template Method** | 算法骨架固定，细节可变 | 父类定义流程，子类实现步骤 |

### 共同主题

三种模式都在解决**"如何让行为可变"**：
- Strategy：**整体替换**——换掉整个算法
- State：**状态驱动**——状态变了，行为自动变
- Template Method：**部分替换**——骨架不变，步骤可变

---

## 4.2 Strategy（策略模式）

### 定义

> 定义一系列算法，把它们各自封装成类，并使它们可以互相替换。策略模式使得算法可以独立于使用它的客户端变化。

### 现实类比

就像**出行方式**——从 A 到 B，你可以开车、坐地铁、骑自行车。目的地不变，但"怎么去"可以随时切换。导航 APP 不关心你用哪种方式，它只需要调用 `travel()` 接口。

### 问题场景

- 支付系统（微信支付 / 支付宝 / 银行卡）
- 排序算法（快排 / 归并排序 / 插入排序）
- 压缩工具（ZIP / RAR / GZIP）
- 折扣计算（满减 / 打折 / 会员价）

### 实现

#### 策略接口与具体策略

```cpp
#include <iostream>
#include <memory>
#include <string>

// 策略接口——定义算法的统一接口
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) const = 0;
    virtual std::string name() const = 0;
};

// 具体策略 A：冒泡排序
class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j + 1 < data.size() - i; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }

    std::string name() const override { return "BubbleSort"; }
};

// 具体策略 B：选择排序
class SelectionSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        for (size_t i = 0; i < data.size(); ++i) {
            size_t minIdx = i;
            for (size_t j = i + 1; j < data.size(); ++j) {
                if (data[j] < data[minIdx]) {
                    minIdx = j;
                }
            }
            std::swap(data[i], data[minIdx]);
        }
    }

    std::string name() const override { return "SelectionSort"; }
};

// 具体策略 C：快速排序
class QuickSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) const override {
        quickSort(data, 0, static_cast<int>(data.size()) - 1);
    }

    std::string name() const override { return "QuickSort"; }

private:
    void quickSort(std::vector<int>& data, int low, int high) const {
        if (low >= high) return;
        int pivot = data[high];
        int i = low;
        for (int j = low; j < high; ++j) {
            if (data[j] < pivot) {
                std::swap(data[i], data[j]);
                ++i;
            }
        }
        std::swap(data[i], data[high]);
        quickSort(data, low, i - 1);
        quickSort(data, i + 1, high);
    }
};
```

#### 上下文（使用策略的类）

```cpp
// 上下文——持有策略，可运行时替换
class Sorter {
private:
    std::unique_ptr<SortStrategy> strategy_;

public:
    explicit Sorter(std::unique_ptr<SortStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    // 运行时切换策略
    void setStrategy(std::unique_ptr<SortStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    void performSort(std::vector<int>& data) {
        std::cout << "Sorting with " << strategy_->name() << ":\n";
        strategy_->sort(data);
        for (int val : data) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    std::vector<int> data = {5, 2, 8, 1, 9, 3};

    // 使用冒泡排序
    Sorter sorter(std::unique_ptr<BubbleSort>(new BubbleSort()));
    sorter.performSort(data);
    // Output: Sorting with BubbleSort:
    //         1 2 3 5 8 9

    // 运行时切换为快速排序
    data = {5, 2, 8, 1, 9, 3};
    sorter.setStrategy(std::unique_ptr<QuickSort>(new QuickSort()));
    sorter.performSort(data);
    // Output: Sorting with QuickSort:
    //         1 2 3 5 8 9
}
```

---

### 策略模式 vs if-else

```cpp
// 没有策略模式——硬编码分支
void sort(std::vector<int>& data, const std::string& algorithm) {
    if (algorithm == "bubble") {
        // 冒泡排序逻辑...
    } else if (algorithm == "quick") {
        // 快速排序逻辑...
    } else if (algorithm == "selection") {
        // 选择排序逻辑...
    }
    // 每次新增算法都要改这个函数——违反开闭原则
}

// 有策略模式——新增算法只需添加新类
class InsertionSort : public SortStrategy {
    // 新策略，不修改任何已有代码
};
```

---

### 错误用法

```cpp
// 错误 1：策略类持有上下文引用，导致耦合
class BadStrategy : public SortStrategy {
    Sorter* sorter_;  // 策略不应该知道上下文的存在！
public:
    void sort(std::vector<int>& data) const override {
        sorter_->performSort(data);  // 循环调用！
    }
};

// 错误 2：上下文暴露了策略的具体类型
class BadSorter {
    BubbleSort strategy_;  // 绑定具体类型，无法替换！
public:
    void performSort(std::vector<int>& data) {
        strategy_.sort(data);
    }
};

// 错误 3：策略之间共享可变状态
class StatefulStrategy : public SortStrategy {
    mutable int counter_ = 0;  // 有状态！多线程下不安全
public:
    void sort(std::vector<int>& data) const override {
        ++counter_;  // 策略应该是无状态的
    }
};
```

---

### 面试热点

```
Q: 策略模式和工厂方法模式的区别？
A: 工厂方法：关注"创建什么对象"，产品创建后行为固定
   策略模式：关注"使用什么算法"，策略可以运行时切换

Q: 策略模式的缺点？
A: 1. 客户端必须了解所有策略的区别才能选择
   2. 策略类数量可能很多
   3. 简单场景下用 std::function / lambda 更轻量

Q: C++ 中策略模式的现代替代方案？
A: std::function + lambda 可以替代简单的策略模式：
      std::function<void(std::vector<int>&)> strategy = [](auto& d) { ... };
   适合策略逻辑简单、不需要多态继承的场景。
```

---

## 4.3 State（状态模式）

### 定义

> 允许一个对象在其内部状态改变时改变它的行为。对象看起来好像修改了它的类。

### 现实类比

就像**自动售货机**——投币前按按钮无反应，投币后按按钮出货，缺货时投币直接退款。同一个按钮，不同状态下行为完全不同。你不需要每次按按钮时用 if-else 判断状态，机器内部自动根据当前状态执行对应逻辑。

### 问题场景

- TCP 连接（Closed → Listening → Established → Closed）
- 文档审批（草稿 → 审核中 → 已发布）
- 游戏角色（站立 → 奔跑 → 跳跃 → 攻击）
- 播放器（停止 → 播放 → 暂停）

### 实现

#### 状态接口

```cpp
#include <iostream>
#include <memory>
#include <string>

// 前向声明
class VendingMachine;

// 状态抽象基类
class VendingState {
public:
    virtual ~VendingState() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void pressButton(VendingMachine& machine) = 0;
    virtual void dispense(VendingMachine& machine) = 0;
    virtual std::string name() const = 0;
};
```

#### 上下文（持有当前状态）

```cpp
// 上下文——自动售货机
class VendingMachine {
private:
    std::unique_ptr<VendingState> state_;
    int stock_;

public:
    explicit VendingMachine(int stock);  // 定义在所有状态类之后

    void setState(std::unique_ptr<VendingState> state) {
        std::cout << "State: " << state_->name() << " → " << state->name() << "\n";
        state_ = std::move(state);
    }

    void insertCoin() { state_->insertCoin(*this); }
    void pressButton() { state_->pressButton(*this); }
    void dispense() { state_->dispense(*this); }

    int getStock() const { return stock_; }
    void reduceStock() { --stock_; }
};
```

#### 具体状态

```cpp
// 具体状态：空闲（等待投币）
class IdleState : public VendingState {
public:
    void insertCoin(VendingMachine& machine) override;  // 定义在后面
    void pressButton(VendingMachine& machine) override {
        std::cout << "Please insert a coin first.\n";
    }
    void dispense(VendingMachine& machine) override {
        std::cout << "Please insert a coin first.\n";
    }
    std::string name() const override { return "Idle"; }
};

// 具体状态：已投币
class HasCoinState : public VendingState {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Coin already inserted.\n";
    }
    void pressButton(VendingMachine& machine) override;  // 定义在后面
    void dispense(VendingMachine& machine) override {
        std::cout << "Press the button first.\n";
    }
    std::string name() const override { return "HasCoin"; }
};

// 具体状态：出货中
class DispensingState : public VendingState {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Please wait, dispensing...\n";
    }
    void pressButton(VendingMachine& machine) override {
        std::cout << "Already dispensing...\n";
    }
    void dispense(VendingMachine& machine) override;  // 定义在后面
    std::string name() const override { return "Dispensing"; }
};

// 具体状态：缺货
class SoldOutState : public VendingState {
public:
    void insertCoin(VendingMachine& machine) override {
        std::cout << "Sorry, sold out. Returning coin.\n";
    }
    void pressButton(VendingMachine& machine) override {
        std::cout << "Sorry, sold out.\n";
    }
    void dispense(VendingMachine& machine) override {
        std::cout << "Nothing to dispense.\n";
    }
    std::string name() const override { return "SoldOut"; }
};
```

#### 状态转换逻辑

```cpp
// 延迟定义——因为需要创建其他状态对象
void IdleState::insertCoin(VendingMachine& machine) {
    std::cout << "Coin inserted.\n";
    machine.setState(std::unique_ptr<HasCoinState>(new HasCoinState()));
}

void HasCoinState::pressButton(VendingMachine& machine) {
    std::cout << "Button pressed. Dispensing...\n";
    machine.setState(std::unique_ptr<DispensingState>(new DispensingState()));
    machine.dispense();  // 触发出货
}

void DispensingState::dispense(VendingMachine& machine) {
    machine.reduceStock();
    std::cout << "Item dispensed! Stock remaining: " << machine.getStock() << "\n";
    if (machine.getStock() > 0) {
        machine.setState(std::unique_ptr<IdleState>(new IdleState()));
    } else {
        machine.setState(std::unique_ptr<SoldOutState>(new SoldOutState()));
    }
}

// 构造函数
VendingMachine::VendingMachine(int stock) : stock_(stock) {
    if (stock > 0) {
        state_ = std::unique_ptr<IdleState>(new IdleState());
    } else {
        state_ = std::unique_ptr<SoldOutState>(new SoldOutState());
    }
}
```

**客户端代码**：

```cpp
int main() {
    VendingMachine machine(2);  // 库存 2 个

    // 正常购买
    machine.insertCoin();    // Coin inserted.
    machine.pressButton();   // Button pressed. Dispensing...
    // State: Idle → HasCoin
    // State: HasCoin → Dispensing
    // Item dispensed! Stock remaining: 1
    // State: Dispensing → Idle

    // 未投币就按按钮
    machine.pressButton();   // Please insert a coin first.

    // 再次购买（最后一个）
    machine.insertCoin();
    machine.pressButton();
    // Item dispensed! Stock remaining: 0
    // State: Dispensing → SoldOut

    // 缺货
    machine.insertCoin();    // Sorry, sold out. Returning coin.
}
```

---

### 状态模式 vs if-else 状态机

```cpp
// 没有状态模式——所有逻辑堆在一起
class BadVendingMachine {
    enum State { IDLE, HAS_COIN, DISPENSING, SOLD_OUT };
    State state_ = IDLE;

    void insertCoin() {
        if (state_ == IDLE) {
            state_ = HAS_COIN;
        } else if (state_ == HAS_COIN) {
            std::cout << "Already inserted.\n";
        } else if (state_ == DISPENSING) {
            std::cout << "Wait...\n";
        } else if (state_ == SOLD_OUT) {
            std::cout << "Sold out.\n";
        }
        // 每种操作都有 4 个分支！
        // 新增状态时每个方法都要改——违反开闭原则
    }
};
```

---

### Strategy vs State

| 维度 | Strategy | State |
|------|----------|-------|
| **切换时机** | 客户端主动切换 | 状态内部自动转换 |
| **关注点** | "用哪个算法" | "当前是什么状态" |
| **策略/状态之间** | 互相不知道对方存在 | 状态之间知道彼此（负责转换） |
| **类比** | 选择出行方式 | 交通灯自动变化 |

---

### 错误用法

```cpp
// 错误 1：状态转换逻辑放在上下文中，而不是状态类中
class BadMachine {
    std::unique_ptr<VendingState> state_;
    void insertCoin() {
        // 上下文不应该管转换逻辑！
        if (dynamic_cast<IdleState*>(state_.get())) {
            state_ = std::unique_ptr<HasCoinState>(new HasCoinState());
        }
    }
};

// 错误 2：状态对象持有上下文的 shared_ptr（循环引用）
class LeakyState : public VendingState {
    std::shared_ptr<VendingMachine> machine_;  // 循环引用！
};

// 错误 3：状态类有自己的成员变量且不清理
class DirtyState : public VendingState {
    int accumulatedCoins_ = 0;  // 状态切换后这些数据丢失
    // 如果需要跨状态共享数据，应放在上下文中
};
```

---

### 面试热点

```
Q: 状态模式和策略模式的核心区别？
A: 策略模式：客户端选择策略，策略之间互不知晓
   状态模式：状态自己决定下一个状态，自动转换

Q: 状态模式的适用场景？
A: 当对象的行为取决于它的状态，且状态数量有限（通常 3-10 个），
   且状态转换逻辑复杂时，用状态模式替代 if-else/switch。

Q: 状态模式如何避免状态爆炸？
A: 1. 只对关键行为建模，不为每个细微变化创建状态
   2. 使用层次状态机（父状态 + 子状态）
   3. 共享数据放在上下文中，状态类保持无状态或轻状态
```

---

## 4.4 Template Method（模板方法模式）

### 定义

> 在一个方法中定义算法的骨架，将一些步骤延迟到子类中。模板方法使子类可以不改变算法结构，就能重定义该算法的某些特定步骤。

### 现实类比

就像**做菜的标准流程**——所有菜都遵循"准备食材 → 加工 → 调味 → 装盘"的固定流程。但每道菜的具体操作不同：红烧肉的"加工"是炖，沙拉的"加工"是拌。流程（模板）不变，每一步的细节由具体菜品决定。

### 问题场景

- 数据处理管道（读取 → 解析 → 转换 → 输出）
- 游戏 AI（初始化 → 回合循环 → 结束判定）
- 单元测试框架（setUp → runTest → tearDown）
- 文档生成（打开 → 写入内容 → 格式化 → 保存）

### 实现

#### 抽象类（定义模板方法）

```cpp
#include <iostream>
#include <string>
#include <fstream>

// 抽象类——定义数据挖掘的算法骨架
class DataMiner {
public:
    virtual ~DataMiner() = default;

    // 模板方法——定义算法骨架，final 禁止子类覆盖
    void mine(const std::string& path) final {
        std::string rawData = openFile(path);
        std::string data = extractData(rawData);
        std::string analysis = analyzeData(data);
        sendReport(analysis);
        std::cout << "---\n";
    }

protected:
    // 需要子类实现的抽象步骤
    virtual std::string openFile(const std::string& path) = 0;
    virtual std::string extractData(const std::string& rawData) = 0;

    // 可选步骤（钩子）——子类可以覆盖，但不强制
    virtual std::string analyzeData(const std::string& data) {
        std::cout << "Default analysis on: " << data << "\n";
        return "Analysis of [" + data + "]";
    }

    // 固定步骤——子类不需要覆盖
    void sendReport(const std::string& analysis) {
        std::cout << "Report sent: " << analysis << "\n";
    }
};
```

#### 具体子类

```cpp
// 具体子类 A：PDF 数据挖掘
class PDFMiner : public DataMiner {
protected:
    std::string openFile(const std::string& path) override {
        std::cout << "Opening PDF file: " << path << "\n";
        return "PDF raw content from " + path;
    }

    std::string extractData(const std::string& rawData) override {
        std::cout << "Extracting text from PDF...\n";
        return "PDF extracted data";
    }

    // 覆盖钩子方法——自定义分析逻辑
    std::string analyzeData(const std::string& data) override {
        std::cout << "PDF-specific analysis on: " << data << "\n";
        return "PDF Analysis of [" + data + "]";
    }
};

// 具体子类 B：CSV 数据挖掘
class CSVMiner : public DataMiner {
protected:
    std::string openFile(const std::string& path) override {
        std::cout << "Opening CSV file: " << path << "\n";
        return "CSV raw content from " + path;
    }

    std::string extractData(const std::string& rawData) override {
        std::cout << "Parsing CSV rows...\n";
        return "CSV extracted data";
    }

    // 使用默认的 analyzeData（不覆盖）
};

// 具体子类 C：JSON 数据挖掘
class JSONMiner : public DataMiner {
protected:
    std::string openFile(const std::string& path) override {
        std::cout << "Opening JSON file: " << path << "\n";
        return "JSON raw content from " + path;
    }

    std::string extractData(const std::string& rawData) override {
        std::cout << "Parsing JSON nodes...\n";
        return "JSON extracted data";
    }
};
```

**客户端代码**：

```cpp
int main() {
    PDFMiner pdfMiner;
    CSVMiner csvMiner;
    JSONMiner jsonMiner;

    std::cout << "=== PDF Mining ===\n";
    pdfMiner.mine("report.pdf");
    // Output:
    // Opening PDF file: report.pdf
    // Extracting text from PDF...
    // PDF-specific analysis on: PDF extracted data
    // Report sent: PDF Analysis of [PDF extracted data]

    std::cout << "=== CSV Mining ===\n";
    csvMiner.mine("data.csv");
    // Output:
    // Opening CSV file: data.csv
    // Parsing CSV rows...
    // Default analysis on: CSV extracted data
    // Report sent: Analysis of [CSV extracted data]

    std::cout << "=== JSON Mining ===\n";
    jsonMiner.mine("config.json");
    // Output:
    // Opening JSON file: config.json
    // Parsing JSON nodes...
    // Default analysis on: JSON extracted data
    // Report sent: Analysis of [JSON extracted data]
}
```

---

### 模板方法中的三种步骤

| 步骤类型 | 说明 | 实现方式 |
|---------|------|---------|
| **抽象步骤** | 必须由子类实现 | 纯虚函数 `= 0` |
| **可选步骤（钩子）** | 子类可以覆盖，有默认实现 | 虚函数 + 默认实现 |
| **固定步骤** | 子类不能覆盖 | 非虚函数或 `final` |

---

### Template Method vs Strategy

| 维度 | Template Method | Strategy |
|------|----------------|----------|
| **机制** | 继承（编译时绑定） | 组合（运行时绑定） |
| **粒度** | 替换算法的某些步骤 | 替换整个算法 |
| **灵活性** | 较低（继承层次固定） | 较高（随时切换） |
| **代码复用** | 父类复用公共逻辑 | 策略之间无共享 |
| **适用场景** | 算法骨架固定，细节可变 | 算法整体可替换 |

---

### 错误用法

```cpp
// 错误 1：模板方法不是 final，子类可以改变算法骨架
class BadMiner : public DataMiner {
public:
    // 不应覆盖模板方法！这会破坏算法结构
    void mine(const std::string& path) {
        // 完全跳过某些步骤...
        sendReport("hacked");
    }
};

// 错误 2：子类需要调用 super（C++ 不强制）
class FragileMiner : public DataMiner {
protected:
    std::string analyzeData(const std::string& data) override {
        // 忘记调用基类逻辑？
        // C++ 不像 Java 有 @Override 编译检查（C++11 有 override 关键字）
        return "only my analysis";
    }
};

// 错误 3：把所有步骤都做成抽象的——失去了模板方法的意义
class OverAbstractMiner {
public:
    virtual void mine(const std::string& path) = 0;
    // 没有骨架、没有固定步骤，这不是模板方法，只是普通接口
};
```

---

### 面试热点

```
Q: 模板方法模式的核心思想？
A: "好莱坞原则"——Don't call us, we'll call you.
   父类控制流程，子类提供细节。
   子类不主动调用父类，而是被父类调用。

Q: 模板方法和策略模式如何选择？
A: 如果算法骨架固定、只有部分步骤不同 → 模板方法
   如果整个算法可替换、需要运行时切换 → 策略模式

Q: C++ 中如何防止子类覆盖模板方法？
A: 使用 C++11 的 final 关键字：
      void mine(const std::string& path) final;
   子类如果尝试覆盖会编译错误。

Q: 钩子方法的作用？
A: 提供可选的扩展点——子类可以选择覆盖（自定义），
   也可以不覆盖（使用默认行为）。
   这比纯虚函数更灵活，避免强迫子类实现无意义的空方法。
```

---

## 4.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Strategy** | 算法可替换 | 封装算法为对象，运行时切换 | "替换" |
| **State** | 状态决定行为 | 每种状态封装为类，自动转换 | "状态机" |
| **Template Method** | 骨架固定，细节可变 | 父类定义流程，子类实现步骤 | "骨架" |

### 统一思想

三种模式都在解决**"如何让行为灵活可变，同时保持结构清晰"**：

- Strategy：**整体替换**——把"做什么"交给策略对象
- State：**自动切换**——把"什么状态做什么"分散到状态类
- Template Method：**部分替换**——把"怎么做这一步"交给子类

### 组合 vs 继承

| 机制 | 代表模式 | 优势 | 劣势 |
|------|---------|------|------|
| **组合** | Strategy、State | 运行时灵活切换 | 对象数量多 |
| **继承** | Template Method | 代码复用，结构清晰 | 编译时固定，不够灵活 |

### 下一课预告

第 5 课我们将学习 **Command** 和 **Iterator** 模式，理解如何将请求和遍历操作封装为对象，实现撤销/重做、宏命令等高级功能。

---

## 4.6 课后练习

1. **Strategy**：实现一个"文本压缩器"系统：
   - `CompressionStrategy` 是策略接口
   - 实现 `ZipCompression`、`GzipCompression`、`RarCompression`
   - `FileCompressor` 上下文可以运行时切换压缩策略
   - 每种策略输出不同的压缩信息（压缩比、速度等）

2. **State**：实现一个"音乐播放器"状态机：
   - 状态：`StoppedState`、`PlayingState`、`PausedState`
   - 操作：`play()`、`pause()`、`stop()`
   - 实现完整的状态转换逻辑（如：暂停状态下按 play 恢复播放）

3. **Template Method**：实现一个"游戏 AI"框架：
   - `GameAI` 基类定义模板方法 `turn()`：`collectResources()` → `buildStructures()` → `attack()`
   - `OrcsAI` 和 `MonstersAI` 实现不同的具体步骤
   - `buildStructures()` 是钩子方法（怪物不建造建筑）
