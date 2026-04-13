#include <algorithm>
#include <iostream>
#include <iterator>
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
  std::unique_ptr<Approver> next_; // 下一个处理者
  std::string name_;

public:
  explicit Approver(const std::string &name) : name_(name) {}
  virtual ~Approver() = default;

  // 设置下一个处理者，返回下一个处理者的引用（支持链式调用）
  Approver &setNext(std::unique_ptr<Approver> next) {
    next_ = std::move(next);
    return *next_;
  }

  // 处理请求--子类实现具体逻辑
  virtual void handle(const ExpenseRequest& request) = 0;

protected:
  // 传递给下一个处理者
  void passNext(const ExpenseRequest &request) {
    if (next_) {
      next_->handle(request);
    } else {
      std::cout << "No one can approve: " << request.description << " ($"
                << request.amount << ")\n";
    }
  }
};

// 具体处理者
// 组长：审批 < 500
class TeamLead : public Approver {
public:
  using Approver::Approver;

  void handle(const ExpenseRequest &request) override {
    if (request.amount < 500) {
      std::cout << name_ << " approved: " << request.description << " ($"
                << request.amount << " )\n";
    } else {
      passNext(request);
    }
   }
};

// 经理：审批 < 5000
class Manager : public Approver {
public:
  using Approver::Approver;

  void handle(const ExpenseRequest &request) override {
    if (request.amount < 5000) {
      std::cout << name_ << " approved: " << request.description << " ($"
                << request.amount << ")\n";
    } else {
      passNext(request);
    }
  }
};

// 总监：审批 < 50000
class Director : public Approver {
public:
  using Approver::Approver;

  void handle(const ExpenseRequest &request) override {
    if (request.amount < 50000) {
      std::cout << name_ << " approved: " << request.description << " ($" << request.amount << ")\n";
    } else {
      passNext(request);
    }
  }
};

// CEO：审批任意金额
class CEO : public Approver {
public:
  using Approver::Approver;

  void handle(const ExpenseRequest &request) override {
    std::cout << name_ << " approved: " << request.description << " ($"
              << request.amount << ")\n";
  }
};

int main() {
  // 构建责任链：组长 -> 经理 -> 总监 -> CEO
  auto chain = std::unique_ptr<TeamLead>(new TeamLead("TeamLead Mao"));
  auto &manager =
      chain->setNext(std::unique_ptr<Manager>(new Manager("Manager Li")));
  auto &director = manager.setNext(
      std::unique_ptr<Director>(new Director("Director Wang")));
  director.setNext(std::unique_ptr<CEO>(new CEO("CEO Chen")));

  // 提交不同金额的报销
  chain->handle({"Office supplies", 200, "Alice"});

  chain->handle({"Team dinner", 300, "Bob"});

  chain->handle({"Conference travel", 15000, "Carol"});

  chain->handle({"New server cluster", 80000, "Dave"});

  return 0;
}
