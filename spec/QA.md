# 第 2 课：面试考点

## explicit 关键字

```
Q: explicit 的作用是什么？
A: 阻止单参数构造函数的隐式类型转换。
   避免非预期的类型转换，防止意外的对象创建。

Q: 为什么要用 explicit 修饰装饰器的构造函数？
A: 装饰器接收 std::unique_ptr<Coffee>，如果允许隐式转换：
      CoffeeDecorator d = std::make_unique<BasicCoffee>();  // 危险！
   客户端可能无意中丢失 unique_ptr 的所有权。
   explicit 确保必须显式构造：
      CoffeeDecorator d(std::make_unique<BasicCoffee>());  // 明确

---

## using 继承构造函数

```
Q: using CoffeeDecorator::CoffeeDecorator; 的作用是什么？
A: C++11 构造函数继承——让子类继承父类的构造函数。
   MilkDecorator 自动获得 CoffeeDecorator 的构造函数，
   不用手动重写：
      MilkDecorator(std::unique_ptr<Coffee>) 
          : CoffeeDecorator(std::move(coffee)) {}
   等同于手动编写上面的代码。
```
