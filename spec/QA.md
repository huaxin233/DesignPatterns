# Q&A 集

---

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
```

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

---

## 成员函数后的 const

```
Q: 成员函数声明末尾的 const 有什么作用？
A: 承诺该函数不会修改对象的任何成员变量。
   编译器会强制检查，函数体内修改成员变量会编译报错。
   this 指针类型变为 const ClassName*。

Q: 为什么 getName() 要标记 const 而 update() 不用？
A: getName() 只读取成员、不修改状态，应标记 const。
   update() 的语义是"接收通知后更新自身状态"，
   通常需要修改成员变量，所以不标记 const。

Q: const 成员函数的实际影响？
A: 1) 函数体内不能修改成员变量（除非成员是 mutable）
   2) const 对象/引用只能调用 const 成员函数
   3) 经验法则：不修改状态的函数都应标记 const（const correctness）
```

---

## 单例返回指针还是引用

```
Q: getInstance() 返回指针还是引用更好？
A: 返回引用更好。
   1) 语义：引用表示"一定存在"，指针暗示"可能为 null"，单例不可能不存在
   2) 安全：返回指针时调用者可能 delete 它，摧毁单例
   3) 简洁：config.get("key") 比 config->get("key") 更自然

Q: 如何比较两个引用是否指向同一对象？
A: 取地址比较：&config1 == &config2
   引用没有重载 operator== 时不能直接用 config1 == config2，
   那比较的是对象内容（需要自定义 operator==），而不是身份。
```
