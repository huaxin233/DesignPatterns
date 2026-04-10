#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

// 前向声明
class VendingMachine;

// 状态抽象基类
class VendingState {
public:
  virtual ~VendingState() = default;
  virtual void insertCoin(VendingMachine &machine) = 0;
  virtual void pressButton(VendingMachine &machine) = 0;
  virtual void dispense(VendingMachine &machine) = 0;
  virtual std::string name() const = 0;
};

// 上下文--自动售货机
class VendingMachine {
private:
  std::unique_ptr<VendingState> state_;
  int stock_;

public:
  explicit VendingMachine(int stock); // 定义在所有状态类之后

  void setState(std::unique_ptr<VendingState> state) {
    std::cout << "State: " << state_->name() << " -> " << state->name()
              << "\n";
    state_ = std::move(state);
  }

  void insertCoin() { state_->insertCoin(*this); }
  void pressButton() { state_->pressButton(*this); }
  void dispense() { state_->dispense(*this); }

  int getStock() const { return stock_; }
  void reduceStock() { --stock_; }
};

// 具体状态：空闲（等待投币）
class IdleState : public VendingState {
public:
  void insertCoin(VendingMachine &machine) override; // 定义在后面
  void pressButton(VendingMachine &) override {
    std::cout << "Please insert a coin first.\n";
  }
  void dispense(VendingMachine &) override {
    std::cout << "Please insert a coin first.\n";
  }
  std::string name() const override { return "Idle"; }
};

// 具体状态：已投币
class HasCoinState : public VendingState {
public:
  void insertCoin(VendingMachine &) override {
    std::cout << "Coin already inserted.\n";
  }
  void pressButton(VendingMachine &) override; // 定义在后面
  void dispense(VendingMachine &) override {
    std::cout << "Press the button first.\n";
  }
  std::string name() const override { return "HasCoin"; }
};

// 具体状态：出货中
class DispensingState : public VendingState {
public:
  void insertCoin(VendingMachine &) override {
    std::cout << "Please wait, dispensing...\n";
  }
  void pressButton(VendingMachine &) override {
    std::cout << "Already dispensing...\n";
  }
  void dispense(VendingMachine &) override; // 定义在后面
  std::string name() const override { return "Dispensing"; }
};

// 具体状态：缺货
class SoldOutState : public VendingState {
public:
  void insertCoin(VendingMachine &) override {
    std::cout << "Sorry, sold out. Returning coin.\n";
  }
  void pressButton(VendingMachine &) override {
    std::cout << "Sorry, sold out.\n";
  }
  void dispense(VendingMachine &) override {
    std::cout << "Nothing to dispense.\n";
  }
  std::string name() const override { return "SoldOut";}
};

// 延迟定义--因为需要创建其他状态对象
void IdleState::insertCoin(VendingMachine &machine) {
  std::cout << "Coin inserted.\n";
  machine.setState(std::unique_ptr<HasCoinState>(new HasCoinState()));
}

void HasCoinState::pressButton(VendingMachine &machine) {
  std::cout << "Button pressed. Dispensing...\n";
  machine.setState(std::unique_ptr<DispensingState>(new DispensingState()));
  machine.dispense(); // 触发出货
}

void DispensingState::dispense(VendingMachine &machine) {
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

int main() {
  VendingMachine machine(2); // 库存2个

  // 正常购买
  machine.insertCoin();
  machine.pressButton();

  // 未投币就按按钮
  machine.pressButton();

  // 再次购买
  machine.insertCoin();
  machine.pressButton();

  // 缺货
  machine.insertCoin();

  return 0;
}
