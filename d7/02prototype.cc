#include <cstdint>
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
  Goblin(const std::string &weapon) : weapon_(weapon) {
    name = "Goblin";
    health = 50;
    attack = 8;
    defense = 3;
  }

  std::unique_ptr<Monster> clone() const override {
    // 拷贝构造：复制所有字段
    return std::unique_ptr<Goblin>(new Goblin(*this));
  }

  void display() const override {
    std::cout << name << " [HP:" << health << " ATK:" << attack
              << " DEF:" << defense << " Weapon:" << weapon_ << "]\n";
  }
};

// 具体原型 B：龙
class Dragon : public Monster {
private:
  std::string element_;
  int wingspan_;

public:
  Dragon(const std::string &element, int wingspan)
      : element_(element), wingspan_(wingspan) {
    name = "Dragon";
    health = 500;
    attack = 50;
    defense = 30;
  }

  std::unique_ptr<Monster> clone() const override {
    return std::unique_ptr<Dragon>(new Dragon(*this));
  }

  void display() const override {
    std::cout << name << " [HP" << health << " ATK:" << attack
              << " DEF:" << defense << " Element:" << element_
              << " Wingspan:" << wingspan_ << "m]\n";
  }
};

// int main() {
//   // 创建原型
//   Goblin goblinPrototype("Short Sowrd");
//   Dragon dragonPrototype("Fire", 20);

//   // 克隆并微调
//   auto goblin1 = goblinPrototype.clone();
//   goblin1->name = "Goblin Scout";
//   goblin1->health = 40;

//   auto goblin2 = goblinPrototype.clone();
//   goblin2->name = "Goblin Chief";
//   goblin2->attack = 15;

//   auto dragon1 = dragonPrototype.clone();
//   dragon1->name = "Elder Dragon";
//   dragon1->health = 800;

//   goblin1->display();
//   goblin2->display();
//   dragon1->display();

//   return 0;
// }

// 原型注册表--集中管理原型模板
class MonsterRegistry {
private:
  std::unordered_map<std::string, std::unique_ptr<Monster>> prototypes_;

public:
  void registerPrototype(const std::string &key,
                         std::unique_ptr<Monster> prototype) {
    prototypes_[key] = std::move(prototype);
  }

  std::unique_ptr<Monster> create(const std::string &key) const {
    auto it = prototypes_.find(key);
    if (it != prototypes_.end()) {
      return it->second->clone();
    }
    return nullptr;
  }
};

int main() {
  MonsterRegistry registry;

  // 注册原型模板
  registry.registerPrototype(
      "goblin", std::unique_ptr<Goblin>(new Goblin("Short Sowrd")));
  registry.registerPrototype("dragon",
                             std::unique_ptr<Dragon>(new Dragon("Fire", 20)));

  // 通过字符串键快速创建
  auto enemy1 = registry.create("goblin");
  enemy1->name = "Goblin Scout";
  enemy1->health = 40;
  auto enemy2 = registry.create("goblin");
  enemy2->name = "Goblin Chief";
  enemy2->attack = 15;
  auto enemy3 = registry.create("dragon");
  enemy3->name = "Elder Dragon";
  enemy3->health = 800;

  enemy1->display();
  enemy2->display();
  enemy3->display();

  return 0;
}
