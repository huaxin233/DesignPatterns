#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

// 产品层次结构
// 抽象产品--所有运输工具的统一接口
class Transport {
public:
    virtual ~Transport() = default;
    virtual void deliver(const std::string& cargo) const = 0;
    virtual double getCostPerKg() const = 0;
};

// 具体产品 A：卡车
class Truck : public Transport {
public:
    void deliver(const std::string& cargo) const override {
        std::cout << "Truck delivering [" << cargo << "] by road\n";
    }

    double getCostPerKg() const override {return 1.5;}
};

// 具体产品 B： 轮船
class Ship : public Transport {
public:
    void deliver(const std::string& carge) const override {
        std::cout << "Ship delivering [" << carge << "] by sea\n";
    }

    double getCostPerKg() const override {return 0.8;}
};

// 具体产品 C： 飞机
class Airplane : public Transport {
    void deliver(const std::string& carge) const override {
        std::cout << "Airplane delivering [" << carge << "] by air\n";
    }

    double getCostPerKg() const override {return 5.0;}
};

// 工厂层次结构
// 抽象工厂（Creator）--定义工厂方法
class LogisticsCompany {
public:
    virtual ~LogisticsCompany() = default;

    // 工厂方法--由子类实现，决定创建哪种运输工具
    virtual std::unique_ptr<Transport> createTransport() const = 0;

    // 业务逻辑--使用工厂方法创建的产品，不依赖具体产品类型
    void planDelivery(const std::string& cargo, double weight) const {
        auto transport = createTransport(); // 调用工厂方法
        double cost = transport->getCostPerKg() * weight;
        std::cout << "Cost estimate: $" << cost << "\n";
        transport->deliver(cargo);
    }
};

// 具体工厂 A：陆路物流
class RoadLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Truck());
    }
};

// 具体工厂 B：海运物流
class SeaLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Ship());
    }
};

// 具体工厂 C：空运物流
class AirLogistics : public LogisticsCompany {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::unique_ptr<Transport>(new Airplane());
    }
};

// 客户端代码--只依赖抽象接口，不知道具体产品类型
void processOrder(const LogisticsCompany& company, const std::string& cargo, double weight) {
    company.planDelivery(cargo, weight);
}

int main() {
    RoadLogistics road;
    SeaLogistics sea;
    AirLogistics air;

    std::cout << "=== Road delivery ===\n";
    processOrder(road, "Electronics", 100);

    std::cout << "\n=== Sea delivery ===\n";
    processOrder(sea, "Furniture", 500);

    std::cout << "\n=== Air delivery ===\n";
    processOrder(air, "Medicine", 10);

    return 0;
}
