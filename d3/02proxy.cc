#include <iostream>
#include <string>
#include <memory>

// Subject 接口--客户端看到的统一接口
class Image {
public:
  virtual ~Image() = default;
  virtual void display() const = 0;
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
};

// RealSubject--真实图片（可能很大）
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
  explicit RealImage(const std::string &filename) : filename_(filename) {
    loadFromDisk(); // 构造时直接加载
  }

  void display() const override {
    std::cout << "Displaying: " << filename_ << " (" << width_ << "x" << height_
              << ")\n";
  }

  int getWidth() const override { return width_; }
  int getHeight() const override { return height_; }
};

// Proxy--虚代理，延迟加载
class ImageProxy : public Image {
private:
  std::string filename_;
  mutable std::unique_ptr<RealImage> realImage_; // mutable允许在const方法中修改

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

int main() {

  // 创建代理对象（此时不加载真实图片）
  std::unique_ptr<Image> image =
    std::unique_ptr<ImageProxy>(new ImageProxy("photo.jpg"));

  std::cout << "Image created, not loaded yet.\n";

  // 使用时才加载
  image->display();

  // 再次使用，不重新加载
  image->display();

  return 0;
}
