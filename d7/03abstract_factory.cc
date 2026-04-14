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

// windows产品族
class WindowsButton : public Button {
public:
  void render() const override { std::cout << "[Windows Button] rendered\n"; }
  void onClick() const override { std::cout << "[Windows Button] clicked\n"; }
};

class WindowsTextBox : public TextBox {
public:
  void render() const override { std::cout << "[Windows TextBox] rendered\n"; }
  void setText(const std::string &text) override {
    std::cout << "[Windows TextBox] text set to: " << text << "\n";
  }
};

class WindowsCheckbox : public Checkbox {
public:
  void render() const override { std::cout << "[Windows Checkbox] rendered\n"; }
  void toggle() override { std::cout << "[Windows Checkbox] toggled\n"; }
};

// mac产品族
class MacButton: public Button {
public:
  void render() const override { std::cout << "[Mac Button] rendered\n"; }
  void onClick() const override { std::cout << "[Mac Button] clicked\n"; }
};

class MacTextBox : public TextBox {
public:
  void render() const override { std::cout << "[Mac TextBox] rendered\n"; }
  void setText(const std::string &text) override {
    std::cout << "[Mac TextBox] text set to: " << text << "\n";
  }
};

class MacCheckbox : public Checkbox {
public:
  void render() const override { std::cout << "[Mac Checkbox] rendered\n"; }
  void toggle() override { std::cout << "[Mac Checkbox] toggled\n"; }
};

// 抽象工厂--创建一整族 UI 组件
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
    return std::unique_ptr<WindowsButton>(new WindowsButton());
  }

  std::unique_ptr<TextBox> createTextBox() const override {
    return std::unique_ptr<WindowsTextBox>(new WindowsTextBox());
  }

  std::unique_ptr<Checkbox> createCheckbox() const override {
    return std::unique_ptr<WindowsCheckbox>(new WindowsCheckbox());
  }
};


// 具体工厂 A：Mac UI 工厂
class MacUIFactory : public UIFactory {
public:
  std::unique_ptr<Button> createButton() const override {
    return std::unique_ptr<MacButton>(new MacButton());
  }

  std::unique_ptr<TextBox> createTextBox() const override {
    return std::unique_ptr<MacTextBox>(new MacTextBox());
  }

  std::unique_ptr<Checkbox> createCheckbox() const override {
    return std::unique_ptr<MacCheckbox>(new MacCheckbox());
  }
};

// 应用--只依赖抽象工厂和抽象产品
class Application {
private:
  std::unique_ptr<Button> button_;
  std::unique_ptr<TextBox> textBox_;
  std::unique_ptr<Checkbox> checkbox_;

public:
  explicit Application(const UIFactory &factory) {
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
    textBox_->setText("Hello world");
    checkbox_->toggle();
  }
};

int main() {
  // 根据平台选择工厂--整个应用只需改这一行
  std::unique_ptr<UIFactory> factory;

#ifdef _WIN32
  factory = std::unique_ptr<WindowsUIFactory>(new WindowsUIFactory());
#else
  factory = std::unique_ptr<MacUIFactory>(new MacUIFactory());
#endif

  Application app(*factory);
  app.renderUI();
  app.interact();

  return 0;
}
