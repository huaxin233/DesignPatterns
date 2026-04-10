# 第 3 课：进阶结构型模式

> **前置知识**：第 1、2 课内容（类继承层次结构、多态、组合、智能指针）
> **本课时长**：约 90 分钟
> **学习目标**：掌握 Composite、Proxy、Bridge 三种进阶结构型模式，理解更复杂的对象组合结构

---

## 3.1 课程导入

### 从简单组合到复杂结构

第 2 课我们学习了三种基础结构型模式，它们处理的是**一对一**的对象组合：

- Adapter：一个类适配另一个类
- Decorator：一个对象包装另一个对象
- Facade：一组类提供一个统一入口

第 3 课我们将学习处理**更复杂的结构**：

| 结构 | 解决的问题 | 代表模式 |
|------|---------|---------|
| **树形递归结构** | 整体与部分的一致性 | Composite |
| **替身对象** | 访问控制与延迟加载 | Proxy |
| **双维度分离** | 两个独立维度变化 | Bridge |

---

## 3.2 Composite（组合模式）

### 定义

> 将对象组合成树形结构以表示"部分-整体"的层次结构。组合模式使得用户对单个对象和组合对象的使用具有一致性。

### 现实类比

就像**文件系统**——文件夹里可以放文件，也可以放文件夹。文件夹和文件都"是一个项目"，但文件夹可以包含其他项目。这就是"整体-部分"的一致性：无论你操作的是文件还是文件夹，接口是一样的。

### 问题场景

- 文件系统（文件和文件夹）
- 组织架构（公司 → 部门 → 团队 → 员工）
- GUI 容器（窗口可以包含面板、按钮、文本框）
- 菜单系统（菜单项和子菜单）

### 实现

#### 组件基类（统一接口）

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// 组件抽象基类——文件和文件夹的统一接口
class FileSystemComponent {
public:
    virtual ~FileSystemComponent() = default;

    // 统一接口：显示名称
    virtual void display(int indent = 0) const = 0;

    // 统一接口：获取大小
    virtual int getSize() const = 0;
};
```

#### 叶节点：文件

```cpp
// 叶节点——具体文件
class File : public FileSystemComponent {
private:
    std::string name_;
    int size_;

public:
    File(const std::string& name, int size) : name_(name), size_(size) {}

    void display(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "📄 " << name_
                  << " (" << size_ << " KB)\n";
    }

    int getSize() const override {
        return size_;
    }
};
```

#### 容器节点：文件夹

```cpp
// 组合节点——文件夹，可以包含子组件
class Folder : public FileSystemComponent {
private:
    std::string name_;
    std::vector<std::unique_ptr<FileSystemComponent>> children_;

public:
    explicit Folder(const std::string& name) : name_(name) {}

    // 添加子组件（文件或文件夹）
    void add(std::unique_ptr<FileSystemComponent> component) {
        children_.push_back(std::move(component));
    }

    // 移除子组件
    void remove(const std::string& name) {
        children_.erase(
            std::remove_if(children_.begin(), children_.end(),
                [&name](const std::unique_ptr<FileSystemComponent>& c) {
                    return c->getName() == name;
                }),
            children_.end()
        );
    }

    void display(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "📁 " << name_
                  << " (" << getSize() << " KB)\n";
        // 递归显示所有子组件
        for (const auto& child : children_) {
            child->display(indent + 4);
        }
    }

    int getSize() const override {
        int total = 0;
        for (const auto& child : children_) {
            total += child->getSize();
        }
        return total;
    }

    // 辅助方法：获取名称（用于 remove）
    const std::string& getName() const { return name_; }
};
```

**客户端代码**：

```cpp
int main() {
    // 创建文件
    auto file1 = std::unique_ptr<File>(new File("report.pdf", 1024));
    auto file2 = std::unique_ptr<File>(new File("notes.txt", 128));
    auto file3 = std::unique_ptr<File>(new File("image.png", 2048));

    // 创建子文件夹
    auto subFolder = std::unique_ptr<Folder>(new Folder("images"));
    subFolder->add(std::unique_ptr<File>(new File("photo1.jpg", 512)));
    subFolder->add(std::unique_ptr<File>(new File("photo2.jpg", 768)));

    // 创建根文件夹
    Folder root("root");
    root.add(std::move(file1));
    root.add(std::move(file2));
    root.add(std::move(subFolder));
    root.add(std::move(file3));

    // 统一接口调用——无需知道是文件还是文件夹
    root.display();
    // Output:
    // 📁 root (3712 KB)
    //     📄 report.pdf (1024 KB)
    //     📄 notes.txt (128 KB)
    //     📁 images (1280 KB)
    //         📄 photo1.jpg (512 KB)
    //         📄 photo2.jpg (768 KB)
    //     📄 image.png (2048 KB)

    std::cout << "Total size: " << root.getSize() << " KB\n";
}
```

---

### 透明方式 vs 安全方式

| 方式 | 特点 | 风险 |
|------|------|------|
| **透明方式** | 叶节点和容器有完全相同的接口 | 容器特有方法（add/remove）在叶节点调用时无意义 |
| **安全方式** | 容器和叶节点接口分开，只在容器上暴露 add/remove | 客户端需要区分对象类型 |

> **推荐**：透明方式更常用（std::filesystem 采用此方式），符合里氏替换原则。

---

### 错误用法

```cpp
// 错误 1：组合结构循环引用导致内存泄漏
class BadFolder : public FileSystemComponent {
    std::vector<FileSystemComponent*> children_;  // 裸指针！不用 smart ptr
public:
    void add(FileSystemComponent* c) { children_.push_back(c); }
    // 手动 delete？不安全！子文件夹可能再次 delete 子元素
};

// 错误 2：在遍历时修改容器（迭代器失效）
class RecklessFolder : public FileSystemComponent {
    std::vector<std::unique_ptr<FileSystemComponent>> children_;
public:
    void badOperation() {
        for (auto& child : children_) {
            // 在遍历过程中可能删除元素，导致迭代器失效
            if (child->getSize() == 0) {
                children_.clear();  // 危险！
            }
        }
    }
};

// 错误 3：叶节点不支持的操作未做处理
class ThoughtlessFile : public FileSystemComponent {
    void add(std::unique_ptr<FileSystemComponent> c) override {
        // 叶节点不应该支持 add，但没做任何处理或报错
    }
};
```

---

### 面试热点

```
Q: 组合模式的核心思想是什么？
A: 让单个对象（叶节点）和组合对象（容器）具有统一的接口，
   客户端可以一致地处理两者，无需关心具体类型。

Q: 组合模式与装饰器模式的区别？
A: 装饰器：包装一个对象，添加职责（一对一）
   组合：包装多个对象，形成树形结构（一对多）

Q: 组合模式在哪些标准库/框架中有应用？
A: std::filesystem（path、directory_entry）
   Qt 的 QObject 树（parent-child）
   DOM 树（Element 和 Text 节点）
```

---

## 3.3 Proxy（代理模式）

### 定义

> 为其他对象提供一种代理以控制对这个对象的访问。

### 现实类比

就像**房产中介**——你不直接去找房东（房主对象），而是通过中介（代理对象）来操作。中介可以帮你：
- 房东不在时先展示房源（虚代理）
- 筛选不合格租客（保护代理）
- 收取中介费（智能引用）

### 问题场景

| 代理类型 | 使用场景 |
|---------|---------|
| **虚代理** | 图片加载（大图延迟加载，先显示占位符） |
| **保护代理** | 权限控制（只有 VIP 才能访问某些资源） |
| **远程代理** | 远程服务调用（RMI、gRPC -stub） |
| **智能引用** | 自动加锁、引用计数（shared_ptr 实现） |

### 实现

#### 虚代理（延迟加载）

```cpp
#include <iostream>
#include <string>
#include <memory>

// Subject 接口——客户端看到的统一接口
class Image {
public:
    virtual ~Image() = default;
    virtual void display() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
};

// RealSubject——真实图片（可能很大）
class RealImage : public Image {
private:
    std::string filename_;
    int width_;
    int height_;

    // 模拟从磁盘加载
    void loadFromDisk() {
        std::cout << "Loading high-resolution image: " << filename_ << "\n";
        width_ = 1920;
        height_ = 1080;
    }

public:
    explicit RealImage(const std::string& filename) : filename_(filename) {
        loadFromDisk();  // 构造时直接加载
    }

    void display() const override {
        std::cout << "Displaying: " << filename_
                  << " (" << width_ << "x" << height_ << ")\n";
    }

    int getWidth() const override { return width_; }
    int getHeight() const override { return height_; }
};

// Proxy——虚代理，延迟加载
class ImageProxy : public Image {
private:
    std::string filename_;
    mutable std::unique_ptr<RealImage> realImage_;  // mutable 允许在 const 方法中修改

public:
    explicit ImageProxy(const std::string& filename) : filename_(filename) {}

    void display() const override {
        // 第一次使用时才加载真实图片
        if (!realImage_) {
            std::cout << "[Proxy] Lazy loading: " << filename_ << "\n";
            realImage_ = std::unique_ptr<RealImage>(new RealImage(filename_));
        }
        realImage_->display();
    }

    int getWidth() const override {
        if (!realImage_) {
            realImage_ = std::unique_ptr<RealImage>(new RealImage(filename_));
        }
        return realImage_->getWidth();
    }

    int getHeight() const override {
        if (!realImage_) {
            realImage_ = std::unique_ptr<RealImage>(new RealImage(filename_));
        }
        return realImage_->getHeight();
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 创建代理对象（此时不加载真实图片）
    std::unique_ptr<Image> image = std::unique_ptr<ImageProxy>(new ImageProxy("photo.jpg"));

    std::cout << "Image created, not loaded yet.\n";

    // 使用时才加载
    image->display();
    // Output:
    // [Proxy] Lazy loading: photo.jpg
    // Loading high-resolution image: photo.jpg
    // Displaying: photo.jpg (1920x1080)

    // 再次使用，不重新加载
    image->display();
    // Output:
    // Displaying: photo.jpg (1920x1080)
}
```

---

#### 保护代理（权限控制）

```cpp
#include <iostream>
#include <string>

// Subject 接口
class SensitiveResource {
public:
    virtual ~SensitiveResource() = default;
    virtual void access() const = 0;
};

// RealSubject——真实敏感资源
class RealSensitiveResource : public SensitiveResource {
public:
    void access() const override {
        std::cout << "Accessing CONFIDENTIAL data...\n";
    }
};

// Proxy——保护代理，权限检查
class ResourceProxy : public SensitiveResource {
private:
    std::unique_ptr<SensitiveResource> realResource_;
    std::string userRole_;

    bool hasPermission() const {
        return userRole_ == "admin" || userRole_ == "manager";
    }

public:
    explicit ResourceProxy(const std::string& role)
        : userRole_(role), realResource_(std::unique_ptr<SensitiveResource>(new RealSensitiveResource())) {}

    void access() const override {
        if (!hasPermission()) {
            std::cout << "Access denied! Requires admin or manager role.\n";
            return;
        }
        std::cout << "[Audit] User role '" << userRole_ << "' granted access.\n";
        realResource_->access();
    }
};
```

**客户端代码**：

```cpp
int main() {
    std::unique_ptr<SensitiveResource> userAccess =
        std::unique_ptr<ResourceProxy>(new ResourceProxy("user"));
    std::unique_ptr<SensitiveResource> adminAccess =
        std::unique_ptr<ResourceProxy>(new ResourceProxy("admin"));

    userAccess->access();
    // Output: Access denied! Requires admin or manager role.

    adminAccess->access();
    // Output:
    // [Audit] User role 'admin' granted access.
    // Accessing CONFIDENTIAL data...
}
```

---

### 代理 vs 装饰器

| 维度 | 代理模式 | 装饰器模式 |
|------|---------|-----------|
| **目的** | 控制访问（保护、延迟加载） | 添加功能（增强行为） |
| **替身 vs 包装** | 替身，不改变被代理对象 | 包装，改变被包装对象 |
| **客户端感知** | 客户端可能不知道代理存在 | 客户端知道在使用装饰器 |
| **对象创建时机** | 虚代理延迟创建 | 装饰器立即组合 |

---

### 错误用法

```cpp
// 错误 1：代理类泄露了真实对象的引用
class LeakyProxy : public Image {
    RealImage* realImage_;  // 直接暴露！
public:
    RealImage* getRealImage() { return realImage_; }  // 危险！
};

// 错误 2：代理类添加了被代理接口没有的功能
class OverreachProxy : public Image {
    RealImage* realImage_;
public:
    void download() {  // Image 接口没有 download！这是扩展，不是代理
        realImage_->download();
    }
};

// 错误 3：虚代理没有正确处理 nullptr
class ForgetfulProxy : public Image {
    mutable RealImage* realImage_ = nullptr;  // 裸指针危险
public:
    void display() const override {
        // 如果 realImage_ 为 nullptr 会崩溃
        realImage_->display();
    }
};
```

---

### 面试热点

```
Q: 代理模式和装饰器模式的核心区别？
A: 代理控制"是否允许访问原对象"。
   装饰器控制"如何在访问时添加新行为"。
   两者结构相似，但意图不同。

Q: shared_ptr 是代理模式吗？
A: 可以认为是"智能引用代理"——它控制了指针的访问，
   提供引用计数、自动删除等功能。

Q: 远程代理的实现难点是什么？
A: 序列化/反序列化（将对象转换为字节流）、
   网络通信（Stub/Skeleton 机制）、异常处理。
```

---

## 3.4 Bridge（桥接模式）

### 定义

> 将抽象部分与它的实现部分分离，使它们都可以独立地变化。

### 现实类比

就像**遥控器与电器**——遥控器（抽象）可以控制电视、空调、音箱（实现）。遥控器和电器是两个独立变化的维度：
- 可以有 TV 遥控器、AC 遥控器
- 遥控器可以是普通遥控器或智能遥控器

两者独立变化，互不影响。

### 问题场景

- 跨平台应用（Windows/Linux/Mac × 32bit/64bit）
- 多维度变化的消息系统（文本/图片/视频 × 普通/紧急/系统）
- 设备与驱动（不同设备类型 × 不同操作系统）

### 实现

#### 实现部分（独立变化）

```cpp
#include <iostream>
#include <string>
#include <memory>

// 实现接口——设备驱动层
class Device {
public:
    virtual ~Device() = default;
    virtual void powerOn() = 0;
    virtual void powerOff() = 0;
    virtual void setVolume(int level) = 0;
    virtual int getVolume() const = 0;
};

// 具体实现 A：电视
class TV : public Device {
private:
    int volume_ = 0;
    bool powered_ = false;

public:
    void powerOn() override {
        powered_ = true;
        std::cout << "TV powered on\n";
    }

    void powerOff() override {
        powered_ = false;
        std::cout << "TV powered off\n";
    }

    void setVolume(int level) override {
        volume_ = std::max(0, std::min(100, level));
        std::cout << "TV volume set to " << volume_ << "\n";
    }

    int getVolume() const override { return volume_; }
};

// 具体实现 B：音响
class SoundSystem : public Device {
private:
    int volume_ = 0;
    bool powered_ = false;

public:
    void powerOn() override {
        powered_ = true;
        std::cout << "Sound system powered on\n";
    }

    void powerOff() override {
        powered_ = false;
        std::cout << "Sound system powered off\n";
    }

    void setVolume(int level) override {
        volume_ = std::max(0, std::min(100, level));
        std::cout << "Sound system volume set to " << volume_ << "\n";
    }

    int getVolume() const override { return volume_; }
};
```

#### 抽象部分（独立变化）

```cpp
// 抽象层——遥控器
class RemoteControl {
protected:
    std::unique_ptr<Device> device_;  // 桥接：指向实现

public:
    explicit RemoteControl(std::unique_ptr<Device> device)
        : device_(std::move(device)) {}

    virtual ~RemoteControl() = default;

    void power() {
        if (device_->getVolume() == 0) {
            device_->powerOn();
        } else {
            device_->powerOff();
        }
    }

    void volumeUp() {
        device_->setVolume(device_->getVolume() + 10);
    }

    void volumeDown() {
        device_->setVolume(device_->getVolume() - 10);
    }
};

// 扩展抽象——高级遥控器（带显示屏）
class AdvancedRemoteControl : public RemoteControl {
private:
    int displayBrightness_ = 50;

public:
    using RemoteControl::RemoteControl;

    void setDisplayBrightness(int level) {
        displayBrightness_ = level;
        std::cout << "Display brightness: " << displayBrightness_ << "\n";
    }

    void mute() {
        device_->setVolume(0);
        std::cout << "Muted\n";
    }
};
```

**客户端代码**：

```cpp
int main() {
    // 组合 1：TV + 普通遥控器
    std::unique_ptr<RemoteControl> tvRemote =
        std::unique_ptr<RemoteControl>(new RemoteControl(std::unique_ptr<Device>(new TV())));
    tvRemote->power();
    tvRemote->volumeUp();
    // Output:
    // TV powered on
    // TV volume set to 10

    // 组合 2：SoundSystem + 高级遥控器
    std::unique_ptr<AdvancedRemoteControl> soundRemote =
        std::unique_ptr<AdvancedRemoteControl>(
            new AdvancedRemoteControl(std::unique_ptr<Device>(new SoundSystem())));
    soundRemote->power();
    soundRemote->volumeUp();
    soundRemote->mute();
    // Output:
    // Sound system powered on
    // Sound system volume set to 10
    // Muted

    // 组合 3：TV + 高级遥控器
    std::unique_ptr<AdvancedRemoteControl> tvAdvancedRemote =
        std::unique_ptr<AdvancedRemoteControl>(new AdvancedRemoteControl(std::unique_ptr<Device>(new TV())));
    tvAdvancedRemote->power();
    tvAdvancedRemote->setDisplayBrightness(80);
    // Output:
    // TV powered on
    // Display brightness: 80
}
```

---

### 桥接 vs 装饰器 vs 适配器

| 模式 | 解决的问题 | 变化维度 |
|------|---------|---------|
| **Bridge** | 两个维度独立变化 | 抽象和实现可以独立扩展 |
| **Decorator** | 动态添加职责 | 层层叠加，不限制维度 |
| **Adapter** | 接口不兼容 | 把一个接口变成另一个 |

---

### 错误用法

```cpp
// 错误 1：滥用桥接——不需要两个维度变化时强行分离
class OverEngineered {
    class Implementation {};
    class Abstraction {};  // 过度设计！
};

// 错误 2：桥接方向反了——抽象持有实现的引用，但实现反过来持有抽象
class ConfusedImpl : public Device {
    RemoteControl* remote_;  // 错误！这会导致循环引用
};

// 错误 3：把桥接变成继承——两个维度都用继承实现
class TVRemoteControl : public RemoteControl, public TV {};  // 多继承！复杂
```

---

### 面试热点

```
Q: 桥接模式的核心思想是什么？
A: "发现"并"分离"两个独立变化的维度，
   让它们可以独立扩展，而不是组合爆炸。

Q: 桥接模式和装饰器模式的区别？
A: 桥接：两个维度分离（抽象 × 实现），可以独立扩展
   装饰器：只有一个维度变化，层层叠加

Q: 什么情况下应该用桥接模式？
A: 当你发现类爆炸（n×m 个类）时，
   考虑是否有两个独立维度可以分离。
```

---

## 3.5 本课总结

### 三种模式的对比

| 模式 | 核心问题 | 解决方案 | 关键词 |
|------|---------|---------|--------|
| **Composite** | 部分与整体一致性 | 树形结构，递归组合 | "递归" |
| **Proxy** | 访问控制 | 替身对象，延迟/保护/远程 | "替身" |
| **Bridge** | 双维度独立变化 | 抽象与实现分离，各自扩展 | "分离" |

### 统一思想

三种模式都在解决**"如何应对复杂结构的扩展"**：

- Composite：**递归组合**——把整体和部分统一对待
- Proxy：**间接访问**——通过替身控制对原对象的访问
- Bridge：**分离变化**——把两个独立维度拆分开来

### 结构型模式全景

```
一对一组合          →  一对多组合        →  间接访问
Adapter             Composite           Proxy
Decorator           Bridge
Facade
```

---

## 3.6 课后练习

1. **Composite**：实现一个"公司组织架构"系统：
   - `Employee` 是基础组件（只有姓名和职位）
   - `Manager` 可以管理其他 Employee（下属列表）
   - 支持 `displayHierarchy()` 显示层级结构
   - 支持 `getSalary()` 返回自己或整个团队的工资总和

2. **Proxy**：实现一个"文档编辑器"虚代理：
   - `Document` 是真实文档（可能很大）
   - `DocumentProxy` 在显示时才加载文档内容
   - 支持预览模式（只加载前 100 个字符）

3. **Bridge**：实现一个"消息发送系统"：
   - 实现维度：`EmailSender`、`SMSSender`、`PushSender`
   - 抽象维度：`NormalMessage`、`UrgentMessage`、`SystemMessage`
   - 两个维度可以自由组合，客户端只需调用 `send(message)`
