#include <csetjmp>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Context--存储变量绑定
class PermissionContext {
private:
  std::unordered_set<std::string> roles_;
  std::unordered_map<std::string, int> attributes_;

public:
  void addRole(const std::string& role) { roles_.insert(role); }
  bool hasRole(const std::string &role) const {
    return roles_.count(role) > 0;
  }

  void setAttribute(const std::string& key, int value) {
    attributes_[key] = value;
  }

  int getAttribute(const std::string &key) const {
    auto it = attributes_.find(key);
    return it != attributes_.end() ? it->second : 0;
  }
};

// 抽象表达式--所有节点的统一接口
class BoolExpression {
public:
public:
  virtual ~BoolExpression() = default;
  virtual bool interpret(const PermissionContext &ctx) const = 0;
  virtual std::string toString() const = 0;
};

// 终结符：角色检查
class RoleExpression : public BoolExpression {
private:
  std::string role_;

public:
  explicit RoleExpression(const std::string& role) : role_(role) {}

  bool interpret(const PermissionContext &ctx) const override {
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
  AttributeExpression(const std::string &key, int threshold)
      : key_(key), threshold_(threshold) {}

  bool interpret(const PermissionContext &ctx) const override {
    return ctx.getAttribute(key_) >= threshold_;
  }

  std::string toString() const override {
    return key_ + " >= " + std::to_string(threshold_);
  }
};

// 非终结符表达式：AND组合
class AndExpression : public BoolExpression {
private:
  std::unique_ptr<BoolExpression> left_;
  std::unique_ptr<BoolExpression> right_;

public:
  AndExpression(std::unique_ptr<BoolExpression> left,
                std::unique_ptr<BoolExpression> right)
    : left_(std::move(left)), right_(std::move(right)) {}

  bool interpret(const PermissionContext &ctx) const override {
    return left_->interpret(ctx) && right_->interpret(ctx);
  }

  std::string toString() const override {
    return "(" + left_->toString() + " AND " + right_->toString() + ")";
  }
};

// 非终结符：OR组合
class OrExpression : public BoolExpression {
private:
  std::unique_ptr<BoolExpression> left_;
  std::unique_ptr<BoolExpression> right_;

public:
  OrExpression(std::unique_ptr<BoolExpression> left,
               std::unique_ptr<BoolExpression> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  bool interpret(const PermissionContext &ctx) const override {
    return left_->interpret(ctx) || right_->interpret(ctx);
  }

  std::string toString() const override {
    return "(" + left_->toString() + " OR " + right_->toString() + ")";
  }
};

// 非终结符：NOT取反
class NotExpression : public BoolExpression {
private:
  std::unique_ptr<BoolExpression> expr_;

public:
  explicit NotExpression(std::unique_ptr<BoolExpression> expr)
      : expr_(std::move(expr)) {}

  bool interpret(const PermissionContext &ctx) const override {
    return !expr_->interpret(ctx);
  }

  std::string toString() const override {
    return "NOT(" + expr_->toString() + ")";
  }
};

int main() {
  // 构建规则：(hasRole(admin) OR (hasRole(editor)) AND level >= 5)
  auto rule = std::unique_ptr<OrExpression>(new OrExpression(
      std::unique_ptr<RoleExpression>(new RoleExpression("admin")),
      std::unique_ptr<AndExpression>(new AndExpression(
          std::unique_ptr<RoleExpression>(new RoleExpression("editor")),
          std::unique_ptr<AttributeExpression>(
              new AttributeExpression("level", 5))))));

  std::cout << "Role: " << rule->toString() << "\n\n";

  // 测试用户1：管理员
  PermissionContext admin;
  admin.addRole("admin");
  admin.setAttribute("level", 1);
  std::cout << "Admin: " << (rule->interpret(admin) ? "GRANTED" : "DENIED")
            << "\n";

  // 测试用户2：高级编辑
  PermissionContext seniorEditor;
  seniorEditor.addRole("editor");
  seniorEditor.setAttribute("level", 7);
  std::cout << "Senior Editor: " << (rule->interpret(seniorEditor) ? "GRANTED" : "DENIED") << "\n";

  // 测试用户3：初级编辑
  PermissionContext juniorEditor;
  juniorEditor.addRole("editor");
  juniorEditor.setAttribute("level", 2);
  std::cout << "Junior Editor: "
            << (rule->interpret(juniorEditor) ? "GRANTED" : "DENIED") << "\n";

  // 测试用户4：普通用户
  PermissionContext viewer;
  viewer.addRole("viewer");
  viewer.setAttribute("level", 10);
  std::cout << "Viewer: " << (rule->interpret(viewer) ? "GRANTED" : "DENIED")
            << "\n";

  return 0;
}

