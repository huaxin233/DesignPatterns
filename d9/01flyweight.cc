#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

// Flyweight--共享的粒子类型（内在状态）
class ParticleType {
private:
  std::string name_;
  std::string color_;
  std::string texture_; // 假设纹理数据很大（几MB）
  double baseSize_;

public:
  ParticleType(const std::string &name, const std::string &color,
               const std::string &texture, double baseSize)
      : name_(name), color_(color), texture_(texture), baseSize_(baseSize) {}

  const std::string& getName() const { return name_; }
  const std::string& getColor() const { return color_; }
  double getBaseSize() const { return baseSize_; }

  void render(double x, double y, double speed, double direction) const {
    std::cout << "  [" << name_ << "] color=" << color_ << " size=" << baseSize_
              << " at (" << x << ", " << y << ")" << " speed=" << speed
              << " dir=" << direction << "\n";
  }
};

// Flyweight Factory--管理共享对象的创建和缓存
class ParticleTypeFactory {
private:
  std::unordered_map<std::string, std::shared_ptr<ParticleType>> types_;

public:
  std::shared_ptr<ParticleType> getType(const std::string &name,
                                        const std::string &color,
                                        const std::string &texture,
                                        double baseSize) {
    std::string key = name + "_" + color;
    auto it = types_.find(key);
    if (it != types_.end()) {
      return it->second;
    }

    auto type = std::make_shared<ParticleType>(name, color, texture, baseSize);
    types_[key] = type;
    std::cout << "  [Factory] Created new ParticleType: " << key << "\n";
    return type;
  }

  size_t typeCount() const { return types_.size(); }
};

// 每个粒子实例--只保存外在状态 + 指向共享 Flyweight 的指针
struct Particle {
  std::shared_ptr<ParticleType> type; // 指向共享的内在状态
  double x, y;                        // 外在状态：位置
  double speed;                       // 外在状态：速度
  double direction;                   // 外在状态：方向（度）

  void render() const {
    type->render(x, y, speed, direction);
  }

  void update(double dt) {
    double rad = direction * 3.14159 / 180.0;
    x += speed * std::cos(rad) * dt;
    y += speed * std::sin(rad) * dt;
  }
};

class ParticleSystem {
private:
  ParticleTypeFactory factory_;
  std::vector<Particle> particles_;

public:
  void addExplosion(double x, double y, int count) {
    auto fireType = factory_.getType("fire", "orange", "fire_texture_2MB", 3.0);
    auto smokeType =
        factory_.getType("smoke", "gray", "smoke_texture_4MB", 5.0);
    auto sparkType =
        factory_.getType("saprk", "yellow", "spark_texture_1MB", 1.0);

    for (int i = 0; i < count; ++i) {
      double angle = 360.0 * i / count;
      double spd = 50.0 + (i % 3) * 20.0;

      if (i % 3 == 0) {
        particles_.push_back({fireType, x, y, spd, angle});
      } else if (i % 3 == 1) {
        particles_.push_back({smokeType, x, y, spd * 0.5, angle});
      } else
        particles_.push_back({sparkType, x, y, spd * 1.5, angle});
    }
  }

  void render() const {
    std::cout << "Rendering " << particles_.size() << " particles:\n";
    for (const auto &p : particles_) {
      p.render();
    }
  }

  void printMemoryStats() const {
    std::cout << "\n=== Memory Statistics ===\n"
              << "Particle instances: " << particles_.size() << "\n"
              << "Unique ParticleTypes: " << factory_.typeCount() << "\n"
              << "Per-particle cost: ~" << sizeof(Particle) << " bytes "
              << "(without Flyweight: ~" << sizeof(Particle) + 200
              << "+ bytes)\n"
              << "Shared texture data: only " << factory_.typeCount()
              << " copies (not " << particles_.size() << ")\n";
  }
};

int main() {
  ParticleSystem system;

  // 两次爆炸，共24个粒子，但只有3中ParticleType
  system.addExplosion(100.0, 200.0, 12);
  system.addExplosion(500.0, 300.0, 12);

  system.render();
  system.printMemoryStats();

  return 0;
}

