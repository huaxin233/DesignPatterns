#include <iostream>
#include <memory>
#include <pthread.h>
#include <string>
#include <type_traits>

// 产品抽象基类
class Product {

public:
    virtual ~Product() = default;
    virtual void use() const = 0;
};

// 具体产品A
class ConcreteProductA : public Product {

public:
    void use() const override {
        std::cout << "Using ConcreteProductA\n";
    }
};

// 具体产品B
class ConcreteProductB : public Product {

public:
    void use() const override {
        std::cout << "Using ConcreteProductB\n";
    }
};

// 创建者抽象类
class Creator {

public:
    virtual ~Creator() = default;

    // 工厂方法--子类决定创建什么产品
    virtual std::unique_ptr<Product> factoryMethod() const = 0;

    // 使用产品的模板逻辑
    void doSomething() const {
        auto product = factoryMethod();
        product->use();
    }
};

// 具体创建者 A--创建产品A
class ConcreteCreatorA : public Creator {

public:
    std::unique_ptr<Product> factoryMethod() const override {
        return std::make_unique<ConcreteProductA>();
    }
};

// 具体创建者 B--创建产品B
class ConcreteCreatorB : public Creator {

public:
    std::unique_ptr<Product> factoryMethod() const override {
        return std::make_unique<ConcreteProductB>();
    }
};

int main()
{
    std::unique_ptr<Creator> creator;

    // 根据配置或条件选择创建者
    std::string type = "A";
    if(type == "A") {
        creator = std::make_unique<ConcreteCreatorA>();
    } else {
        creator = std::make_unique<ConcreteCreatorB>();
    }

    creator->doSomething();

    return 0;
}

