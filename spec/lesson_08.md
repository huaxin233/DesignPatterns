# 第 8 课：存储与解释

> **前置知识**：第 1-7 课内容（行为型模式基础、组合结构、Command 的 undo/redo）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Memento、Interpreter、Visitor 三种高级行为型模式，理解状态存储、解释执行与操作扩展

---

## 8.1 课程导入

### 从对象通信到高级行为

前几课我们学习了对象间如何分配职责、传递请求、封装命令。这些都是关于**对象之间怎么交互**的问题。

第 8 课我们进入行为型模式中更"深层"的三个主题：

| 主题 | 解决的问题 | 代表模式 |
|------|---------|---------|
| **状态快照** | 如何保存和恢复对象状态，且不破坏封装 | Memento |
| **语言解释** | 如何定义一种语法并解释执行 | Interpreter |
| **操作扩展** | 如何在不修改数据结构的前提下添加新操作 | Visitor |

这三个模式看似无关，但都在解决一个共同的深层问题：**如何在不破坏封装性的前提下，从外部访问或操作对象的内部结构**。

---

## 8.2 Memento（备忘录模式）

### 定义

> 在不破坏封装性的前提下，捕获一个对象的内部状态，并在该对象之外保存这个状态，以便以后将该对象恢复到原先保存的状态。

### 现实类比

就像**游戏存档**——你在打 Boss 之前存一个档，打输了就读档回到存档点。游戏不需要暴露所有内部数据（血量、装备、位置），只是把"当前状态"打包成一个存档文件。你不能直接修改存档文件的内容，只能整体加载。

### 问题场景

- 文本编辑器的撤销/重做（Ctrl+Z / Ctrl+Y）
- 游戏存档与读档
- 数据库事务的回滚
- 绘图软件的历史记录

### 三个核心角色

| 角色 | 职责 | 类比 |
|------|------|------|
| **Originator**（原发器） | 创建和恢复备忘录 | 游戏本身 |
| **Memento**（备忘录） | 存储原发器的内部状态 | 存档文件 |
| **Caretaker**（管理者） | 保管备忘录，但不能修改 | 存档管理器 |

### 实现

#### 基础版：文本编辑器撤销

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Memento——状态快照（不可修改）
class EditorMemento {
private:
    std::string content_;
    int cursorPos_;

    // 只允许 Originator 访问内部状态
    friend class TextEditor;

    EditorMemento(const std::string& content, int cursorPos)
        : content_(content), cursorPos_(cursorPos) {}

    const std::string& getContent() const { return content_; }
    int getCursorPos() const { return cursorPos_; }

public:
    // 对外只暴露描述信息，不暴露具体状态
    std::string getDescription() const {
        return "Snapshot [" + std::to_string(content_.size()) + " chars]";
    }
};
```

#### Originator：文本编辑器

```cpp
// Originator——拥有需要保存的状态
class TextEditor {
private:
    std::string content_;
    int cursorPos_ = 0;

public:
    void type(const std::string& text) {
        content_.insert(cursorPos_, text);
        cursorPos_ += text.size();
        std::cout << "Typed: \"" << text << "\" → \""
                  << content_ << "\" (cursor at " << cursorPos_ << ")\n";
    }

    void moveCursor(int pos) {
        cursorPos_ = std::max(0, std::min(pos, static_cast<int>(content_.size())));
    }

    void deleteBack(int count) {
        int start = std::max(0, cursorPos_ - count);
        content_.erase(start, cursorPos_ - start);
        cursorPos_ = start;
        std::cout << "Deleted " << count << " chars → \""
                  << content_ << "\"\n";
    }

    // 创建备忘录
    std::unique_ptr<EditorMemento> save() const {
        std::cout << "  [Save] \"" << content_ << "\"\n";
        return std::unique_ptr<EditorMemento>(
            new EditorMemento(content_, cursorPos_));
    }

    // 从备忘录恢复
    void restore(const EditorMemento& memento) {
        content_ = memento.getContent();
        cursorPos_ = memento.getCursorPos();
        std::cout << "  [Restore] \"" << content_
                  << "\" (cursor at " << cursorPos_ << ")\n";
    }

    const std::string& getContent() const { return content_; }
};
```

#### Caretaker：历史管理器

```cpp
// Caretaker——管理备忘录，不窥探内容
class UndoManager {
private:
    std::vector<std::unique_ptr<EditorMemento>> undoStack_;
    std::vector<std::unique_ptr<EditorMemento>> redoStack_;

public:
    void save(std::unique_ptr<EditorMemento> memento) {
        undoStack_.push_back(std::move(memento));
        redoStack_.clear();  // 新操作清除 redo 栈
    }

    std::unique_ptr<EditorMemento> undo() {
        if (undoStack_.empty()) {
            std::cout << "  [Undo] Nothing to undo\n";
            return nullptr;
        }
        auto memento = std::move(undoStack_.back());
        undoStack_.pop_back();
        return memento;
    }

    void saveForRedo(std::unique_ptr<EditorMemento> memento) {
        redoStack_.push_back(std::move(memento));
    }

    std::unique_ptr<EditorMemento> redo() {
        if (redoStack_.empty()) {
            std::cout << "  [Redo] Nothing to redo\n";
            return nullptr;
        }
        auto memento = std::move(redoStack_.back());
        redoStack_.pop_back();
        return memento;
    }

    size_t undoCount() const { return undoStack_.size(); }
    size_t redoCount() const { return redoStack_.size(); }
};
```

**客户端代码**：

```cpp
int main() {
    TextEditor editor;
    UndoManager history;

    // 输入并保存状态
    editor.type("Hello");
    history.save(editor.save());

    editor.type(" World");
    history.save(editor.save());

    editor.type("!!!");
    history.save(editor.save());

    std::cout << "\nContent: \"" << editor.getContent() << "\"\n\n";
    // Content: "Hello World!!!"

    // Undo：先保存当前状态到 redo 栈，再恢复上一个状态
    std::cout << "--- Undo ---\n";
    history.saveForRedo(editor.save());
    auto memento = history.undo();
    if (memento) editor.restore(*memento);
    // Restore: "Hello World"

    std::cout << "\n--- Undo ---\n";
    history.saveForRedo(editor.save());
    memento = history.undo();
    if (memento) editor.restore(*memento);
    // Restore: "Hello"

    std::cout << "\n--- Redo ---\n";
    history.save(editor.save());
    memento = history.redo();
    if (memento) editor.restore(*memento);
    // Restore: "Hello World"
}
```

---

### Memento 与 Command 的关系

第 5 课学的 Command 模式也支持 undo，两者有什么区别？

| 维度 | Memento | Command |
|------|---------|---------|
| **撤销方式** | 存完整状态快照，恢复即可 | 记录"逆操作"，撤销时执行逆操作 |
| **内存开销** | 大（每次存完整状态） | 小（只存操作信息） |
| **实现复杂度** | 简单（save/restore） | 复杂（每个命令都要实现 undo） |
| **适用场景** | 状态小、操作复杂 | 状态大、操作简单可逆 |

> **实际项目中常常两者结合**：Command 负责封装操作和简单逆操作，Memento 在关键节点保存完整快照（如"保存"按钮）。

---

### 错误用法

```cpp
// 错误 1：Memento 暴露了 setter，破坏了"不可修改"语义
class BrokenMemento {
public:
    std::string state;  // 公开！任何人都能改
    void setState(const std::string& s) { state = s; }  // 危险！
};

// 错误 2：Caretaker 偷看并修改 Memento 的内容
class NosyCaretaker {
    std::vector<std::unique_ptr<EditorMemento>> history_;
public:
    void tamper() {
        // 试图直接访问 Memento 的私有状态——编译错误（正确设计下）
        // history_[0]->content_ = "hacked";
        // 但如果 Memento 设计不当，这里就能篡改历史
    }
};

// 错误 3：不限制备忘录数量，导致内存爆炸
class UnboundedHistory {
    std::vector<std::unique_ptr<EditorMemento>> history_;
public:
    void save(std::unique_ptr<EditorMemento> m) {
        history_.push_back(std::move(m));
        // 永远不清理！对象越来越多，内存越来越大
        // 正确做法：限制最大数量或使用 LRU 策略
    }
};
```

---

### 面试热点

```
Q: Memento 模式如何保持封装性？
A: Memento 的内部状态只有 Originator 能访问（通过 friend 或嵌套类），
   Caretaker 只负责保管，无法查看或修改状态内容。

Q: Memento 模式的缺点是什么？
A: 如果对象状态很大（如大型文档、图片），频繁创建备忘录会消耗大量内存。
   解决方案：增量存储（只存差异）、限制历史深度、压缩存储。

Q: Memento 和序列化有什么区别？
A: 序列化是通用的状态持久化技术，会暴露对象的完整结构。
   Memento 是设计模式，强调封装性——外部不知道状态的具体格式。
```

---

## 8.3 Interpreter（解释器模式）

### 定义

> 给定一个语言，定义它的文法的一种表示，并定义一个解释器，该解释器使用该表示来解释语言中的句子。

### 现实类比

就像**数学老师批改计算题**——学生写下 `(3 + 5) × 2`，老师按运算法则拆解：先算括号里的加法，再算乘法。每种运算符就是一条"文法规则"，老师就是"解释器"。

### 问题场景

- 数学表达式求值（计算器）
- 正则表达式引擎
- SQL 解析器
- 配置文件解析（如 JSON、YAML）
- 游戏中的脚本引擎
- 规则引擎（权限表达式、过滤条件）

### 核心思想：语法树

Interpreter 的核心是把"语言"表示为一棵**抽象语法树（AST）**：

```
    表达式: (x + 3) * y

          Multiply
          /     \
       Add       Var(y)
      /   \
   Var(x)  Num(3)
```

- **终结符表达式**（Terminal）：叶节点，如数字、变量
- **非终结符表达式**（Non-terminal）：内部节点，如加法、乘法

### 实现

#### 规则引擎：权限表达式求值

我们实现一个简单的布尔表达式解释器，用于权限判断：

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Context——存储变量绑定
class PermissionContext {
private:
    std::unordered_set<std::string> roles_;
    std::unordered_map<std::string, int> attributes_;

public:
    void addRole(const std::string& role) { roles_.insert(role); }
    bool hasRole(const std::string& role) const {
        return roles_.count(role) > 0;
    }

    void setAttribute(const std::string& key, int value) {
        attributes_[key] = value;
    }
    int getAttribute(const std::string& key) const {
        auto it = attributes_.find(key);
        return it != attributes_.end() ? it->second : 0;
    }
};
```

#### 抽象表达式

```cpp
// 抽象表达式——所有节点的统一接口
class BoolExpression {
public:
    virtual ~BoolExpression() = default;
    virtual bool interpret(const PermissionContext& ctx) const = 0;
    virtual std::string toString() const = 0;
};
```

#### 终结符表达式

```cpp
// 终结符：角色检查
class RoleExpression : public BoolExpression {
private:
    std::string role_;

public:
    explicit RoleExpression(const std::string& role) : role_(role) {}

    bool interpret(const PermissionContext& ctx) const override {
        return ctx.hasRole(role_);
    }

    std::string toString() const override {
        return "hasRole(" + role_ + ")";
    }
};

// 终结符：属性比较
class AttributeExpression : public BoolExpression {
private:
    std::string key_;
    int threshold_;

public:
    AttributeExpression(const std::string& key, int threshold)
        : key_(key), threshold_(threshold) {}

    bool interpret(const PermissionContext& ctx) const override {
        return ctx.getAttribute(key_) >= threshold_;
    }

    std::string toString() const override {
        return key_ + " >= " + std::to_string(threshold_);
    }
};
```

#### 非终结符表达式

```cpp
// 非终结符：AND 组合
class AndExpression : public BoolExpression {
private:
    std::unique_ptr<BoolExpression> left_;
    std::unique_ptr<BoolExpression> right_;

public:
    AndExpression(std::unique_ptr<BoolExpression> left,
                  std::unique_ptr<BoolExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    bool interpret(const PermissionContext& ctx) const override {
        return left_->interpret(ctx) && right_->interpret(ctx);
    }

    std::string toString() const override {
        return "(" + left_->toString() + " AND " + right_->toString() + ")";
    }
};

// 非终结符：OR 组合
class OrExpression : public BoolExpression {
private:
    std::unique_ptr<BoolExpression> left_;
    std::unique_ptr<BoolExpression> right_;

public:
    OrExpression(std::unique_ptr<BoolExpression> left,
                 std::unique_ptr<BoolExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {}

    bool interpret(const PermissionContext& ctx) const override {
        return left_->interpret(ctx) || right_->interpret(ctx);
    }

    std::string toString() const override {
        return "(" + left_->toString() + " OR " + right_->toString() + ")";
    }
};

// 非终结符：NOT 取反
class NotExpression : public BoolExpression {
private:
    std::unique_ptr<BoolExpression> expr_;

public:
    explicit NotExpression(std::unique_ptr<BoolExpression> expr)
        : expr_(std::move(expr)) {}

    bool interpret(const PermissionContext& ctx) const override {
        return !expr_->interpret(ctx);
    }

    std::string toString() const override {
        return "NOT(" + expr_->toString() + ")";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 构建规则：(hasRole(admin) OR (hasRole(editor) AND level >= 5))
    auto rule = std::make_unique<OrExpression>(
        std::make_unique<RoleExpression>("admin"),
        std::make_unique<AndExpression>(
            std::make_unique<RoleExpression>("editor"),
            std::make_unique<AttributeExpression>("level", 5)
        )
    );

    std::cout << "Rule: " << rule->toString() << "\n\n";

    // 测试用户 1：管理员
    PermissionContext admin;
    admin.addRole("admin");
    admin.setAttribute("level", 1);
    std::cout << "Admin: " << (rule->interpret(admin) ? "GRANTED" : "DENIED") << "\n";
    // GRANTED（命中 hasRole(admin)）

    // 测试用户 2：高级编辑
    PermissionContext seniorEditor;
    seniorEditor.addRole("editor");
    seniorEditor.setAttribute("level", 7);
    std::cout << "Senior Editor: "
              << (rule->interpret(seniorEditor) ? "GRANTED" : "DENIED") << "\n";
    // GRANTED（命中 hasRole(editor) AND level >= 5）

    // 测试用户 3：初级编辑
    PermissionContext juniorEditor;
    juniorEditor.addRole("editor");
    juniorEditor.setAttribute("level", 2);
    std::cout << "Junior Editor: "
              << (rule->interpret(juniorEditor) ? "GRANTED" : "DENIED") << "\n";
    // DENIED（level < 5）

    // 测试用户 4：普通用户
    PermissionContext viewer;
    viewer.addRole("viewer");
    viewer.setAttribute("level", 10);
    std::cout << "Viewer: "
              << (rule->interpret(viewer) ? "GRANTED" : "DENIED") << "\n";
    // DENIED（既不是 admin 也不是 editor）
}
```

---

### Interpreter 与 Composite 的关系

你可能注意到了——Interpreter 的语法树结构就是 Composite 模式！

| 角色 | Composite | Interpreter |
|------|-----------|-------------|
| **Component** | FileSystemComponent | BoolExpression |
| **Leaf** | File | RoleExpression / AttributeExpression |
| **Composite** | Folder | AndExpression / OrExpression |
| **操作** | display() / getSize() | interpret(context) |

> Interpreter = Composite + 递归求值。语法树是 Composite 结构，interpret() 是递归操作。

---

### 错误用法

```cpp
// 错误 1：用 Interpreter 处理复杂语法——应该用 Parser Generator
class OverEngineeredInterpreter {
    // 试图手写一个完整的 SQL 解释器
    // SQL 语法极其复杂，手写 AST 不现实
    // 正确做法：用 ANTLR、Bison 等工具生成解析器
};

// 错误 2：Context 和 Expression 耦合——表达式持有上下文引用
class CoupledExpression : public BoolExpression {
    PermissionContext& ctx_;  // 错误！表达式不应绑定到特定上下文
public:
    CoupledExpression(PermissionContext& ctx) : ctx_(ctx) {}
    bool interpret(const PermissionContext&) const override {
        return ctx_.hasRole("admin");  // 忽略了参数，用成员变量
    }
    std::string toString() const override { return "broken"; }
};

// 错误 3：忘了处理递归终止条件
class InfiniteExpression : public BoolExpression {
    std::unique_ptr<BoolExpression> self_;  // 引用自己！
public:
    bool interpret(const PermissionContext& ctx) const override {
        return self_->interpret(ctx);  // 无限递归
    }
    std::string toString() const override { return "infinite"; }
};
```

---

### 面试热点

```
Q: Interpreter 模式适合什么场景？
A: 语法简单且不频繁变化的领域特定语言（DSL）。
   比如：权限规则、数学表达式、查询过滤器。
   复杂语法应使用 Parser Generator（ANTLR、Bison）。

Q: Interpreter 模式的缺点？
A: 1) 复杂语法会导致大量的表达式类
   2) 性能不高（递归遍历语法树）
   3) 语法变化时需要修改类层次
   对性能要求高的场景，可以将语法树编译为字节码再执行。

Q: Interpreter 和 Composite 的关系？
A: Interpreter 的 AST 就是 Composite 结构。
   Composite 强调"统一处理整体与部分"，
   Interpreter 强调"递归求值"。两者常一起使用。
```

---

## 8.4 Visitor（访问者模式）

### 定义

> 表示一个作用于某对象结构中各元素的操作。Visitor 使你可以在不改变各元素的类的前提下定义作用于这些元素的新操作。

### 现实类比

就像**体检中心的各科医生**——一群员工（数据结构）去体检，不需要每个员工自己会看病。内科医生、外科医生、眼科医生（不同的 Visitor）分别对每个员工执行不同的检查。要增加"口腔科检查"，只需要新增一个医生，而不需要修改员工类。

### 问题场景

- 编译器对 AST 执行多种操作（类型检查、代码生成、优化）
- 文档导出（同一数据结构导出为 HTML、PDF、Markdown）
- 购物车计算（税费、运费、折扣——对不同商品用不同规则）
- 文件系统扫描（杀毒、统计大小、搜索关键字）

### 核心机制：双重分派

C++ 的虚函数只支持**单分派**——根据对象类型决定调用哪个方法。Visitor 通过 `accept` + `visit` 的配合实现**双重分派**——根据"元素类型"和"操作类型"两个维度决定行为：

```
第一次分派：element->accept(visitor)  → 根据 element 类型决定调用哪个 accept
第二次分派：visitor.visit(*this)      → 根据 visitor 类型决定调用哪个 visit
```

### 实现

#### 文档导出系统

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

// 前向声明
class Paragraph;
class Heading;
class CodeBlock;
class Image;

// Visitor 接口——每种元素类型对应一个 visit 方法
class DocumentVisitor {
public:
    virtual ~DocumentVisitor() = default;
    virtual void visit(const Paragraph& p) = 0;
    virtual void visit(const Heading& h) = 0;
    virtual void visit(const CodeBlock& c) = 0;
    virtual void visit(const Image& img) = 0;
};
```

#### Element 接口与具体元素

```cpp
// Element 接口
class DocumentElement {
public:
    virtual ~DocumentElement() = default;
    virtual void accept(DocumentVisitor& visitor) const = 0;
};

// 段落
class Paragraph : public DocumentElement {
private:
    std::string text_;

public:
    explicit Paragraph(const std::string& text) : text_(text) {}

    void accept(DocumentVisitor& visitor) const override {
        visitor.visit(*this);  // 第二次分派
    }

    const std::string& getText() const { return text_; }
};

// 标题
class Heading : public DocumentElement {
private:
    std::string text_;
    int level_;

public:
    Heading(const std::string& text, int level)
        : text_(text), level_(level) {}

    void accept(DocumentVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getText() const { return text_; }
    int getLevel() const { return level_; }
};

// 代码块
class CodeBlock : public DocumentElement {
private:
    std::string code_;
    std::string language_;

public:
    CodeBlock(const std::string& code, const std::string& language)
        : code_(code), language_(language) {}

    void accept(DocumentVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getCode() const { return code_; }
    const std::string& getLanguage() const { return language_; }
};

// 图片
class Image : public DocumentElement {
private:
    std::string url_;
    std::string alt_;

public:
    Image(const std::string& url, const std::string& alt)
        : url_(url), alt_(alt) {}

    void accept(DocumentVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::string& getUrl() const { return url_; }
    const std::string& getAlt() const { return alt_; }
};
```

#### Concrete Visitors

```cpp
// Visitor 1：导出为 HTML
class HtmlExportVisitor : public DocumentVisitor {
private:
    std::ostringstream output_;

public:
    void visit(const Paragraph& p) override {
        output_ << "<p>" << p.getText() << "</p>\n";
    }

    void visit(const Heading& h) override {
        output_ << "<h" << h.getLevel() << ">"
                << h.getText()
                << "</h" << h.getLevel() << ">\n";
    }

    void visit(const CodeBlock& c) override {
        output_ << "<pre><code class=\"" << c.getLanguage() << "\">"
                << c.getCode()
                << "</code></pre>\n";
    }

    void visit(const Image& img) override {
        output_ << "<img src=\"" << img.getUrl()
                << "\" alt=\"" << img.getAlt() << "\" />\n";
    }

    std::string getResult() const { return output_.str(); }
};

// Visitor 2：导出为 Markdown
class MarkdownExportVisitor : public DocumentVisitor {
private:
    std::ostringstream output_;

public:
    void visit(const Paragraph& p) override {
        output_ << p.getText() << "\n\n";
    }

    void visit(const Heading& h) override {
        output_ << std::string(h.getLevel(), '#') << " "
                << h.getText() << "\n\n";
    }

    void visit(const CodeBlock& c) override {
        output_ << "```" << c.getLanguage() << "\n"
                << c.getCode() << "\n```\n\n";
    }

    void visit(const Image& img) override {
        output_ << "![" << img.getAlt() << "](" << img.getUrl() << ")\n\n";
    }

    std::string getResult() const { return output_.str(); }
};

// Visitor 3：统计文档信息
class StatisticsVisitor : public DocumentVisitor {
private:
    int paragraphCount_ = 0;
    int headingCount_ = 0;
    int codeBlockCount_ = 0;
    int imageCount_ = 0;
    int totalChars_ = 0;

public:
    void visit(const Paragraph& p) override {
        paragraphCount_++;
        totalChars_ += p.getText().size();
    }

    void visit(const Heading& h) override {
        headingCount_++;
        totalChars_ += h.getText().size();
    }

    void visit(const CodeBlock& c) override {
        codeBlockCount_++;
        totalChars_ += c.getCode().size();
    }

    void visit(const Image& img) override {
        imageCount_++;
    }

    void printReport() const {
        std::cout << "=== Document Statistics ===\n"
                  << "Paragraphs: " << paragraphCount_ << "\n"
                  << "Headings:   " << headingCount_ << "\n"
                  << "Code blocks:" << codeBlockCount_ << "\n"
                  << "Images:     " << imageCount_ << "\n"
                  << "Total chars:" << totalChars_ << "\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 构建文档结构
    std::vector<std::unique_ptr<DocumentElement>> doc;
    doc.push_back(std::make_unique<Heading>("Design Patterns", 1));
    doc.push_back(std::make_unique<Paragraph>("Design patterns are reusable solutions."));
    doc.push_back(std::make_unique<Heading>("Visitor Pattern", 2));
    doc.push_back(std::make_unique<Paragraph>("Visitor separates algorithms from objects."));
    doc.push_back(std::make_unique<CodeBlock>("element->accept(visitor);", "cpp"));
    doc.push_back(std::make_unique<Image>("visitor.png", "Visitor UML diagram"));

    // 导出为 HTML
    HtmlExportVisitor htmlExporter;
    for (const auto& elem : doc) {
        elem->accept(htmlExporter);
    }
    std::cout << "=== HTML ===\n" << htmlExporter.getResult() << "\n";

    // 导出为 Markdown
    MarkdownExportVisitor mdExporter;
    for (const auto& elem : doc) {
        elem->accept(mdExporter);
    }
    std::cout << "=== Markdown ===\n" << mdExporter.getResult() << "\n";

    // 统计信息
    StatisticsVisitor stats;
    for (const auto& elem : doc) {
        elem->accept(stats);
    }
    stats.printReport();
}
```

输出：

```
=== HTML ===
<h1>Design Patterns</h1>
<p>Design patterns are reusable solutions.</p>
<h2>Visitor Pattern</h2>
<p>Visitor separates algorithms from objects.</p>
<pre><code class="cpp">element->accept(visitor);</code></pre>
<img src="visitor.png" alt="Visitor UML diagram" />

=== Markdown ===
# Design Patterns

Design patterns are reusable solutions.

## Visitor Pattern

Visitor separates algorithms from objects.

```cpp
element->accept(visitor);
```

![Visitor UML diagram](visitor.png)

=== Document Statistics ===
Paragraphs: 2
Headings:   2
Code blocks:1
Images:     1
Total chars:103
```

---

### 何时用 Visitor，何时不用

| 适合 Visitor | 不适合 Visitor |
|-------------|---------------|
| 元素类型稳定，操作经常变化 | 元素类型经常增加 |
| 需要在数据结构上执行多种不相关操作 | 操作和数据高度耦合 |
| 不想在元素类中堆积大量操作方法 | 元素类很少、操作简单 |

> **关键权衡**：新增一个 Visitor 很容易（只需新增一个类），但新增一种元素类型很痛苦（要改所有 Visitor）。这就是"操作维度开放，元素维度封闭"的设计取舍。

---

### Visitor vs Strategy vs Decorator

| 模式 | 目的 | 扩展方向 |
|------|------|---------|
| **Visitor** | 在不修改元素的前提下添加新操作 | 操作维度扩展（新增 Visitor） |
| **Strategy** | 替换算法实现 | 算法维度扩展（新增 Strategy） |
| **Decorator** | 动态添加职责 | 增强维度扩展（包装层叠加） |

---

### 错误用法

```cpp
// 错误 1：在 accept 中执行操作逻辑（应该委托给 visitor）
class BadElement : public DocumentElement {
    void accept(DocumentVisitor& visitor) const override {
        // 错误！不应该在 accept 里写业务逻辑
        std::cout << "I am a paragraph: " << text_ << "\n";
        visitor.visit(*this);  // 这才是 accept 唯一该做的事
    }
};

// 错误 2：修改被访问的元素（Visitor 应该是只读的，或通过返回值传递结果）
class DestructiveVisitor : public DocumentVisitor {
    void visit(Paragraph& p) override {  // 注意：非 const 引用！
        p.setText("MODIFIED");  // 不应修改元素
    }
};

// 错误 3：忘记为新元素类型更新所有 Visitor
// 新增 Table 元素后：
class Table : public DocumentElement {
    void accept(DocumentVisitor& visitor) const override {
        // visitor.visit(*this);  // 编译错误！Visitor 没有 visit(Table&)
        // 这就是 Visitor 的代价——每增加一种元素，所有 Visitor 都要修改
    }
};
```

---

### 面试热点

```
Q: Visitor 模式的双重分派是什么意思？
A: 第一次分派：调用 element->accept(visitor)，根据 element 类型
   进入对应的 accept 方法。
   第二次分派：在 accept 中调用 visitor.visit(*this)，根据 visitor
   类型进入对应的 visit 方法。
   两次虚函数调用，确定了"哪种元素"+"哪种操作"。

Q: Visitor 模式违反了哪些设计原则？又满足了哪些？
A: 违反：开闭原则（对元素类型扩展不友好，需要改所有 Visitor）
   满足：单一职责原则（操作逻辑集中在 Visitor 中，而不是分散在各元素中）
   满足：开闭原则（对操作扩展友好，新增 Visitor 不改元素）

Q: 什么时候应该用 Visitor？
A: 当数据结构稳定（元素类型不常变），但需要频繁添加新操作时。
   典型场景：编译器 AST（节点类型固定，但分析、优化、代码生成等操作不断增加）。
```

---

## 8.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Memento** | 保存和恢复状态 | 状态快照，friend 保护封装 | "快照" |
| **Interpreter** | 定义和执行简单语法 | AST + 递归求值 | "语法树" |
| **Visitor** | 对稳定结构添加新操作 | 双重分派，操作与数据分离 | "双分派" |

### 统一思想

三种模式都在解决**"如何从外部与对象内部结构交互"**：

- Memento：从外部**保存和恢复**对象的内部状态，但不暴露状态细节
- Interpreter：从外部**解释执行**一个树状结构中的每个节点
- Visitor：从外部**遍历并操作**一组对象，但不修改对象本身

它们的共同精神是：**尊重封装性，把变化的部分外置**。

### 行为型模式全景回顾

```
对象间通信            请求封装              解耦机制              高级行为
Observer             Command              Chain of Resp.       Memento
Strategy             Iterator             Mediator             Interpreter
State                                                          Visitor
Template Method
```

---

### 下一课预告

第 9 课（最后一课）我们将学习 **Flyweight**（享元模式），并进行 23 种模式的总复习、SOLID 原则讲解，以及实战场景分析。

---

## 8.6 课后练习

1. **Memento**：实现一个"绘图程序"的撤销系统：
   - `Canvas` 包含一组 `Shape`（位置、颜色、大小）
   - 支持 `addShape()`、`moveShape()`、`removeShape()`
   - 每次操作前自动保存快照，支持多步撤销
   - 限制最多保存 20 个快照（超出时丢弃最早的）

2. **Interpreter**：实现一个简单的数学表达式求值器：
   - 支持变量（`x`、`y`）、数字字面量
   - 支持四则运算（`+`、`-`、`*`、`/`）
   - 实现一个 `parse(const std::string& expr)` 函数，将字符串解析为 AST
   - 提示：先实现不带括号的版本，再考虑优先级

3. **Visitor**：实现一个"编译器 AST"访问者系统：
   - 元素类型：`NumberLiteral`、`BinaryOp`、`VariableRef`、`Assignment`
   - Visitor 1：`PrintVisitor`——将 AST 还原为可读的表达式字符串
   - Visitor 2：`EvalVisitor`——求值 AST 并返回计算结果
   - Visitor 3：`VariableCollector`——收集 AST 中使用的所有变量名
