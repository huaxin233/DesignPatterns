# 第 5 课：命令与迭代

> **前置知识**：第 1–4 课内容（创建型模式、结构型模式、Strategy、State、Template Method）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Command 和 Iterator 两种行为型模式，理解请求封装与遍历抽象

---

## 5.1 课程导入

### 从"行为切换"到"行为封装"

第 4 课我们学习了如何让行为灵活可变（Strategy 替换算法、State 切换状态、Template Method 定制步骤）。第 5 课我们将学习如何把**行为本身封装成对象**：

| 模式 | 解决的问题 | 核心思想 |
|------|---------|---------|
| **Command** | 请求的封装与调度 | 把"做什么"封装成对象，支持撤销、排队、日志 |
| **Iterator** | 遍历的抽象与统一 | 把"怎么遍历"封装成对象，统一访问不同容器 |

### 共同主题

两种模式都在解决**"把操作从执行者身上剥离出来"**：
- Command：把**请求**从调用者剥离出来，变成可以传递、存储、撤销的对象
- Iterator：把**遍历逻辑**从容器剥离出来，变成独立的遍历对象

---

## 5.2 Command（命令模式）

### 定义

> 将一个请求封装为一个对象，从而使你可以用不同的请求对客户进行参数化；对请求排队或记录日志，以及支持可撤销的操作。

### 现实类比

就像**餐厅点单**——顾客不直接对厨师喊"做一份宫保鸡丁"，而是把需求写在订单上交给服务员。订单（Command）记录了"做什么菜"，服务员（Invoker）不关心菜怎么做，厨师（Receiver）按订单执行。订单可以排队、撤销、甚至打印存档。

### 问题场景

- GUI 按钮/菜单项（点击 → 执行操作，支持撤销）
- 文本编辑器（Ctrl+Z 撤销、Ctrl+Y 重做）
- 事务系统（一组操作要么全部成功、要么全部回滚）
- 任务队列（异步执行、延迟执行、优先级调度）
- 宏录制（录制一系列操作，一键回放）

### 实现

#### 命令接口与接收者

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stack>

// 接收者——文本编辑器（实际执行操作的对象）
class TextEditor {
private:
    std::string text_;

public:
    void insertText(size_t pos, const std::string& text) {
        if (pos > text_.size()) pos = text_.size();
        text_.insert(pos, text);
        std::cout << "Insert '" << text << "' at pos " << pos << "\n";
    }

    void deleteText(size_t pos, size_t length) {
        if (pos >= text_.size()) return;
        length = std::min(length, text_.size() - pos);
        std::cout << "Delete '" << text_.substr(pos, length)
                  << "' at pos " << pos << "\n";
        text_.erase(pos, length);
    }

    const std::string& getText() const { return text_; }

    void display() const {
        std::cout << "Text: \"" << text_ << "\"\n";
    }
};

// 命令抽象基类
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};
```

#### 具体命令

```cpp
// 具体命令：插入文本
class InsertCommand : public Command {
private:
    TextEditor& editor_;
    size_t pos_;
    std::string text_;

public:
    InsertCommand(TextEditor& editor, size_t pos, const std::string& text)
        : editor_(editor), pos_(pos), text_(text) {}

    void execute() override {
        editor_.insertText(pos_, text_);
    }

    void undo() override {
        editor_.deleteText(pos_, text_.size());
    }
};

// 具体命令：删除文本
class DeleteCommand : public Command {
private:
    TextEditor& editor_;
    size_t pos_;
    size_t length_;
    std::string deletedText_;  // 保存被删除的文本，用于撤销

public:
    DeleteCommand(TextEditor& editor, size_t pos, size_t length)
        : editor_(editor), pos_(pos), length_(length) {}

    void execute() override {
        // 保存即将被删除的文本
        const auto& text = editor_.getText();
        size_t actualLen = std::min(length_, text.size() - std::min(pos_, text.size()));
        if (pos_ < text.size()) {
            deletedText_ = text.substr(pos_, actualLen);
        }
        editor_.deleteText(pos_, length_);
    }

    void undo() override {
        editor_.insertText(pos_, deletedText_);
    }
};
```

#### 调用者（支持撤销/重做）

```cpp
// 调用者——命令历史管理器
class CommandManager {
private:
    std::stack<std::unique_ptr<Command>> undoStack_;
    std::stack<std::unique_ptr<Command>> redoStack_;

public:
    void executeCommand(std::unique_ptr<Command> cmd) {
        cmd->execute();
        undoStack_.push(std::move(cmd));
        // 执行新命令后清空重做栈
        while (!redoStack_.empty()) redoStack_.pop();
    }

    void undo() {
        if (undoStack_.empty()) {
            std::cout << "Nothing to undo.\n";
            return;
        }
        auto& cmd = undoStack_.top();
        cmd->undo();
        redoStack_.push(std::move(cmd));
        undoStack_.pop();
    }

    void redo() {
        if (redoStack_.empty()) {
            std::cout << "Nothing to redo.\n";
            return;
        }
        auto& cmd = redoStack_.top();
        cmd->execute();
        undoStack_.push(std::move(cmd));
        redoStack_.pop();
    }
};
```

**客户端代码**：

```cpp
int main() {
    TextEditor editor;
    CommandManager manager;

    // 插入文本
    manager.executeCommand(
        std::unique_ptr<InsertCommand>(new InsertCommand(editor, 0, "Hello")));
    editor.display();  // Text: "Hello"

    manager.executeCommand(
        std::unique_ptr<InsertCommand>(new InsertCommand(editor, 5, " World")));
    editor.display();  // Text: "Hello World"

    // 删除文本
    manager.executeCommand(
        std::unique_ptr<DeleteCommand>(new DeleteCommand(editor, 5, 6)));
    editor.display();  // Text: "Hello"

    // 撤销
    std::cout << "\n--- Undo ---\n";
    manager.undo();    // 撤销删除
    editor.display();  // Text: "Hello World"

    manager.undo();    // 撤销第二次插入
    editor.display();  // Text: "Hello"

    // 重做
    std::cout << "\n--- Redo ---\n";
    manager.redo();    // 重做第二次插入
    editor.display();  // Text: "Hello World"
}
```

---

### 宏命令（组合命令）

```cpp
// 宏命令——组合多个命令，一键执行/撤销
class MacroCommand : public Command {
private:
    std::vector<std::unique_ptr<Command>> commands_;

public:
    void addCommand(std::unique_ptr<Command> cmd) {
        commands_.push_back(std::move(cmd));
    }

    void execute() override {
        for (auto& cmd : commands_) {
            cmd->execute();
        }
    }

    void undo() override {
        // 撤销时反序执行
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
    }
};
```

**使用示例**：

```cpp
int main() {
    TextEditor editor;
    CommandManager manager;

    // 创建宏：一键输入 "Hello World!"
    auto macro = std::unique_ptr<MacroCommand>(new MacroCommand());
    macro->addCommand(std::unique_ptr<InsertCommand>(new InsertCommand(editor, 0, "Hello")));
    macro->addCommand(std::unique_ptr<InsertCommand>(new InsertCommand(editor, 5, " World")));
    macro->addCommand(std::unique_ptr<InsertCommand>(new InsertCommand(editor, 11, "!")));

    manager.executeCommand(std::move(macro));
    editor.display();  // Text: "Hello World!"

    // 一键撤销整个宏
    manager.undo();
    editor.display();  // Text: ""
}
```

---

### Command vs Strategy

| 维度 | Command | Strategy |
|------|---------|----------|
| **目的** | 封装请求，支持撤销/排队 | 封装算法，支持替换 |
| **生命周期** | 命令执行后通常保留（用于撤销） | 策略切换后旧策略可丢弃 |
| **关注点** | "做了什么"（可回溯） | "怎么做"（可替换） |
| **典型用法** | 撤销/重做、任务队列 | 排序算法、支付方式 |

---

### 错误用法

```cpp
// 错误 1：命令对象不保存撤销所需的状态
class BadDeleteCommand : public Command {
    TextEditor& editor_;
    size_t pos_;
    size_t length_;
    // 没有保存被删除的文本！undo 时无法恢复
public:
    void undo() override {
        // 无法恢复被删除的内容...
        editor_.insertText(pos_, "???");  // 数据丢失！
    }
};

// 错误 2：命令直接操作接收者内部状态，破坏封装
class InvasiveCommand : public Command {
    TextEditor& editor_;
public:
    void execute() override {
        // 直接修改 editor_ 的私有成员（通过 friend 或 hack）
        // 命令应该通过接收者的公共接口操作！
    }
};

// 错误 3：调用者知道具体命令类型
class BadManager {
    InsertCommand* lastInsert_;  // 绑定具体类型！
public:
    void execute(InsertCommand* cmd) {  // 应该接受 Command*
        cmd->execute();
        lastInsert_ = cmd;
    }
};
```

---

### 面试热点

```
Q: 命令模式的核心作用？
A: 将"请求"封装为对象，实现：
   1. 调用者与接收者解耦
   2. 请求排队和延迟执行
   3. 撤销/重做
   4. 宏命令（组合多个操作）

Q: 命令模式和观察者模式的区别？
A: 命令模式：一对一关系，调用者 → 命令 → 接收者
   观察者模式：一对多关系，Subject → 多个 Observer
   命令关注"执行什么操作"，观察者关注"通知谁"。

Q: 如何实现多级撤销？
A: 用两个栈：undoStack 和 redoStack。
   执行命令时 push 到 undoStack；
   撤销时从 undoStack pop，执行 undo()，push 到 redoStack；
   重做时从 redoStack pop，执行 execute()，push 回 undoStack。
   执行新命令时清空 redoStack。
```

---

## 5.3 Iterator（迭代器模式）

### 定义

> 提供一种方法顺序访问一个聚合对象中的各个元素，而又不暴露该对象的内部表示。

### 现实类比

就像**电视遥控器的频道切换**——你按"下一个"就看到下一个频道，不需要知道频道列表是怎么存储的（数组？链表？数据库？）。遥控器（Iterator）提供了统一的"下一个/上一个"操作，屏蔽了电视内部的频道管理细节。

### 问题场景

- 容器遍历（数组、链表、树、图用同一种方式遍历）
- 数据库结果集（逐行读取，不一次性加载全部）
- 文件系统（遍历目录树中的文件）
- 社交网络（遍历好友列表、消息流）

### 实现

#### 自定义容器与迭代器

```cpp
#include <iostream>
#include <memory>
#include <stdexcept>

// 前向声明
template <typename T>
class LinkedList;

// 节点
template <typename T>
struct Node {
    T data;
    Node* next = nullptr;

    explicit Node(const T& val) : data(val) {}
};

// 迭代器——封装遍历逻辑
template <typename T>
class LinkedListIterator {
private:
    Node<T>* current_;

public:
    explicit LinkedListIterator(Node<T>* start) : current_(start) {}

    bool hasNext() const {
        return current_ != nullptr;
    }

    T& next() {
        if (!current_) {
            throw std::out_of_range("Iterator exhausted");
        }
        T& data = current_->data;
        current_ = current_->next;
        return data;
    }

    T& current() {
        if (!current_) {
            throw std::out_of_range("No current element");
        }
        return current_->data;
    }
};

// 聚合对象——链表
template <typename T>
class LinkedList {
private:
    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    size_t size_ = 0;

public:
    ~LinkedList() {
        while (head_) {
            Node<T>* temp = head_;
            head_ = head_->next;
            delete temp;
        }
    }

    void append(const T& value) {
        auto* node = new Node<T>(value);
        if (!tail_) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            tail_ = node;
        }
        ++size_;
    }

    size_t size() const { return size_; }

    // 创建迭代器——不暴露内部 Node 结构
    LinkedListIterator<T> createIterator() const {
        return LinkedListIterator<T>(head_);
    }
};
```

**客户端代码**：

```cpp
int main() {
    LinkedList<int> list;
    list.append(10);
    list.append(20);
    list.append(30);
    list.append(40);

    // 通过迭代器遍历——不需要知道链表的内部结构
    auto it = list.createIterator();
    while (it.hasNext()) {
        std::cout << it.next() << " ";
    }
    std::cout << "\n";
    // Output: 10 20 30 40
}
```

---

### C++ STL 风格迭代器

上面的 `hasNext()/next()` 是 Java 风格。C++ 标准库使用**运算符重载**实现迭代器，使容器可以使用 range-based for 循环：

```cpp
#include <iostream>
#include <stdexcept>

template <typename T>
struct Node {
    T data;
    Node* next = nullptr;
    explicit Node(const T& val) : data(val) {}
};

template <typename T>
class LinkedList {
private:
    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    size_t size_ = 0;

public:
    ~LinkedList() {
        while (head_) {
            Node<T>* temp = head_;
            head_ = head_->next;
            delete temp;
        }
    }

    void append(const T& value) {
        auto* node = new Node<T>(value);
        if (!tail_) {
            head_ = tail_ = node;
        } else {
            tail_->next = node;
            tail_ = node;
        }
        ++size_;
    }

    // STL 风格迭代器——内部类
    class Iterator {
    private:
        Node<T>* current_;

    public:
        explicit Iterator(Node<T>* node) : current_(node) {}

        // 解引用
        T& operator*() { return current_->data; }

        // 前进
        Iterator& operator++() {
            current_ = current_->next;
            return *this;
        }

        // 比较
        bool operator!=(const Iterator& other) const {
            return current_ != other.current_;
        }
    };

    // begin/end 支持 range-based for
    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
};
```

**客户端代码**：

```cpp
int main() {
    LinkedList<std::string> list;
    list.append("Hello");
    list.append("Iterator");
    list.append("Pattern");

    // range-based for——和 std::vector 一样的语法！
    for (const auto& item : list) {
        std::cout << item << " ";
    }
    std::cout << "\n";
    // Output: Hello Iterator Pattern
}
```

---

### 内部迭代器 vs 外部迭代器

| 类型 | 控制权 | C++ 实现 | 特点 |
|------|--------|---------|------|
| **外部迭代器** | 客户端控制遍历（`++it`） | `begin()/end()` + 运算符重载 | 灵活，可随时中断 |
| **内部迭代器** | 容器控制遍历，客户端传回调 | `forEach(callback)` | 简洁，但不能中断 |

```cpp
// 内部迭代器示例
template <typename T>
class LinkedList {
    // ... 同上 ...
public:
    // 内部迭代器——容器控制遍历
    template <typename Func>
    void forEach(Func func) {
        Node<T>* current = head_;
        while (current) {
            func(current->data);
            current = current->next;
        }
    }
};

// 使用
list.forEach([](const std::string& item) {
    std::cout << item << "\n";
});
```

> **C++ 惯例**：优先使用外部迭代器（STL 风格），因为它更灵活，支持标准库算法（`std::find`、`std::sort` 等）。

---

### STL 迭代器分类

| 迭代器类型 | 能力 | 代表容器 |
|-----------|------|---------|
| **InputIterator** | 只读、单次前进 | `std::istream_iterator` |
| **OutputIterator** | 只写、单次前进 | `std::ostream_iterator` |
| **ForwardIterator** | 读写、多次前进 | `std::forward_list` |
| **BidirectionalIterator** | 前进 + 后退 | `std::list`、`std::set` |
| **RandomAccessIterator** | 随机访问 | `std::vector`、`std::deque` |

```
RandomAccess ⊃ Bidirectional ⊃ Forward ⊃ Input
                                        ⊃ Output
```

---

### 错误用法

```cpp
// 错误 1：迭代器失效——遍历时修改容器
void badTraversal(LinkedList<int>& list) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == 20) {
            list.append(99);  // 可能导致迭代器失效！
        }
    }
}

// 错误 2：迭代器暴露了容器内部结构
template <typename T>
class LeakyIterator {
public:
    Node<T>* getNode() { return current_; }  // 暴露内部 Node！
    // 客户端可以直接修改 Node 的 next 指针，破坏链表
};

// 错误 3：没有实现 end()，无法用 range-based for
template <typename T>
class IncompleteList {
public:
    Iterator begin() { return Iterator(head_); }
    // 缺少 end()！
    // for (auto& item : list) 编译错误
};
```

---

### 面试热点

```
Q: 迭代器模式的核心作用？
A: 将遍历逻辑从容器中剥离出来，提供统一的遍历接口。
   客户端不需要知道容器的内部结构（数组、链表、树），
   只需要通过迭代器的接口逐个访问元素。

Q: C++ STL 迭代器和 GoF 迭代器的区别？
A: GoF 迭代器：hasNext()/next() 方法，面向对象风格
   STL 迭代器：operator++/operator* 运算符重载，泛型编程风格
   STL 迭代器更轻量（通常是指针或指针包装器），
   且可以和标准库算法无缝配合。

Q: 迭代器失效是什么？如何避免？
A: 当容器结构被修改（插入/删除元素）时，
   已有的迭代器可能指向无效内存。
   避免方法：
   1. 遍历时不修改容器
   2. 使用返回新迭代器的修改方法（如 std::vector::erase 返回下一个有效迭代器）
   3. 使用不会失效的容器（如 std::list 的迭代器在其他位置插入/删除时不失效）

Q: 什么是惰性迭代器？
A: 元素在需要时才计算/加载，而不是一次性全部准备好。
   例如：读取大文件时逐行迭代，而不是一次读入内存。
   C++20 的 std::ranges::views 就是惰性求值的。
```

---

## 5.4 本课总结

### 两种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Command** | 请求封装与调度 | 把操作封装为对象，支持撤销/排队 | "封装请求" |
| **Iterator** | 遍历抽象与统一 | 把遍历逻辑封装为对象，统一接口 | "遍历抽象" |

### 统一思想

两种模式都在解决**"把操作从执行者身上独立出来"**：

- Command：把**请求操作**从调用者身上独立出来——调用者不需要知道具体做什么
- Iterator：把**遍历操作**从容器身上独立出来——客户端不需要知道容器结构

### 封装的对象

| 模式 | 封装了什么 | 带来的能力 |
|------|---------|---------|
| **Command** | 一次操作（做什么 + 撤销怎么做） | 撤销/重做、排队、宏命令、日志 |
| **Iterator** | 一次遍历（当前位置 + 前进方式） | 统一遍历接口、隐藏容器内部结构 |

### 下一课预告

第 6 课我们将学习 **Chain of Responsibility** 和 **Mediator** 模式，理解对象间调用的解耦机制——如何让请求在多个处理者之间传递，以及如何用中介者减少对象间的直接依赖。

---

## 5.5 课后练习

1. **Command**：实现一个"绘图程序"命令系统：
   - `Canvas` 是接收者，支持 `drawCircle(x, y, r)` 和 `drawRect(x, y, w, h)`
   - `DrawCircleCommand` 和 `DrawRectCommand` 是具体命令
   - `CommandManager` 支持执行、撤销、重做
   - 实现一个 `MacroCommand`，可以一键绘制"房子"（一个矩形 + 一个三角形屋顶）

2. **Iterator**：实现一个"二叉树"的多种遍历迭代器：
   - `BinaryTree<T>` 是聚合对象
   - `InOrderIterator`：中序遍历（左 → 根 → 右）
   - `PreOrderIterator`：前序遍历（根 → 左 → 右）
   - 两种迭代器都支持 STL 风格（`begin()/end()` + 运算符重载）
   - 客户端可以用 range-based for 遍历

3. **综合练习**：实现一个"数据库查询系统"：
   - 用 **Command** 封装 SQL 操作（`InsertCommand`、`DeleteCommand`、`UpdateCommand`）
   - 用 **Iterator** 遍历查询结果集（`ResultSetIterator`）
   - 支持事务回滚（一组 Command 的批量撤销）
