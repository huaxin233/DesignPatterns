# 第 7 课：高级创建型模式

> **前置知识**：第 1 课内容（Singleton、Factory Method）、智能指针、移动语义、拷贝构造函数
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Builder、Prototype、Abstract Factory 三种高级创建型模式，理解复杂对象创建的各种手段

---

## 7.1 课程导入

### 从简单创建到复杂创建

第 1 课我们学了两种基础创建型模式：

- Singleton：控制实例数量——全局唯一
- Factory Method：控制谁创建——子类决定

但现实中对象创建的需求远比这复杂：

| 需求 | 痛点 | 解决方案 |
|------|------|---------|
| 对象有很多可选参数 | 构造函数参数爆炸 | **Builder** |
| 需要复制已有对象 | 不知道对象的具体类型 | **Prototype** |
| 需要创建一族相关对象 | 多个工厂方法难以保证一致性 | **Abstract Factory** |

---

## 7.2 Builder（建造者模式）

### 定义

> 将一个复杂对象的构建与它的表示分离，使得同样的构建过程可以创建不同的表示。

### 现实类比

就像**点餐系统**——你不需要一次性说出"我要一个加芝士、不要洋葱、加培根、中号、配薯条和可乐的汉堡"。而是一步一步选择：主食 → 配料 → 饮料 → 大小。同样的点餐流程可以组合出完全不同的套餐。

### 问题场景

- 配置对象（数据库连接配置、HTTP 请求构建）
- 文档生成（HTML/PDF/Markdown 不同格式）
- 游戏角色创建（种族、职业、技能、装备逐步选择）
- SQL 查询构建器（SELECT → FROM → WHERE → ORDER BY）

### 问题：构造函数参数爆炸

```cpp
// 反面教材：参数太多，调用者根本记不住顺序
class Pizza {
public:
    Pizza(const std::string& dough, const std::string& sauce,
          const std::string& cheese, bool pepperoni, bool mushrooms,
          bool onions, bool olives, bool peppers, int size) {
        // ...
    }
};

// 调用时完全不可读
Pizza p("thin", "tomato", "mozzarella", true, false, true, false, true, 12);
// true/false 分别是什么？没人知道
```

---

### 实现

#### 版本 1：经典 Builder（带 Director）

```cpp
#include <iostream>
#include <string>
#include <memory>
#include <vector>

// 产品类——复杂对象
class House {
private:
    std::string foundation_;
    std::string structure_;
    std::string roof_;
    std::vector<std::string> features_;

public:
    void setFoundation(const std::string& f) { foundation_ = f; }
    void setStructure(const std::string& s) { structure_ = s; }
    void setRoof(const std::string& r) { roof_ = r; }
    void addFeature(const std::string& f) { features_.push_back(f); }

    void show() const {
        std::cout << "House:\n"
                  << "  Foundation: " << foundation_ << "\n"
                  << "  Structure:  " << structure_ << "\n"
                  << "  Roof:       " << roof_ << "\n"
                  << "  Features:   ";
        for (const auto& f : features_) {
            std::cout << f << " ";
        }
        std::cout << "\n";
    }
};

// 抽象建造者
class HouseBuilder {
public:
    virtual ~HouseBuilder() = default;
    virtual void buildFoundation() = 0;
    virtual void buildStructure() = 0;
    virtual void buildRoof() = 0;
    virtual void addFeatures() = 0;
    virtual std::unique_ptr<House> getResult() = 0;
};

// 具体建造者 A：木屋
class WoodenHouseBuilder : public HouseBuilder {
private:
    std::unique_ptr<House> house_;

public:
    WoodenHouseBuilder() : house_(std::make_unique<House>()) {}

    void buildFoundation() override {
        house_->setFoundation("Wooden piles");
    }

    void buildStructure() override {
        house_->setStructure("Wooden frame");
    }

    void buildRoof() override {
        house_->setRoof("Wooden shingles");
    }

    void addFeatures() override {
        house_->addFeature("Fireplace");
        house_->addFeature("Porch");
    }

    std::unique_ptr<House> getResult() override {
        return std::move(house_);
    }
};

// 具体建造者 B：石屋
class StoneHouseBuilder : public HouseBuilder {
private:
    std::unique_ptr<House> house_;

public:
    StoneHouseBuilder() : house_(std::make_unique<House>()) {}

    void buildFoundation() override {
        house_->setFoundation("Concrete foundation");
    }

    void buildStructure() override {
        house_->setStructure("Stone walls");
    }

    void buildRoof() override {
        house_->setRoof("Slate tiles");
    }

    void addFeatures() override {
        house_->addFeature("Garden");
        house_->addFeature("Garage");
    }

    std::unique_ptr<House> getResult() override {
        return std::move(house_);
    }
};

// Director——控制构建顺序
class Director {
public:
    std::unique_ptr<House> construct(HouseBuilder& builder) {
        builder.buildFoundation();
        builder.buildStructure();
        builder.buildRoof();
        builder.addFeatures();
        return builder.getResult();
    }
};
```

**客户端代码**：

```cpp
int main() {
    Director director;

    WoodenHouseBuilder woodenBuilder;
    auto woodenHouse = director.construct(woodenBuilder);
    woodenHouse->show();
    // Output:
    // House:
    //   Foundation: Wooden piles
    //   Structure:  Wooden frame
    //   Roof:       Wooden shingles
    //   Features:   Fireplace Porch

    StoneHouseBuilder stoneBuilder;
    auto stoneHouse = director.construct(stoneBuilder);
    stoneHouse->show();
    // Output:
    // House:
    //   Foundation: Concrete foundation
    //   Structure:  Stone walls
    //   Roof:       Slate tiles
    //   Features:   Garden Garage
}
```

---

#### 版本 2：链式调用 Builder（现代 C++ 推荐）

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <optional>

class HttpRequest {
public:
    // 公开字段，由 Builder 设置
    std::string method;
    std::string url;
    std::vector<std::pair<std::string, std::string>> headers;
    std::optional<std::string> body;
    int timeout_ms = 30000;

    void send() const {
        std::cout << method << " " << url << "\n";
        for (const auto& [key, value] : headers) {
            std::cout << "  " << key << ": " << value << "\n";
        }
        if (body) {
            std::cout << "  Body: " << *body << "\n";
        }
        std::cout << "  Timeout: " << timeout_ms << "ms\n";
    }
};

// 链式 Builder——每个方法返回自身引用
class HttpRequestBuilder {
private:
    HttpRequest request_;

public:
    HttpRequestBuilder& setMethod(const std::string& method) {
        request_.method = method;
        return *this;
    }

    HttpRequestBuilder& setUrl(const std::string& url) {
        request_.url = url;
        return *this;
    }

    HttpRequestBuilder& addHeader(const std::string& key, const std::string& value) {
        request_.headers.emplace_back(key, value);
        return *this;
    }

    HttpRequestBuilder& setBody(const std::string& body) {
        request_.body = body;
        return *this;
    }

    HttpRequestBuilder& setTimeout(int ms) {
        request_.timeout_ms = ms;
        return *this;
    }

    HttpRequest build() {
        // 可以在这里做参数校验
        if (request_.method.empty()) {
            request_.method = "GET";
        }
        return std::move(request_);
    }
};
```

**客户端代码**：

```cpp
int main() {
    auto request = HttpRequestBuilder()
        .setMethod("POST")
        .setUrl("https://api.example.com/users")
        .addHeader("Content-Type", "application/json")
        .addHeader("Authorization", "Bearer token123")
        .setBody(R"({"name": "Alice"})")
        .setTimeout(5000)
        .build();

    request.send();
    // Output:
    // POST https://api.example.com/users
    //   Content-Type: application/json
    //   Authorization: Bearer token123
    //   Body: {"name": "Alice"}
    //   Timeout: 5000ms
}
```

**链式调用的核心**：每个 setter 返回 `*this`（Builder 自身的引用），允许连续调用。

---

### 经典 Builder vs 链式 Builder

| 维度 | 经典 Builder（带 Director） | 链式 Builder |
|------|---------------------------|-------------|
| **构建顺序** | Director 控制固定顺序 | 客户端自由决定 |
| **适用场景** | 步骤有严格顺序（盖房子） | 参数可选、无严格顺序（配置） |
| **复杂度** | 较高（Builder + Director） | 较低（一个 Builder 类） |
| **灵活性** | 同样的流程，不同的建造者 | 同一个建造者，不同的参数组合 |

> **实际工程**：链式 Builder 更常见，大多数配置类/请求类采用此形式。

---

### 错误用法

```cpp
// 错误 1：Builder 不可复用——build() 后状态残留
class BadBuilder {
    HttpRequest request_;
public:
    HttpRequest build() {
        return request_;  // 没有重置状态！
    }
    // 第二次调用 build() 会返回混合了上次配置的对象
};

// 错误 2：Builder 暴露了产品的内部细节
class LeakyBuilder {
public:
    HttpRequest& getInternalRequest() {
        return request_;  // 直接暴露产品引用，破坏封装！
    }
};

// 错误 3：参数少时滥用 Builder（过度设计）
class PointBuilder {  // 只有 x, y 两个参数
public:
    PointBuilder& setX(int x) { /* ... */ return *this; }
    PointBuilder& setY(int y) { /* ... */ return *this; }
    Point build() { /* ... */ }
    // 过度设计！直接 Point(x, y) 就好
};
```

---

### 面试热点

```
Q: Builder 模式的核心价值是什么？
A:
   1. 解决构造函数参数爆炸问题
   2. 分步构建，每一步都有明确语义
   3. 同样的构建过程可以产生不同的产品
   4. 链式调用提升代码可读性

Q: Builder 和 Factory Method 的区别？
A: Factory Method：一步创建，关注"创建哪种类型"
   Builder：分步创建，关注"如何一步步构建复杂对象"
   Factory 是"选择"，Builder 是"组装"
```

---

## 7.3 Prototype（原型模式）

### 定义

> 用原型实例指定创建对象的种类，并通过拷贝这些原型来创建新对象。

### 现实类比

就像**细胞分裂**——一个细胞不需要从零开始构建新细胞，而是直接复制自己，然后在副本上做少量修改。原型模式就是"先复制，再微调"。

### 问题场景

- 创建成本高的对象（需要读取数据库/网络请求来初始化）
- 不知道具体类型的对象复制（只知道基类指针）
- 配置模板（一个配置"原型"，克隆后微调）
- 游戏中的敌人生成（从模板怪物克隆，然后调整属性）

### 问题：如何复制一个只知道基类指针的对象？

```cpp
// 你只有一个基类指针，不知道具体类型
Shape* original = getShapeFromSomewhere();

// 错误！你不知道 original 是 Circle 还是 Rectangle
Shape* copy = new Shape(*original);  // 对象切片！丢失子类数据

// 正确做法：让对象自己克隆自己
std::unique_ptr<Shape> copy = original->clone();  // 多态克隆
```

---

### 实现

#### 基础原型

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// 原型接口
class Monster {
public:
    virtual ~Monster() = default;
    virtual std::unique_ptr<Monster> clone() const = 0;
    virtual void display() const = 0;

    // 属性
    std::string name;
    int health = 100;
    int attack = 10;
    int defense = 5;
};

// 具体原型 A：哥布林
class Goblin : public Monster {
private:
    std::string weapon_;

public:
    Goblin(const std::string& weapon) : weapon_(weapon) {
        name = "Goblin";
        health = 50;
        attack = 8;
        defense = 3;
    }

    std::unique_ptr<Monster> clone() const override {
        // 拷贝构造：复制所有字段
        return std::make_unique<Goblin>(*this);
    }

    void display() const override {
        std::cout << name << " [HP:" << health
                  << " ATK:" << attack
                  << " DEF:" << defense
                  << " Weapon:" << weapon_ << "]\n";
    }
};

// 具体原型 B：龙
class Dragon : public Monster {
private:
    std::string element_;
    int wingspan_;

public:
    Dragon(const std::string& element, int wingspan)
        : element_(element), wingspan_(wingspan) {
        name = "Dragon";
        health = 500;
        attack = 50;
        defense = 30;
    }

    std::unique_ptr<Monster> clone() const override {
        return std::make_unique<Dragon>(*this);
    }

    void display() const override {
        std::cout << name << " [HP:" << health
                  << " ATK:" << attack
                  << " DEF:" << defense
                  << " Element:" << element_
                  << " Wingspan:" << wingspan_ << "m]\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 创建原型
    Goblin goblinPrototype("Short Sword");
    Dragon dragonPrototype("Fire", 20);

    // 克隆并微调
    auto goblin1 = goblinPrototype.clone();
    goblin1->name = "Goblin Scout";
    goblin1->health = 40;

    auto goblin2 = goblinPrototype.clone();
    goblin2->name = "Goblin Chief";
    goblin2->attack = 15;

    auto dragon1 = dragonPrototype.clone();
    dragon1->name = "Elder Dragon";
    dragon1->health = 800;

    goblin1->display();  // Goblin Scout [HP:40 ATK:8 DEF:3 Weapon:Short Sword]
    goblin2->display();  // Goblin Chief [HP:50 ATK:15 DEF:3 Weapon:Short Sword]
    dragon1->display();  // Elder Dragon [HP:800 ATK:50 DEF:30 Element:Fire Wingspan:20m]
}
```

---

#### 原型注册表（Prototype Registry）

```cpp
// 原型注册表——集中管理原型模板
class MonsterRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<Monster>> prototypes_;

public:
    void registerPrototype(const std::string& key,
                           std::unique_ptr<Monster> prototype) {
        prototypes_[key] = std::move(prototype);
    }

    std::unique_ptr<Monster> create(const std::string& key) const {
        auto it = prototypes_.find(key);
        if (it != prototypes_.end()) {
            return it->second->clone();
        }
        return nullptr;
    }
};
```

**使用注册表**：

```cpp
int main() {
    MonsterRegistry registry;

    // 注册原型模板
    registry.registerPrototype("goblin",
        std::make_unique<Goblin>("Short Sword"));
    registry.registerPrototype("dragon",
        std::make_unique<Dragon>("Fire", 20));

    // 通过字符串键快速创建
    auto enemy1 = registry.create("goblin");
    auto enemy2 = registry.create("dragon");
    auto enemy3 = registry.create("goblin");  // 又一个哥布林

    enemy1->display();
    enemy2->display();
    enemy3->display();
}
```

---

### 深拷贝 vs 浅拷贝

```cpp
class DeepCopyExample {
private:
    std::string name_;
    std::unique_ptr<int[]> data_;   // 堆上数据
    int dataSize_;

public:
    DeepCopyExample(const std::string& name, int size)
        : name_(name), dataSize_(size), data_(std::make_unique<int[]>(size)) {
        for (int i = 0; i < size; ++i) {
            data_[i] = i * 10;
        }
    }

    // 深拷贝：复制堆上的数据，而不是指针
    std::unique_ptr<DeepCopyExample> clone() const {
        auto copy = std::make_unique<DeepCopyExample>(name_, dataSize_);
        // 逐元素复制堆数据
        for (int i = 0; i < dataSize_; ++i) {
            copy->data_[i] = data_[i];
        }
        return copy;
    }
};
```

| 维度 | 浅拷贝 | 深拷贝 |
|------|--------|--------|
| **指针成员** | 复制指针地址（共享数据） | 复制指针指向的数据（独立副本） |
| **修改副本** | 会影响原对象 | 不影响原对象 |
| **性能** | 快（只复制地址） | 慢（需要复制全部数据） |
| **安全性** | 可能导致 double-free | 安全，各自独立 |

> **原型模式必须使用深拷贝**——否则克隆对象与原型共享内部状态，修改一个会影响另一个。

---

### 错误用法

```cpp
// 错误 1：clone() 中忘记复制基类成员
class BadGoblin : public Monster {
public:
    std::unique_ptr<Monster> clone() const override {
        auto copy = std::make_unique<BadGoblin>();
        // 忘记复制 name, health, attack, defense！
        return copy;
    }
};

// 错误 2：浅拷贝导致共享内部状态
class BadClone : public Monster {
    int* sharedData_;  // 裸指针
public:
    std::unique_ptr<Monster> clone() const override {
        auto copy = std::make_unique<BadClone>(*this);
        // 默认拷贝构造只复制指针，两个对象共享同一块内存！
        // delete 其中一个后，另一个变成悬挂指针
        return copy;
    }
};

// 错误 3：对象切片——用基类拷贝构造函数
Monster* original = new Dragon("Ice", 15);
Monster copy = *original;  // 对象切片！Dragon 特有的数据全部丢失
```

---

### 面试热点

```
Q: Prototype 和 Factory Method 的区别？
A: Factory Method：通过子类的工厂方法创建新对象
   Prototype：通过克隆已有对象创建新对象
   Factory 是"从头造"，Prototype 是"复制改"

Q: C++ 中如何正确实现 clone()？
A: 1. 基类声明 virtual clone()，返回 unique_ptr<Base>
   2. 子类 override clone()，在其中用拷贝构造创建自身
   3. 确保拷贝构造函数实现了深拷贝
   4. 使用 std::make_unique<Derived>(*this) 是最常见的做法

Q: 什么时候用 Prototype 而不是 new？
A: 1. 创建成本高（需要读文件/网络/数据库来初始化）
   2. 只有基类指针，不知道具体类型
   3. 需要大量相似对象（先建模板，再克隆微调）
```

---

## 7.4 Abstract Factory（抽象工厂模式）

### 定义

> 提供一个接口，用于创建**相关或依赖对象的家族**，而无需指定它们的具体类。

### 现实类比

就像**宜家的家具系列**——宜家有"北欧简约"系列和"工业复古"系列。每个系列都包含椅子、桌子、沙发，但风格完全统一。你选了"北欧简约"系列，得到的椅子、桌子、沙发一定是同一风格的——不会出现北欧椅子配工业桌子的混搭。

### 问题场景

- 跨平台 UI 框架（Windows/Mac/Linux × Button/Checkbox/TextBox）
- 数据库访问层（MySQL/PostgreSQL/SQLite × Connection/Command/Reader）
- 游戏主题（暗黑风/可爱风 × 角色/场景/UI）

### 问题：多个工厂方法如何保证产品族一致性？

```cpp
// 危险！各自独立的工厂方法无法保证一致性
auto button = createButton("windows");    // Windows 按钮
auto checkbox = createCheckbox("mac");    // Mac 复选框！风格不一致！

// Abstract Factory 解决：一个工厂创建整族产品
auto factory = createFactory("windows");
auto button = factory->createButton();     // Windows 按钮
auto checkbox = factory->createCheckbox(); // Windows 复选框——风格一致
```

---

### 实现

#### 抽象产品族

```cpp
#include <iostream>
#include <memory>
#include <string>

// 抽象产品 A：按钮
class Button {
public:
    virtual ~Button() = default;
    virtual void render() const = 0;
    virtual void onClick() const = 0;
};

// 抽象产品 B：文本框
class TextBox {
public:
    virtual ~TextBox() = default;
    virtual void render() const = 0;
    virtual void setText(const std::string& text) = 0;
};

// 抽象产品 C：复选框
class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual void render() const = 0;
    virtual void toggle() = 0;
};
```

#### Windows 产品族

```cpp
class WindowsButton : public Button {
public:
    void render() const override {
        std::cout << "[Windows Button] rendered\n";
    }
    void onClick() const override {
        std::cout << "[Windows Button] clicked\n";
    }
};

class WindowsTextBox : public TextBox {
public:
    void render() const override {
        std::cout << "[Windows TextBox] rendered\n";
    }
    void setText(const std::string& text) override {
        std::cout << "[Windows TextBox] text set to: " << text << "\n";
    }
};

class WindowsCheckbox : public Checkbox {
public:
    void render() const override {
        std::cout << "[Windows Checkbox] rendered\n";
    }
    void toggle() override {
        std::cout << "[Windows Checkbox] toggled\n";
    }
};
```

#### Mac 产品族

```cpp
class MacButton : public Button {
public:
    void render() const override {
        std::cout << "[Mac Button] rendered\n";
    }
    void onClick() const override {
        std::cout << "[Mac Button] clicked\n";
    }
};

class MacTextBox : public TextBox {
public:
    void render() const override {
        std::cout << "[Mac TextBox] rendered\n";
    }
    void setText(const std::string& text) override {
        std::cout << "[Mac TextBox] text set to: " << text << "\n";
    }
};

class MacCheckbox : public Checkbox {
public:
    void render() const override {
        std::cout << "[Mac Checkbox] rendered\n";
    }
    void toggle() override {
        std::cout << "[Mac Checkbox] toggled\n";
    }
};
```

#### 抽象工厂与具体工厂

```cpp
// 抽象工厂——创建一整族 UI 组件
class UIFactory {
public:
    virtual ~UIFactory() = default;
    virtual std::unique_ptr<Button> createButton() const = 0;
    virtual std::unique_ptr<TextBox> createTextBox() const = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() const = 0;
};

// 具体工厂 A：Windows UI 工厂
class WindowsUIFactory : public UIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<WindowsButton>();
    }
    std::unique_ptr<TextBox> createTextBox() const override {
        return std::make_unique<WindowsTextBox>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<WindowsCheckbox>();
    }
};

// 具体工厂 B：Mac UI 工厂
class MacUIFactory : public UIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<MacButton>();
    }
    std::unique_ptr<TextBox> createTextBox() const override {
        return std::make_unique<MacTextBox>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<MacCheckbox>();
    }
};
```

#### 客户端代码

```cpp
// 应用程序——只依赖抽象工厂和抽象产品
class Application {
private:
    std::unique_ptr<Button> button_;
    std::unique_ptr<TextBox> textBox_;
    std::unique_ptr<Checkbox> checkbox_;

public:
    explicit Application(const UIFactory& factory) {
        button_ = factory.createButton();
        textBox_ = factory.createTextBox();
        checkbox_ = factory.createCheckbox();
    }

    void renderUI() const {
        button_->render();
        textBox_->render();
        checkbox_->render();
    }

    void interact() const {
        button_->onClick();
        textBox_->setText("Hello World");
        checkbox_->toggle();
    }
};

int main() {
    // 根据平台选择工厂——整个应用只需要改这一行
    std::unique_ptr<UIFactory> factory;

    #ifdef _WIN32
        factory = std::make_unique<WindowsUIFactory>();
    #else
        factory = std::make_unique<MacUIFactory>();
    #endif

    Application app(*factory);
    app.renderUI();
    app.interact();
    // Mac 平台 Output:
    // [Mac Button] rendered
    // [Mac TextBox] rendered
    // [Mac Checkbox] rendered
    // [Mac Button] clicked
    // [Mac TextBox] text set to: Hello World
    // [Mac Checkbox] toggled
}
```

---

### 三种工厂模式对比

| 模式 | 工厂数量 | 产品数量 | 核心问题 |
|------|---------|---------|---------|
| **Simple Factory** | 1 个工厂类 | 1 种产品的多种变体 | 集中创建逻辑 |
| **Factory Method** | N 个工厂子类 | 1 种产品的多种变体 | 子类决定创建类型 |
| **Abstract Factory** | N 个工厂子类 | M 种产品的多种变体 | 创建**一族**相关产品 |

```
Simple Factory:     1 工厂  →  N 产品变体
Factory Method:     N 工厂  →  每个工厂 1 产品
Abstract Factory:   N 工厂  →  每个工厂 M 产品（产品族）
```

---

### 错误用法

```cpp
// 错误 1：混用不同工厂的产品（破坏一致性）
WindowsUIFactory winFactory;
MacUIFactory macFactory;
auto button = winFactory.createButton();     // Windows
auto textBox = macFactory.createTextBox();   // Mac！风格混乱

// 错误 2：在抽象工厂中添加新产品类型（违反开闭原则）
class UIFactory {
    virtual std::unique_ptr<Button> createButton() = 0;
    virtual std::unique_ptr<TextBox> createTextBox() = 0;
    // 新增 Slider？所有具体工厂都要改！
    virtual std::unique_ptr<Slider> createSlider() = 0;
};

// 错误 3：只有一种产品时用抽象工厂（过度设计）
class SingleProductFactory {
    virtual std::unique_ptr<Button> createButton() = 0;
    // 只有一种产品，用 Factory Method 就够了！
};
```

---

### 面试热点

```
Q: Abstract Factory 和 Factory Method 的区别？
A: Factory Method：创建单个产品，每个子类对应一种产品
   Abstract Factory：创建产品族，每个子类对应一组相关产品
   Abstract Factory 通常内部使用多个 Factory Method

Q: Abstract Factory 的最大缺点？
A: 添加新产品类型时，需要修改抽象工厂接口和所有具体工厂。
   如果产品族经常新增产品类型，Abstract Factory 不合适。
   但如果是新增产品族（如新增 Linux 风格），只需新增一个工厂类。

Q: 什么时候该用 Abstract Factory？
A: 1. 系统需要独立于产品的创建方式
   2. 系统有多个产品族，运行时只使用其中一族
   3. 同一族的产品需要保证一致性
```

---

## 7.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Builder** | 复杂对象参数过多 | 分步构建，链式调用 | "一步一步" |
| **Prototype** | 复制未知类型的对象 | 虚函数 clone()，深拷贝 | "克隆" |
| **Abstract Factory** | 一族相关产品的一致性 | 工厂族，每族一个工厂 | "产品族" |

### 统一思想

三种模式都在解决**"如何优雅地创建复杂对象"**：

- Builder：**分解复杂度**——把一次性的大构造拆成多步小构造
- Prototype：**复用已有对象**——不从零开始，而是在已有基础上修改
- Abstract Factory：**保证一致性**——相关的对象必须来自同一族

### 五种创建型模式全景

```
控制实例数量          →  控制谁创建        →  控制创建过程
Singleton             Factory Method       Builder
                      Abstract Factory     Prototype
```

| 场景 | 推荐模式 |
|------|---------|
| 全局唯一实例 | Singleton |
| 单产品，子类决定类型 | Factory Method |
| 多产品族，保证一致性 | Abstract Factory |
| 参数多、步骤多的复杂对象 | Builder |
| 复制已有对象 | Prototype |

### 下一课预告

第 8 课我们将学习三个高级行为型模式：**Memento**、**Interpreter**、**Visitor**，理解状态存储、解释执行与操作扩展。

---

## 7.6 课后练习

1. **Builder**：实现一个"SQL 查询构建器"：
   - 支持链式调用：`QueryBuilder().select("name, age").from("users").where("age > 18").orderBy("name").build()`
   - `build()` 返回拼接好的 SQL 字符串
   - 支持可选的 `limit()` 和 `groupBy()`

2. **Prototype**：实现一个"文档模板系统"：
   - `Document` 是原型基类，包含标题、内容、样式
   - `Report` 和 `Letter` 是具体原型
   - 通过 `DocumentRegistry` 注册模板，克隆后修改标题和内容
   - 确保深拷贝——修改克隆体不影响原模板

3. **Abstract Factory**：实现一个"游戏主题工厂"：
   - 两个主题：`DarkTheme` 和 `LightTheme`
   - 每个主题包含：`Background`、`Character`、`Enemy` 三种产品
   - 客户端通过工厂创建一整套主题组件，保证风格一致

---
