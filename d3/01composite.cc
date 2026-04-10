#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// 组件抽象基类--文件和文件夹的统一接口
class FileSystemComponent {
public:
  virtual ~FileSystemComponent() = default;

  // 统一接口：显示名称
  virtual void display(int indent = 0) const = 0;

  // 统一接口：获取大小
  virtual int getSize() const = 0;

  // 统一接口：获取名称
  virtual const std::string& getName() const = 0;
};

// 叶节点--具体文件
class File : public FileSystemComponent {
private:
  std::string name_;
  int size_;

public:
  File(const std::string& name, int size) : name_(name), size_(size) {}

  void display(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "file " << name_ << " (" << size_
              << " KB\n";
  }

  int getSize() const override {
    return size_;
  }

  const std::string& getName() const override {
    return name_;
  }
};

// 组合节点--文件夹，可以包含子组件
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
  void remove(const std::string &name) {
    children_.erase(
        std::remove_if(children_.begin(), children_.end(),
                       [&name](const std::unique_ptr<FileSystemComponent> &c) {
                         return c->getName() == name;
                       }),
        children_.end());
  }

  void display(int indent = 0) const override {
    std::cout << std::string(indent, ' ') << "Folder " << name_ << " ("
              << getSize() << " KB\n";
    // 递归显示所有子组件
    for (const auto &child : children_) {
      child->display(indent + 4);
    }
  }

  int getSize() const override {
    int total = 0;
    for (const auto &child : children_) {
      total += child->getSize();
    }
    return total;
  }

  // 辅助方法：获取名称（用于remove）
  const std::string& getName() const { return name_; }
};

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

  // 统一接口调用--无需知道是文件还是文件夹
  root.display();

  std::cout << "Total size: " << root.getSize() << " KB\n";

  return 0;
}

