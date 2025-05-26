# Design Patterns in Software Engineering

A comprehensive guide to the 23 Gang of Four (GoF) design patterns with C++ implementations.

## Table of Contents

### 🏗️ Creational Patterns
1. [Singleton](#singleton)
2. [Factory Method](#factory-method)
3. [Abstract Factory](#abstract-factory)
4. [Builder](#builder)
5. [Prototype](#prototype)

### 🧱 Structural Patterns
6. [Adapter](#adapter)
7. [Bridge](#bridge)
8. [Composite](#composite)
9. [Decorator](#decorator)
10. [Facade](#facade)
11. [Flyweight](#flyweight)
12. [Proxy](#proxy)

### 🔄 Behavioral Patterns
13. [Chain of Responsibility](#chain-of-responsibility)
14. [Command](#command)
15. [Interpreter](#interpreter)
16. [Iterator](#iterator)
17. [Mediator](#mediator)
18. [Memento](#memento)
19. [Observer](#observer)
20. [State](#state)
21. [Strategy](#strategy)
22. [Template Method](#template-method)
23. [Visitor](#visitor)

---

## 🏗️ Creational Patterns

Creational patterns focus on object creation mechanisms, trying to create objects in a manner suitable to the situation.

### Singleton

**Category:** Creational

**Definition:** Ensures a class has only one instance and provides a global point of access to it.

**Real-world Analogy:** Like a government - there can only be one president at a time, and everyone knows how to contact the president's office.

**C++ Implementation:**

```cpp
#include <iostream>
#include <mutex>

class Singleton {
private:
    static Singleton* instance;
    static std::mutex mutex_;
    
    // Private constructor to prevent instantiation
    Singleton() {
        std::cout << "Singleton instance created\n";
    }

public:
    // Delete copy constructor and assignment operator
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    static Singleton* getInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance == nullptr) {
            instance = new Singleton();
        }
        return instance;
    }
    
    void doSomething() {
        std::cout << "Doing something...\n";
    }
};

// Static member definitions
Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mutex_;

// Usage
int main() {
    Singleton* s1 = Singleton::getInstance();
    Singleton* s2 = Singleton::getInstance();
    
    std::cout << "s1 == s2: " << (s1 == s2) << std::endl; // Output: 1 (true)
    
    s1->doSomething();
    return 0;
}
```

**Bad Usage Example:**

```cpp
// DON'T DO THIS - Not thread-safe
class BadSingleton {
private:
    static BadSingleton* instance;
    BadSingleton() {}
    
public:
    static BadSingleton* getInstance() {
        if (instance == nullptr) {  // Race condition!
            instance = new BadSingleton();
        }
        return instance;
    }
};
```

---

### Factory Method

**Category:** Creational

**Definition:** Creates objects without specifying the exact class to create. It defines an interface for creating objects, but lets subclasses decide which class to instantiate.

**Real-world Analogy:** Like a restaurant kitchen - you order "pasta" from the menu, but the kitchen decides whether to make spaghetti, penne, or fettuccine based on availability.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>

// Product interface
class Animal {
public:
    virtual ~Animal() = default;
    virtual void makeSound() = 0;
};

// Concrete products
class Dog : public Animal {
public:
    void makeSound() override {
        std::cout << "Woof! Woof!\n";
    }
};

class Cat : public Animal {
public:
    void makeSound() override {
        std::cout << "Meow! Meow!\n";
    }
};

// Creator (Factory)
class AnimalFactory {
public:
    enum AnimalType { DOG, CAT };
    
    static std::unique_ptr<Animal> createAnimal(AnimalType type) {
        switch (type) {
            case DOG:
                return std::make_unique<Dog>();
            case CAT:
                return std::make_unique<Cat>();
            default:
                return nullptr;
        }
    }
};

// Usage
int main() {
    auto dog = AnimalFactory::createAnimal(AnimalFactory::DOG);
    auto cat = AnimalFactory::createAnimal(AnimalFactory::CAT);
    
    dog->makeSound(); // Output: Woof! Woof!
    cat->makeSound(); // Output: Meow! Meow!
    
    return 0;
}
```

---

### Abstract Factory

**Category:** Creational

**Definition:** Provides an interface for creating families of related or dependent objects without specifying their concrete classes.

**Real-world Analogy:** Like a furniture store that has different styles (Modern, Victorian) - each style has its own factory that produces matching chairs, tables, and sofas.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>

// Abstract products
class Chair {
public:
    virtual ~Chair() = default;
    virtual void sitOn() = 0;
};

class Table {
public:
    virtual ~Table() = default;
    virtual void putOn() = 0;
};

// Modern furniture family
class ModernChair : public Chair {
public:
    void sitOn() override {
        std::cout << "Sitting on a modern chair\n";
    }
};

class ModernTable : public Table {
public:
    void putOn() override {
        std::cout << "Putting items on a modern table\n";
    }
};

// Victorian furniture family
class VictorianChair : public Chair {
public:
    void sitOn() override {
        std::cout << "Sitting on a Victorian chair\n";
    }
};

class VictorianTable : public Table {
public:
    void putOn() override {
        std::cout << "Putting items on a Victorian table\n";
    }
};

// Abstract factory
class FurnitureFactory {
public:
    virtual ~FurnitureFactory() = default;
    virtual std::unique_ptr<Chair> createChair() = 0;
    virtual std::unique_ptr<Table> createTable() = 0;
};

// Concrete factories
class ModernFurnitureFactory : public FurnitureFactory {
public:
    std::unique_ptr<Chair> createChair() override {
        return std::make_unique<ModernChair>();
    }
    
    std::unique_ptr<Table> createTable() override {
        return std::make_unique<ModernTable>();
    }
};

class VictorianFurnitureFactory : public FurnitureFactory {
public:
    std::unique_ptr<Chair> createChair() override {
        return std::make_unique<VictorianChair>();
    }
    
    std::unique_ptr<Table> createTable() override {
        return std::make_unique<VictorianTable>();
    }
};

// Usage
int main() {
    std::unique_ptr<FurnitureFactory> factory = std::make_unique<ModernFurnitureFactory>();
    
    auto chair = factory->createChair();
    auto table = factory->createTable();
    
    chair->sitOn();  // Output: Sitting on a modern chair
    table->putOn();  // Output: Putting items on a modern table
    
    return 0;
}
```

---

### Builder

**Category:** Creational

**Definition:** Constructs complex objects step by step. It allows you to produce different types and representations of an object using the same construction code.

**Real-world Analogy:** Like building a house - you follow a step-by-step process (foundation, walls, roof) and can customize each step to create different types of houses.

**C++ Implementation:**

```cpp
#include <iostream>
#include <string>

class Computer {
private:
    std::string cpu;
    std::string ram;
    std::string storage;
    std::string gpu;
    
public:
    void setCPU(const std::string& cpu) { this->cpu = cpu; }
    void setRAM(const std::string& ram) { this->ram = ram; }
    void setStorage(const std::string& storage) { this->storage = storage; }
    void setGPU(const std::string& gpu) { this->gpu = gpu; }
    
    void showSpecs() {
        std::cout << "Computer Specs:\n";
        std::cout << "CPU: " << cpu << "\n";
        std::cout << "RAM: " << ram << "\n";
        std::cout << "Storage: " << storage << "\n";
        std::cout << "GPU: " << gpu << "\n\n";
    }
};

class ComputerBuilder {
private:
    Computer computer;
    
public:
    ComputerBuilder& setCPU(const std::string& cpu) {
        computer.setCPU(cpu);
        return *this;
    }
    
    ComputerBuilder& setRAM(const std::string& ram) {
        computer.setRAM(ram);
        return *this;
    }
    
    ComputerBuilder& setStorage(const std::string& storage) {
        computer.setStorage(storage);
        return *this;
    }
    
    ComputerBuilder& setGPU(const std::string& gpu) {
        computer.setGPU(gpu);
        return *this;
    }
    
    Computer build() {
        return computer;
    }
};

// Usage
int main() {
    ComputerBuilder builder;
    
    Computer gamingPC = builder
        .setCPU("Intel i9-13900K")
        .setRAM("32GB DDR5")
        .setStorage("1TB NVMe SSD")
        .setGPU("RTX 4080")
        .build();
    
    gamingPC.showSpecs();
    
    return 0;
}
```

---

### Prototype

**Category:** Creational

**Definition:** Creates objects by cloning an existing instance rather than creating new instances from scratch.

**Real-world Analogy:** Like using a photocopy machine - instead of writing a document from scratch, you make copies of an existing document.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <string>

class Shape {
public:
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() = 0;
    virtual void draw() = 0;
    
protected:
    std::string color;
    
public:
    void setColor(const std::string& color) { this->color = color; }
    std::string getColor() const { return color; }
};

class Circle : public Shape {
private:
    int radius;
    
public:
    Circle(int radius) : radius(radius) {}
    
    std::unique_ptr<Shape> clone() override {
        auto cloned = std::make_unique<Circle>(radius);
        cloned->setColor(this->color);
        return cloned;
    }
    
    void draw() override {
        std::cout << "Drawing a " << color << " circle with radius " << radius << "\n";
    }
    
    void setRadius(int radius) { this->radius = radius; }
};

class Rectangle : public Shape {
private:
    int width, height;
    
public:
    Rectangle(int width, int height) : width(width), height(height) {}
    
    std::unique_ptr<Shape> clone() override {
        auto cloned = std::make_unique<Rectangle>(width, height);
        cloned->setColor(this->color);
        return cloned;
    }
    
    void draw() override {
        std::cout << "Drawing a " << color << " rectangle " << width << "x" << height << "\n";
    }
};

// Usage
int main() {
    auto originalCircle = std::make_unique<Circle>(5);
    originalCircle->setColor("red");
    
    auto clonedCircle = originalCircle->clone();
    
    originalCircle->draw(); // Drawing a red circle with radius 5
    clonedCircle->draw();   // Drawing a red circle with radius 5
    
    return 0;
}
```

---

## 🧱 Structural Patterns

Structural patterns deal with object composition and typically identify simple ways to realize relationships between different objects.

### Adapter

**Category:** Structural

**Definition:** Allows incompatible interfaces to work together. It acts as a bridge between two incompatible interfaces.

**Real-world Analogy:** Like a power adapter that allows you to plug a US device into a European outlet - it converts one interface to another.

**C++ Implementation:**

```cpp
#include <iostream>

// Target interface (what client expects)
class MediaPlayer {
public:
    virtual ~MediaPlayer() = default;
    virtual void play(const std::string& audioType, const std::string& fileName) = 0;
};

// Adaptee (existing interface that needs adaptation)
class AdvancedMediaPlayer {
public:
    virtual ~AdvancedMediaPlayer() = default;
    virtual void playVlc(const std::string& fileName) = 0;
    virtual void playMp4(const std::string& fileName) = 0;
};

class VlcPlayer : public AdvancedMediaPlayer {
public:
    void playVlc(const std::string& fileName) override {
        std::cout << "Playing vlc file: " << fileName << "\n";
    }
    
    void playMp4(const std::string& fileName) override {
        // Do nothing
    }
};

class Mp4Player : public AdvancedMediaPlayer {
public:
    void playVlc(const std::string& fileName) override {
        // Do nothing
    }
    
    void playMp4(const std::string& fileName) override {
        std::cout << "Playing mp4 file: " << fileName << "\n";
    }
};

// Adapter
class MediaAdapter : public MediaPlayer {
private:
    std::unique_ptr<AdvancedMediaPlayer> advancedPlayer;
    
public:
    MediaAdapter(const std::string& audioType) {
        if (audioType == "vlc") {
            advancedPlayer = std::make_unique<VlcPlayer>();
        } else if (audioType == "mp4") {
            advancedPlayer = std::make_unique<Mp4Player>();
        }
    }
    
    void play(const std::string& audioType, const std::string& fileName) override {
        if (audioType == "vlc") {
            advancedPlayer->playVlc(fileName);
        } else if (audioType == "mp4") {
            advancedPlayer->playMp4(fileName);
        }
    }
};

// Client
class AudioPlayer : public MediaPlayer {
private:
    std::unique_ptr<MediaAdapter> adapter;
    
public:
    void play(const std::string& audioType, const std::string& fileName) override {
        if (audioType == "mp3") {
            std::cout << "Playing mp3 file: " << fileName << "\n";
        } else if (audioType == "vlc" || audioType == "mp4") {
            adapter = std::make_unique<MediaAdapter>(audioType);
            adapter->play(audioType, fileName);
        } else {
            std::cout << "Invalid media. " << audioType << " format not supported\n";
        }
    }
};

// Usage
int main() {
    AudioPlayer player;
    
    player.play("mp3", "song.mp3");
    player.play("mp4", "video.mp4");
    player.play("vlc", "movie.vlc");
    player.play("avi", "video.avi");
    
    return 0;
}
```

---

### Bridge

**Category:** Structural

**Definition:** Separates an abstraction from its implementation so that both can be varied independently.

**Real-world Analogy:** Like a remote control and TV - the remote (abstraction) can work with different TV brands (implementations) without changing its interface.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>

// Implementation interface
class DrawingAPI {
public:
    virtual ~DrawingAPI() = default;
    virtual void drawCircle(double x, double y, double radius) = 0;
};

// Concrete implementations
class DrawingAPI1 : public DrawingAPI {
public:
    void drawCircle(double x, double y, double radius) override {
        std::cout << "API1.circle at (" << x << ", " << y << ") radius " << radius << "\n";
    }
};

class DrawingAPI2 : public DrawingAPI {
public:
    void drawCircle(double x, double y, double radius) override {
        std::cout << "API2.circle at (" << x << ", " << y << ") radius " << radius << "\n";
    }
};

// Abstraction
class Shape {
protected:
    std::unique_ptr<DrawingAPI> drawingAPI;
    
public:
    Shape(std::unique_ptr<DrawingAPI> api) : drawingAPI(std::move(api)) {}
    virtual ~Shape() = default;
    virtual void draw() = 0;
    virtual void resizeByPercentage(double pct) = 0;
};

// Refined abstraction
class CircleShape : public Shape {
private:
    double x, y, radius;
    
public:
    CircleShape(double x, double y, double radius, std::unique_ptr<DrawingAPI> api)
        : Shape(std::move(api)), x(x), y(y), radius(radius) {}
    
    void draw() override {
        drawingAPI->drawCircle(x, y, radius);
    }
    
    void resizeByPercentage(double pct) override {
        radius *= (1.0 + pct / 100.0);
    }
};

// Usage
int main() {
    auto circle1 = std::make_unique<CircleShape>(1, 2, 3, std::make_unique<DrawingAPI1>());
    auto circle2 = std::make_unique<CircleShape>(5, 7, 11, std::make_unique<DrawingAPI2>());
    
    circle1->resizeByPercentage(2.5);
    circle1->draw();
    
    circle2->resizeByPercentage(2.5);
    circle2->draw();
    
    return 0;
}
```

---

### Composite

**Category:** Structural

**Definition:** Composes objects into tree structures to represent part-whole hierarchies. It lets clients treat individual objects and compositions uniformly.

**Real-world Analogy:** Like a file system - both files and folders can be treated the same way (you can copy, move, or delete both), but folders can contain other files and folders.

**C++ Implementation:**

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Component interface
class FileSystemComponent {
public:
    virtual ~FileSystemComponent() = default;
    virtual void display(int depth = 0) = 0;
    virtual int getSize() = 0;
    
    // Default implementations for composite operations
    virtual void add(std::shared_ptr<FileSystemComponent> component) {
        throw std::runtime_error("Operation not supported");
    }
    
    virtual void remove(std::shared_ptr<FileSystemComponent> component) {
        throw std::runtime_error("Operation not supported");
    }
};

// Leaf
class File : public FileSystemComponent {
private:
    std::string name;
    int size;
    
public:
    File(const std::string& name, int size) : name(name), size(size) {}
    
    void display(int depth = 0) override {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "File: " << name << " (" << size << " bytes)\n";
    }
    
    int getSize() override {
        return size;
    }
};

// Composite
class Directory : public FileSystemComponent {
private:
    std::string name;
    std::vector<std::shared_ptr<FileSystemComponent>> children;
    
public:
    Directory(const std::string& name) : name(name) {}
    
    void add(std::shared_ptr<FileSystemComponent> component) override {
        children.push_back(component);
    }
    
    void remove(std::shared_ptr<FileSystemComponent> component) override {
        children.erase(
            std::remove(children.begin(), children.end(), component),
            children.end()
        );
    }
    
    void display(int depth = 0) override {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "Directory: " << name << "\n";
        
        for (auto& child : children) {
            child->display(depth + 1);
        }
    }
    
    int getSize() override {
        int totalSize = 0;
        for (auto& child : children) {
            totalSize += child->getSize();
        }
        return totalSize;
    }
};

// Usage
int main() {
    auto root = std::make_shared<Directory>("root");
    auto documents = std::make_shared<Directory>("documents");
    auto pictures = std::make_shared<Directory>("pictures");
    
    auto file1 = std::make_shared<File>("resume.pdf", 1024);
    auto file2 = std::make_shared<File>("photo1.jpg", 2048);
    auto file3 = std::make_shared<File>("photo2.jpg", 1536);
    
    documents->add(file1);
    pictures->add(file2);
    pictures->add(file3);
    
    root->add(documents);
    root->add(pictures);
    
    root->display();
    std::cout << "Total size: " << root->getSize() << " bytes\n";
    
    return 0;
}
```

---

### Decorator

**Category:** Structural

**Definition:** Attaches new behaviors to objects by placing these objects inside special wrapper objects that contain the behaviors.

**Real-world Analogy:** Like dressing up - you start with basic clothes and add accessories (hat, jewelry, jacket) to enhance your appearance without changing the basic clothes.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <string>

// Component interface
class Coffee {
public:
    virtual ~Coffee() = default;
    virtual std::string getDescription() = 0;
    virtual double getCost() = 0;
};

// Concrete component
class SimpleCoffee : public Coffee {
public:
    std::string getDescription() override {
        return "Simple coffee";
    }
    
    double getCost() override {
        return 2.0;
    }
};

// Base decorator
class CoffeeDecorator : public Coffee {
protected:
    std::unique_ptr<Coffee> coffee;
    
public:
    CoffeeDecorator(std::unique_ptr<Coffee> coffee) : coffee(std::move(coffee)) {}
    
    std::string getDescription() override {
        return coffee->getDescription();
    }
    
    double getCost() override {
        return coffee->getCost();
    }
};

// Concrete decorators
class MilkDecorator : public CoffeeDecorator {
public:
    MilkDecorator(std::unique_ptr<Coffee> coffee) : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() override {
        return coffee->getDescription() + ", milk";
    }
    
    double getCost() override {
        return coffee->getCost() + 0.5;
    }
};

class SugarDecorator : public CoffeeDecorator {
public:
    SugarDecorator(std::unique_ptr<Coffee> coffee) : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() override {
        return coffee->getDescription() + ", sugar";
    }
    
    double getCost() override {
        return coffee->getCost() + 0.2;
    }
};

class WhipDecorator : public CoffeeDecorator {
public:
    WhipDecorator(std::unique_ptr<Coffee> coffee) : CoffeeDecorator(std::move(coffee)) {}
    
    std::string getDescription() override {
        return coffee->getDescription() + ", whip";
    }
    
    double getCost() override {
        return coffee->getCost() + 0.7;
    }
};

// Usage
int main() {
    // Start with simple coffee
    std::unique_ptr<Coffee> coffee = std::make_unique<SimpleCoffee>();
    std::cout << coffee->getDescription() << " $" << coffee->getCost() << "\n";
    
    // Add milk
    coffee = std::make_unique<MilkDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " $" << coffee->getCost() << "\n";
    
    // Add sugar
    coffee = std::make_unique<SugarDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " $" << coffee->getCost() << "\n";
    
    // Add whip
    coffee = std::make_unique<WhipDecorator>(std::move(coffee));
    std::cout << coffee->getDescription() << " $" << coffee->getCost() << "\n";
    
    return 0;
}
```

---

### Facade

**Category:** Structural

**Definition:** Provides a simplified interface to a complex subsystem. It defines a higher-level interface that makes the subsystem easier to use.

**Real-world Analogy:** Like a travel agency - instead of booking flights, hotels, and car rentals separately, you just tell the agency your destination and they handle all the complex arrangements.

**C++ Implementation:**

```cpp
#include <iostream>

// Complex subsystem classes
class CPU {
public:
    void freeze() { std::cout << "CPU: Freezing processor\n"; }
    void jump(long position) { std::cout << "CPU: Jumping to position " << position << "\n"; }
    void execute() { std::cout << "CPU: Executing instructions\n"; }
};

class Memory {
public:
    void load(long position, const std::string& data) {
        std::cout << "Memory: Loading '" << data << "' at position " << position << "\n";
    }
};

class HardDrive {
public:
    std::string read(long lba, int size) {
        std::cout << "HardDrive: Reading " << size << " bytes from LBA " << lba << "\n";
        return "boot_data";
    }
};

// Facade
class ComputerFacade {
private:
    CPU cpu;
    Memory memory;
    HardDrive hardDrive;
    
public:
    void start() {
        std::cout << "Starting computer...\n";
        cpu.freeze();
        memory.load(0, hardDrive.read(0, 1024));
        cpu.jump(0);
        cpu.execute();
        std::cout << "Computer started successfully!\n\n";
    }
    
    void shutdown() {
        std::cout << "Shutting down computer...\n";
        std::cout << "Computer shut down successfully!\n\n";
    }
};

// Usage
int main() {
    ComputerFacade computer;
    
    // Simple interface to complex operations
    computer.start();
    computer.shutdown();
    
    return 0;
}
```

---

### Flyweight

**Category:** Structural

**Definition:** Minimizes memory usage by sharing efficiently common data among multiple similar objects.

**Real-world Analogy:** Like a word processor - instead of storing font information for each character, it stores font data once and references it for all characters using that font.

**C++ Implementation:**

```cpp
#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>

// Flyweight interface
class CharacterFlyweight {
public:
    virtual ~CharacterFlyweight() = default;
    virtual void display(int x, int y, int fontSize) = 0;
};

// Concrete flyweight
class ConcreteCharacter : public CharacterFlyweight {
private:
    char character;  // Intrinsic state
    
public:
    ConcreteCharacter(char c) : character(c) {}
    
    void display(int x, int y, int fontSize) override {
        std::cout << "Character '" << character << "' at (" << x << ", " << y 
                  << ") with font size " << fontSize << "\n";
    }
};

// Flyweight factory
class CharacterFactory {
private:
    static std::unordered_map<char, std::shared_ptr<CharacterFlyweight>> flyweights;
    
public:
    static std::shared_ptr<CharacterFlyweight> getFlyweight(char c) {
        if (flyweights.find(c) == flyweights.end()) {
            flyweights[c] = std::make_shared<ConcreteCharacter>(c);
            std::cout << "Creating flyweight for character '" << c << "'\n";
        }
        return flyweights[c];
    }
    
    static int getCreatedFlyweightsCount() {
        return flyweights.size();
    }
};

// Static member definition
std::unordered_map<char, std::shared_ptr<CharacterFlyweight>> CharacterFactory::flyweights;

// Context class that uses flyweights
class Character {
private:
    std::shared_ptr<CharacterFlyweight> flyweight;
    int x, y, fontSize;  // Extrinsic state
    
public:
    Character(char c, int x, int y, int fontSize) 
        : flyweight(CharacterFactory::getFlyweight(c)), x(x), y(y), fontSize(fontSize) {}
    
    void display() {
        flyweight->display(x, y, fontSize);
    }
};

// Usage
int main() {
    std::string text = "HELLO WORLD";
    std::vector<Character> characters;
    
    // Create characters
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] != ' ') {
            characters.emplace_back(text[i], i * 10, 0, 12);
        }
    }
    
    // Display all characters
    for (auto& character : characters) {
        character.display();
    }
    
    std::cout << "\nTotal flyweights created: " << CharacterFactory::getCreatedFlyweightsCount() << "\n";
    std::cout << "Total characters displayed: " << characters.size() << "\n";
    
    return 0;
}
```

---

### Proxy

**Category:** Structural

**Definition:** Provides a placeholder or surrogate for another object to control access to it.

**Real-world Analogy:** Like a security guard at a building - they control who can enter and may perform additional checks before allowing access to the actual building.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <string>

// Subject interface
class Image {
public:
    virtual ~Image() = default;
    virtual void display() = 0;
};

// Real subject
class RealImage : public Image {
private:
    std::string filename;
    
    void loadFromDisk() {
        std::cout << "Loading image from disk: " << filename << "\n";
        // Simulate expensive loading operation
    }
    
public:
    RealImage(const std::string& filename) : filename(filename) {
        loadFromDisk();
    }
    
    void display() override {
        std::cout << "Displaying image: " << filename << "\n";
    }
};

// Proxy
class ImageProxy : public Image {
private:
    std::string filename;
    mutable std::unique_ptr<RealImage> realImage;
    
public:
    ImageProxy(const std::string& filename) : filename(filename) {}
    
    void display() override {
        if (!realImage) {
            std::cout << "Proxy: Creating real image on first access\n";
            realImage = std::make_unique<RealImage>(filename);
        }
        realImage->display();
    }
};

// Usage
int main() {
    std::cout << "Creating image proxies (no loading yet):\n";
    auto image1 = std::make_unique<ImageProxy>("photo1.jpg");
    auto image2 = std::make_unique<ImageProxy>("photo2.jpg");
    
    std::cout << "\nFirst display of image1 (triggers loading):\n";
    image1->display();
    
    std::cout << "\nSecond display of image1 (no loading):\n";
    image1->display();
    
    std::cout << "\nFirst display of image2 (triggers loading):\n";
    image2->display();
    
    return 0;
}
```

---

## 🔄 Behavioral Patterns

Behavioral patterns focus on communication between objects and the assignment of responsibilities between objects.

### Chain of Responsibility

**Category:** Behavioral

**Definition:** Passes requests along a chain of handlers. Upon receiving a request, each handler decides either to process the request or to pass it to the next handler in the chain.

**Real-world Analogy:** Like a customer service escalation system - your call starts with level 1 support, and if they can't help, it goes to level 2, then level 3, until someone can resolve your issue.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <string>

// Handler interface
class SupportHandler {
protected:
    std::shared_ptr<SupportHandler> nextHandler;
    
public:
    virtual ~SupportHandler() = default;
    
    void setNext(std::shared_ptr<SupportHandler> handler) {
        nextHandler = handler;
    }
    
    virtual void handleRequest(const std::string& request, int priority) {
        if (nextHandler) {
            nextHandler->handleRequest(request, priority);
        } else {
            std::cout << "No handler available for: " << request << "\n";
        }
    }
};

// Concrete handlers
class Level1Support : public SupportHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority <= 1) {
            std::cout << "Level 1 Support handled: " << request << "\n";
        } else {
            std::cout << "Level 1 Support: Escalating to Level 2\n";
            SupportHandler::handleRequest(request, priority);
        }
    }
};

class Level2Support : public SupportHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority <= 2) {
            std::cout << "Level 2 Support handled: " << request << "\n";
        } else {
            std::cout << "Level 2 Support: Escalating to Level 3\n";
            SupportHandler::handleRequest(request, priority);
        }
    }
};

class Level3Support : public SupportHandler {
public:
    void handleRequest(const std::string& request, int priority) override {
        if (priority <= 3) {
            std::cout << "Level 3 Support handled: " << request << "\n";
        } else {
            std::cout << "Level 3 Support: Escalating to Management\n";
            SupportHandler::handleRequest(request, priority);
        }
    }
};

// Usage
int main() {
    auto level1 = std::make_shared<Level1Support>();
    auto level2 = std::make_shared<Level2Support>();
    auto level3 = std::make_shared<Level3Support>();
    
    // Set up the chain
    level1->setNext(level2);
    level2->setNext(level3);
    
    // Test different priority requests
    level1->handleRequest("Password reset", 1);
    level1->handleRequest("Software installation", 2);
    level1->handleRequest("Server crash", 3);
    level1->handleRequest("Data center fire", 4);
    
    return 0;
}
```

---

### Command

**Category:** Behavioral

**Definition:** Encapsulates a request as an object, thereby letting you parameterize clients with different requests, queue or log requests, and support undoable operations.

**Real-world Analogy:** Like a restaurant order - the waiter writes down your order (command) and gives it to the kitchen. The order contains all the information needed to prepare your meal, and it can be queued, modified, or cancelled.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Receiver
class Light {
private:
    std::string location;
    bool isOn;
    
public:
    Light(const std::string& location) : location(location), isOn(false) {}
    
    void turnOn() {
        isOn = true;
        std::cout << location << " light is ON\n";
    }
    
    void turnOff() {
        isOn = false;
        std::cout << location << " light is OFF\n";
    }
    
    bool getState() const { return isOn; }
};

// Command interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Concrete commands
class LightOnCommand : public Command {
private:
    std::shared_ptr<Light> light;
    
public:
    LightOnCommand(std::shared_ptr<Light> light) : light(light) {}
    
    void execute() override {
        light->turnOn();
    }
    
    void undo() override {
        light->turnOff();
    }
};

class LightOffCommand : public Command {
private:
    std::shared_ptr<Light> light;
    
public:
    LightOffCommand(std::shared_ptr<Light> light) : light(light) {}
    
    void execute() override {
        light->turnOff();
    }
    
    void undo() override {
        light->turnOn();
    }
};

// Invoker
class RemoteControl {
private:
    std::vector<std::shared_ptr<Command>> commands;
    std::shared_ptr<Command> lastCommand;
    
public:
    void setCommand(int slot, std::shared_ptr<Command> command) {
        if (slot >= commands.size()) {
            commands.resize(slot + 1);
        }
        commands[slot] = command;
    }
    
    void pressButton(int slot) {
        if (slot < commands.size() && commands[slot]) {
            commands[slot]->execute();
            lastCommand = commands[slot];
        }
    }
    
    void pressUndo() {
        if (lastCommand) {
            lastCommand->undo();
        }
    }
};

// Usage
int main() {
    // Create receivers
    auto livingRoomLight = std::make_shared<Light>("Living Room");
    auto kitchenLight = std::make_shared<Light>("Kitchen");
    
    // Create commands
    auto livingRoomOn = std::make_shared<LightOnCommand>(livingRoomLight);
    auto livingRoomOff = std::make_shared<LightOffCommand>(livingRoomLight);
    auto kitchenOn = std::make_shared<LightOnCommand>(kitchenLight);
    auto kitchenOff = std::make_shared<LightOffCommand>(kitchenLight);
    
    // Create invoker
    RemoteControl remote;
    
    // Set up commands
    remote.setCommand(0, livingRoomOn);
    remote.setCommand(1, livingRoomOff);
    remote.setCommand(2, kitchenOn);
    remote.setCommand(3, kitchenOff);
    
    // Use the remote
    remote.pressButton(0);  // Living room light on
    remote.pressButton(2);  // Kitchen light on
    remote.pressUndo();     // Undo last command (kitchen light off)
    remote.pressButton(1);  // Living room light off
    
    return 0;
}
```

---

### Interpreter

**Category:** Behavioral

**Definition:** Defines a representation for a grammar of a language and provides an interpreter to deal with this grammar.

**Real-world Analogy:** Like a language translator - it takes sentences in one language and converts them to another language by understanding the grammar rules.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// Context
class Context {
private:
    std::unordered_map<std::string, int> variables;
    
public:
    void setVariable(const std::string& name, int value) {
        variables[name] = value;
    }
    
    int getVariable(const std::string& name) {
        return variables.count(name) ? variables[name] : 0;
    }
};

// Abstract expression
class Expression {
public:
    virtual ~Expression() = default;
    virtual int interpret(Context& context) = 0;
};

// Terminal expression
class NumberExpression : public Expression {
private:
    int number;
    
public:
    NumberExpression(int number) : number(number) {}
    
    int interpret(Context& context) override {
        return number;
    }
};

class VariableExpression : public Expression {
private:
    std::string name;
    
public:
    VariableExpression(const std::string& name) : name(name) {}
    
    int interpret(Context& context) override {
        return context.getVariable(name);
    }
};

// Non-terminal expressions
class AddExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    AddExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left(std::move(left)), right(std::move(right)) {}
    
    int interpret(Context& context) override {
        return left->interpret(context) + right->interpret(context);
    }
};

class SubtractExpression : public Expression {
private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
public:
    SubtractExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left(std::move(left)), right(std::move(right)) {}
    
    int interpret(Context& context) override {
        return left->interpret(context) - right->interpret(context);
    }
};

// Usage
int main() {
    Context context;
    context.setVariable("x", 10);
    context.setVariable("y", 5);
    
    // Build expression: (x + y) - 3
    auto expression = std::make_unique<SubtractExpression>(
        std::make_unique<AddExpression>(
            std::make_unique<VariableExpression>("x"),
            std::make_unique<VariableExpression>("y")
        ),
        std::make_unique<NumberExpression>(3)
    );
    
    int result = expression->interpret(context);
    std::cout << "Result of (x + y) - 3 where x=10, y=5: " << result << "\n";
    
    return 0;
}
```

---

### Iterator

**Category:** Behavioral

**Definition:** Provides a way to access the elements of an aggregate object sequentially without exposing its underlying representation.

**Real-world Analogy:** Like a TV remote control - you can navigate through channels one by one without knowing how the TV stores or organizes the channel list internally.

**C++ Implementation:**

```cpp
#include <iostream>
#include <vector>
#include <memory>

template<typename T>
class Iterator {
public:
    virtual ~Iterator() = default;
    virtual bool hasNext() = 0;
    virtual T next() = 0;
};

template<typename T>
class Aggregate {
public:
    virtual ~Aggregate() = default;
    virtual std::unique_ptr<Iterator<T>> createIterator() = 0;
};

// Concrete iterator
template<typename T>
class ConcreteIterator : public Iterator<T> {
private:
    const std::vector<T>& collection;
    size_t position;
    
public:
    ConcreteIterator(const std::vector<T>& collection) 
        : collection(collection), position(0) {}
    
    bool hasNext() override {
        return position < collection.size();
    }
    
    T next() override {
        if (hasNext()) {
            return collection[position++];
        }
        throw std::out_of_range("No more elements");
    }
};

// Concrete aggregate
template<typename T>
class ConcreteAggregate : public Aggregate<T> {
private:
    std::vector<T> items;
    
public:
    void addItem(const T& item) {
        items.push_back(item);
    }
    
    std::unique_ptr<Iterator<T>> createIterator() override {
        return std::make_unique<ConcreteIterator<T>>(items);
    }
    
    size_t size() const { return items.size(); }
};

// Usage
int main() {
    auto collection = std::make_unique<ConcreteAggregate<std::string>>();
    
    collection->addItem("Apple");
    collection->addItem("Banana");
    collection->addItem("Cherry");
    collection->addItem("Date");
    
    auto iterator = collection->createIterator();
    
    std::cout << "Iterating through the collection:\n";
    while (iterator->hasNext()) {
        std::cout << "- " << iterator->next() << "\n";
    }
    
    return 0;
}
```

---

### Mediator

**Category:** Behavioral

**Definition:** Defines how a set of objects interact with each other. It promotes loose coupling by keeping objects from referring to each other explicitly.

**Real-world Analogy:** Like an air traffic control tower - pilots don't communicate directly with each other; instead, they all communicate through the control tower, which coordinates all the flights.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Forward declaration
class Colleague;

// Mediator interface
class Mediator {
public:
    virtual ~Mediator() = default;
    virtual void notify(Colleague* sender, const std::string& event) = 0;
};

// Colleague base class
class Colleague {
protected:
    std::shared_ptr<Mediator> mediator;
    
public:
    Colleague(std::shared_ptr<Mediator> mediator) : mediator(mediator) {}
    virtual ~Colleague() = default;
};

// Concrete colleagues
class Button : public Colleague {
public:
    Button(std::shared_ptr<Mediator> mediator) : Colleague(mediator) {}
    
    void click() {
        std::cout << "Button clicked\n";
        mediator->notify(this, "click");
    }
};

class TextBox : public Colleague {
private:
    std::string text;
    
public:
    TextBox(std::shared_ptr<Mediator> mediator) : Colleague(mediator) {}
    
    void setText(const std::string& text) {
        this->text = text;
        std::cout << "TextBox text set to: " << text << "\n";
        mediator->notify(this, "textChanged");
    }
    
    std::string getText() const { return text; }
};

class CheckBox : public Colleague {
private:
    bool checked;
    
public:
    CheckBox(std::shared_ptr<Mediator> mediator) : Colleague(mediator), checked(false) {}
    
    void setChecked(bool checked) {
        this->checked = checked;
        std::cout << "CheckBox " << (checked ? "checked" : "unchecked") << "\n";
        mediator->notify(this, checked ? "checked" : "unchecked");
    }
    
    bool isChecked() const { return checked; }
};

// Concrete mediator
class DialogMediator : public Mediator {
private:
    std::shared_ptr<Button> button;
    std::shared_ptr<TextBox> textBox;
    std::shared_ptr<CheckBox> checkBox;
    
public:
    void setComponents(std::shared_ptr<Button> button, 
                      std::shared_ptr<TextBox> textBox,
                      std::shared_ptr<CheckBox> checkBox) {
        this->button = button;
        this->textBox = textBox;
        this->checkBox = checkBox;
    }
    
    void notify(Colleague* sender, const std::string& event) override {
        if (sender == button.get() && event == "click") {
            std::cout << "Mediator: Button clicked, processing form...\n";
            if (checkBox->isChecked()) {
                std::cout << "Mediator: Saving text: " << textBox->getText() << "\n";
            } else {
                std::cout << "Mediator: Please check the agreement box\n";
            }
        } else if (sender == textBox.get() && event == "textChanged") {
            std::cout << "Mediator: Text changed, validating...\n";
        } else if (sender == checkBox.get()) {
            if (event == "checked") {
                std::cout << "Mediator: Agreement accepted\n";
            } else {
                std::cout << "Mediator: Agreement declined\n";
            }
        }
    }
};

// Usage
int main() {
    auto mediator = std::make_shared<DialogMediator>();
    
    auto button = std::make_shared<Button>(mediator);
    auto textBox = std::make_shared<TextBox>(mediator);
    auto checkBox = std::make_shared<CheckBox>(mediator);
    
    mediator->setComponents(button, textBox, checkBox);
    
    // Simulate user interactions
    textBox->setText("Hello World");
    checkBox->setChecked(true);
    button->click();
    
    std::cout << "\n";
    
    checkBox->setChecked(false);
    button->click();
    
    return 0;
}
```

---

### Memento

**Category:** Behavioral

**Definition:** Captures and externalizes an object's internal state so that the object can be restored to this state later, without violating encapsulation.

**Real-world Analogy:** Like a save game feature in video games - you can save your progress at any point and restore it later if needed, without the game exposing all its internal mechanics.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Memento
class TextMemento {
private:
    std::string state;
    
public:
    TextMemento(const std::string& state) : state(state) {}
    std::string getState() const { return state; }
};

// Originator
class TextEditor {
private:
    std::string content;
    
public:
    void write(const std::string& text) {
        content += text;
        std::cout << "Current content: " << content << "\n";
    }
    
    void clear() {
        content.clear();
        std::cout << "Content cleared\n";
    }
    
    std::string getContent() const { return content; }
    
    // Create memento
    std::unique_ptr<TextMemento> save() {
        std::cout << "Saving state: " << content << "\n";
        return std::make_unique<TextMemento>(content);
    }
    
    // Restore from memento
    void restore(const TextMemento& memento) {
        content = memento.getState();
        std::cout << "Restored to: " << content << "\n";
    }
};

// Caretaker
class History {
private:
    std::vector<std::unique_ptr<TextMemento>> mementos;
    
public:
    void save(std::unique_ptr<TextMemento> memento) {
        mementos.push_back(std::move(memento));
    }
    
    std::unique_ptr<TextMemento> undo() {
        if (!mementos.empty()) {
            auto memento = std::move(mementos.back());
            mementos.pop_back();
            return memento;
        }
        return nullptr;
    }
    
    size_t size() const { return mementos.size(); }
};

// Usage
int main() {
    TextEditor editor;
    History history;
    
    // Write some text and save states
    editor.write("Hello ");
    history.save(editor.save());
    
    editor.write("World!");
    history.save(editor.save());
    
    editor.write(" How are you?");
    std::cout << "Final content: " << editor.getContent() << "\n\n";
    
    // Undo operations
    std::cout << "Undoing...\n";
    auto memento = history.undo();
    if (memento) {
        editor.restore(*memento);
    }
    
    std::cout << "Undoing again...\n";
    memento = history.undo();
    if (memento) {
        editor.restore(*memento);
    }
    
    return 0;
}
```

---

### Observer

**Category:** Behavioral

**Definition:** Defines a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.

**Real-world Analogy:** Like a newspaper subscription - when a new edition is published, all subscribers automatically receive a copy without the newspaper having to know who the subscribers are individually.

**C++ Implementation:**

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

// Observer interface
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

// Subject interface
class Subject {
public:
    virtual ~Subject() = default;
    virtual void attach(std::shared_ptr<Observer> observer) = 0;
    virtual void detach(std::shared_ptr<Observer> observer) = 0;
    virtual void notify() = 0;
};

// Concrete subject
class NewsAgency : public Subject {
private:
    std::vector<std::shared_ptr<Observer>> observers;
    std::string news;
    
public:
    void attach(std::shared_ptr<Observer> observer) override {
        observers.push_back(observer);
        std::cout << "Observer attached\n";
    }
    
    void detach(std::shared_ptr<Observer> observer) override {
        observers.erase(
            std::remove(observers.begin(), observers.end(), observer),
            observers.end()
        );
        std::cout << "Observer detached\n";
    }
    
    void notify() override {
        std::cout << "Notifying all observers...\n";
        for (auto& observer : observers) {
            observer->update(news);
        }
    }
    
    void setNews(const std::string& news) {
        this->news = news;
        notify();
    }
    
    std::string getNews() const { return news; }
};

// Concrete observers
class NewsChannel : public Observer {
private:
    std::string name;
    
public:
    NewsChannel(const std::string& name) : name(name) {}
    
    void update(const std::string& message) override {
        std::cout << name << " received news: " << message << "\n";
    }
};

class NewsPaper : public Observer {
private:
    std::string name;
    
public:
    NewsPaper(const std::string& name) : name(name) {}
    
    void update(const std::string& message) override {
        std::cout << name << " printing news: " << message << "\n";
    }
};

// Usage
int main() {
    auto newsAgency = std::make_shared<NewsAgency>();
    
    auto cnn = std::make_shared<NewsChannel>("CNN");
    auto bbc = std::make_shared<NewsChannel>("BBC");
    auto times = std::make_shared<NewsPaper>("The Times");
    
    // Subscribe to news
    newsAgency->attach(cnn);
    newsAgency->attach(bbc);
    newsAgency->attach(times);
    
    // Publish news
    std::cout << "\n--- Breaking News ---\n";
    newsAgency->setNews("Major scientific breakthrough announced!");
    
    std::cout << "\n--- Unsubscribing BBC ---\n";
    newsAgency->detach(bbc);
    
    std::cout << "\n--- Another News ---\n";
    newsAgency->setNews("Stock market reaches new high!");
    
    return 0;
}
```

---

### State

**Category:** Behavioral

**Definition:** Allows an object to alter its behavior when its internal state changes. The object will appear to change its class.

**Real-world Analogy:** Like a vending machine - it behaves differently depending on its state (has money, no money, item selected, etc.). The same button press can have different effects based on the current state.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>

// Forward declaration
class VendingMachine;

// State interface
class State {
public:
    virtual ~State() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void selectProduct(VendingMachine& machine) = 0;
    virtual void dispense(VendingMachine& machine) = 0;
};

// Context
class VendingMachine {
private:
    std::unique_ptr<State> currentState;
    int coinCount;
    
public:
    VendingMachine();
    
    void setState(std::unique_ptr<State> state) {
        currentState = std::move(state);
    }
    
    void insertCoin() { currentState->insertCoin(*this); }
    void selectProduct() { currentState->selectProduct(*this); }
    void dispense() { currentState->dispense(*this); }
    
    void addCoin() { coinCount++; }
    void resetCoins() { coinCount = 0; }
    int getCoinCount() const { return coinCount; }
};

// Concrete states
class NoCoinState : public State {
public:
    void insertCoin(VendingMachine& machine) override;
    void selectProduct(VendingMachine& machine) override {
        std::cout << "Please insert coin first\n";
    }
    void dispense(VendingMachine& machine) override {
        std::cout << "Please insert coin first\n";
    }
};

class HasCoinState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        machine.addCoin();
        std::cout << "Coin inserted. Total coins: " << machine.getCoinCount() << "\n";
    }
    void selectProduct(VendingMachine& machine) override;
    void dispense(VendingMachine& machine) override {
        std::cout << "Please select product first\n";
    }
};

class ProductSelectedState : public State {
public:
    void insertCoin(VendingMachine& machine) override {
        machine.addCoin();
        std::cout << "Coin inserted. Total coins: " << machine.getCoinCount() << "\n";
    }
    void selectProduct(VendingMachine& machine) override {
        std::cout << "Product already selected\n";
    }
    void dispense(VendingMachine& machine) override;
};

// State implementations
VendingMachine::VendingMachine() : coinCount(0) {
    currentState = std::make_unique<NoCoinState>();
}

void NoCoinState::insertCoin(VendingMachine& machine) {
    machine.addCoin();
    std::cout << "Coin inserted. Total coins: " << machine.getCoinCount() << "\n";
    machine.setState(std::make_unique<HasCoinState>());
}

void HasCoinState::selectProduct(VendingMachine& machine) {
    std::cout << "Product selected\n";
    machine.setState(std::make_unique<ProductSelectedState>());
}

void ProductSelectedState::dispense(VendingMachine& machine) {
    if (machine.getCoinCount() >= 1) {
        std::cout << "Product dispensed! Enjoy!\n";
        machine.resetCoins();
        machine.setState(std::make_unique<NoCoinState>());
    } else {
        std::cout << "Insufficient coins\n";
        machine.setState(std::make_unique<NoCoinState>());
    }
}

// Usage
int main() {
    VendingMachine machine;
    
    std::cout << "=== Vending Machine Demo ===\n";
    
    machine.selectProduct();  // Should ask for coin
    machine.insertCoin();     // Insert coin
    machine.selectProduct();  // Select product
    machine.dispense();       // Get product
    
    std::cout << "\n=== Another Transaction ===\n";
    machine.insertCoin();
    machine.insertCoin();     // Insert another coin
    machine.selectProduct();
    machine.dispense();
    
    return 0;
}
```

---

### Strategy

**Category:** Behavioral

**Definition:** Defines a family of algorithms, encapsulates each one, and makes them interchangeable. Strategy lets the algorithm vary independently from clients that use it.

**Real-world Analogy:** Like choosing different routes to reach your destination - you can take the highway (fast but tolls), city streets (slower but free), or scenic route (longest but beautiful). The destination is the same, but the strategy differs.

**C++ Implementation:**

```cpp
#include <iostream>
#include <memory>
#include <vector>

// Strategy interface
class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
    virtual std::string getName() = 0;
};

// Concrete strategies
class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Sorting using Bubble Sort\n";
        int n = data.size();
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n - i - 1; j++) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
    
    std::string getName() override { return "Bubble Sort"; }
};

class QuickSort : public SortStrategy {
private:
    void quickSort(std::vector<int>& data, int low, int high) {
        if (low < high) {
            int pi = partition(data, low, high);
            quickSort(data, low, pi - 1);
            quickSort(data, pi + 1, high);
        }
    }
    
    int partition(std::vector<int>& data, int low, int high) {
        int pivot = data[high];
        int i = (low - 1);
        
        for (int j = low; j <= high - 1; j++) {
            if (data[j] < pivot) {
                i++;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[high]);
        return (i + 1);
    }
    
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Sorting using Quick Sort\n";
        quickSort(data, 0, data.size() - 1);
    }
    
    std::string getName() override { return "Quick Sort"; }
};

class MergeSort : public SortStrategy {
private:
    void mergeSort(std::vector<int>& data, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(data, left, mid);
            mergeSort(data, mid + 1, right);
            merge(data, left, mid, right);
        }
    }
    
    void merge(std::vector<int>& data, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;
        
        std::vector<int> leftArr(n1), rightArr(n2);
        
        for (int i = 0; i < n1; i++)
            leftArr[i] = data[left + i];
        for (int j = 0; j < n2; j++)
            rightArr[j] = data[mid + 1 + j];
        
        int i = 0, j = 0, k = left;
        
        while (i < n1 && j < n2) {
            if (leftArr[i] <= rightArr[j]) {
                data[k] = leftArr[i];
                i++;
            } else {
                data[k] = rightArr[j];
                j++;
            }
            k++;
        }
        
        while (i < n1) {
            data[k] = leftArr[i];
            i++;
            k++;
        }
        
        while (j < n2) {
            data[k] = rightArr[j];
            j++;
            k++;
        }
    }
    
public:
    void sort(std::vector<int>& data) override {
        std::cout << "Sorting using Merge Sort\n";
        mergeSort(data, 0, data.size() - 1);
    }
    
    std::string getName() override { return "Merge Sort"; }
};

// Context
class SortContext {
private:
    std::unique_ptr<SortStrategy> strategy;
    
public:
    void setStrategy(std::unique_ptr<SortStrategy> strategy) {
        this->strategy = std::move(strategy);
    }
    
    void executeSort(std::vector<int>& data) {
        if (strategy) {
            strategy->sort(data);
        }
    }
    
    std::string getCurrentStrategy() {
        return strategy ? strategy->getName() : "No strategy set";
    }
};

// Helper function to print vector
void printVector(const std::vector<int>& data) {
    for (int num : data) {
        std::cout << num << " ";
    }
    std::cout << "\n";
}

// Usage
int main() {
    SortContext context;
    
    // Test data
    std::vector<int> data1 = {64, 34, 25, 12, 22, 11, 90};
    std::vector<int> data2 = {64, 34, 25, 12, 22, 11, 90};
    std::vector<int> data3 = {64, 34, 25, 12, 22, 11, 90};
    
    std::cout << "Original data: ";
    printVector(data1);
    std::cout << "\n";
    
    // Use Bubble Sort
    context.setStrategy(std::make_unique<BubbleSort>());
    std::cout << "Using " << context.getCurrentStrategy() << "\n";
    context.executeSort(data1);
    std::cout << "Result: ";
    printVector(data1);
    std::cout << "\n";
    
    // Use Quick Sort
    context.setStrategy(std::make_unique<QuickSort>());
    std::cout << "Using " << context.getCurrentStrategy() << "\n";
    context.executeSort(data2);
    std::cout << "Result: ";
    printVector(data2);
    std::cout << "\n";
    
    // Use Merge Sort
    context.setStrategy(std::make_unique<MergeSort>());
    std::cout << "Using " << context.getCurrentStrategy() << "\n";
    context.executeSort(data3);
    std::cout << "Result: ";
    printVector(data3);
    
    return 0;
}
```

---

### Template Method

**Category:** Behavioral

**Definition:** Defines the skeleton of an algorithm in a base class, letting subclasses override specific steps of the algorithm without changing its structure.

**Real-world Analogy:** Like a recipe for making different types of tea - the basic steps are the same (boil water, add tea, steep, serve), but each type of tea might have specific variations in timing or ingredients.

**C++ Implementation:**

```cpp
#include <iostream>
#include <string>

// Abstract class defining the template method
class DataProcessor {
public:
    // Template method - defines the algorithm skeleton
    void processData() {
        readData();
        processRawData();
        writeData();
        cleanup();
    }
    
protected:
    // Abstract methods to be implemented by subclasses
    virtual void readData() = 0;
    virtual void processRawData() = 0;
    virtual void writeData() = 0;
    
    // Hook method with default implementation
    virtual void cleanup() {
        std::cout << "Default cleanup performed\n";
    }
    
    virtual ~DataProcessor() = default;
};

// Concrete implementation for CSV processing
class CSVProcessor : public DataProcessor {
protected:
    void readData() override {
        std::cout << "Reading data from CSV file\n";
    }
    
    void processRawData() override {
        std::cout << "Processing CSV data: parsing columns, validating format\n";
    }
    
    void writeData() override {
        std::cout << "Writing processed data to CSV output file\n";
    }
    
    void cleanup() override {
        std::cout << "CSV cleanup: closing file handles, clearing buffers\n";
    }
};

// Concrete implementation for JSON processing
class JSONProcessor : public DataProcessor {
protected:
    void readData() override {
        std::cout << "Reading data from JSON file\n";
    }
    
    void processRawData() override {
        std::cout << "Processing JSON data: parsing objects, validating schema\n";
    }
    
    void writeData() override {
        std::cout << "Writing processed data to JSON output file\n";
    }
    
    void cleanup() override {
        std::cout << "JSON cleanup: releasing memory, closing connections\n";
    }
};

// Concrete implementation for XML processing
class XMLProcessor : public DataProcessor {
protected:
    void readData() override {
        std::cout << "Reading data from XML file\n";
    }
    
    void processRawData() override {
        std::cout << "Processing XML data: parsing nodes, validating against XSD\n";
    }
    
    void writeData() override {
        std::cout << "Writing processed data to XML output file\n";
    }
    
    // Using default cleanup (hook method)
};

// Usage
int main() {
    std::cout << "=== Processing CSV Data ===\n";
    CSVProcessor csvProcessor;
    csvProcessor.processData();
    
    std::cout << "\n=== Processing JSON Data ===\n";
    JSONProcessor jsonProcessor;
    jsonProcessor.processData();
    
    std::cout << "\n=== Processing XML Data ===\n";
    XMLProcessor xmlProcessor;
    xmlProcessor.processData();
    
    return 0;
}
```

---

### Visitor

**Category:** Behavioral

**Definition:** Represents an operation to be performed on elements of an object structure. Visitor lets you define a new operation without changing the classes of the elements on which it operates.

**Real-world Analogy:** Like a tax inspector visiting different types of businesses - the inspector can perform tax audits on restaurants, shops, and factories without the businesses needing to know how to conduct tax audits themselves.

**C++ Implementation:**

```cpp
#include <iostream>
#include <vector>
#include <memory>

// Forward declarations
class Book;
class DVD;
class Software;

// Visitor interface
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visit(Book& book) = 0;
    virtual void visit(DVD& dvd) = 0;
    virtual void visit(Software& software) = 0;
};

// Element interface
class ShoppingItem {
public:
    virtual ~ShoppingItem() = default;
    virtual void accept(Visitor& visitor) = 0;
    virtual double getPrice() const = 0;
    virtual std::string getName() const = 0;
};

// Concrete elements
class Book : public ShoppingItem {
private:
    std::string title;
    double price;
    
public:
    Book(const std::string& title, double price) : title(title), price(price) {}
    
    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }
    
    double getPrice() const override { return price; }
    std::string getName() const override { return title; }
    std::string getTitle() const { return title; }
};

class DVD : public ShoppingItem {
private:
    std::string title;
    double price;
    int duration; // in minutes
    
public:
    DVD(const std::string& title, double price, int duration) 
        : title(title), price(price), duration(duration) {}
    
    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }
    
    double getPrice() const override { return price; }
    std::string getName() const override { return title; }
    std::string getTitle() const { return title; }
    int getDuration() const { return duration; }
};

class Software : public ShoppingItem {
private:
    std::string name;
    double price;
    std::string version;
    
public:
    Software(const std::string& name, double price, const std::string& version)
        : name(name), price(price), version(version) {}
    
    void accept(Visitor& visitor) override {
        visitor.visit(*this);
    }
    
    double getPrice() const override { return price; }
    std::string getName() const override { return name; }
    std::string getVersion() const { return version; }
};

// Concrete visitors
class TaxCalculatorVisitor : public Visitor {
private:
    double totalTax = 0.0;
    
public:
    void visit(Book& book) override {
        double tax = book.getPrice() * 0.05; // 5% tax on books
        totalTax += tax;
        std::cout << "Book '" << book.getTitle() << "': $" << book.getPrice() 
                  << " (Tax: $" << tax << ")\n";
    }
    
    void visit(DVD& dvd) override {
        double tax = dvd.getPrice() * 0.10; // 10% tax on DVDs
        totalTax += tax;
        std::cout << "DVD '" << dvd.getTitle() << "': $" << dvd.getPrice() 
                  << " (Tax: $" << tax << ")\n";
    }
    
    void visit(Software& software) override {
        double tax = software.getPrice() * 0.15; // 15% tax on software
        totalTax += tax;
        std::cout << "Software '" << software.getName() << "': $" << software.getPrice() 
                  << " (Tax: $" << tax << ")\n";
    }
    
    double getTotalTax() const { return totalTax; }
};

class ShippingCalculatorVisitor : public Visitor {
private:
    double totalShipping = 0.0;
    
public:
    void visit(Book& book) override {
        double shipping = 2.0; // Fixed shipping for books
        totalShipping += shipping;
        std::cout << "Book '" << book.getTitle() << "': Shipping $" << shipping << "\n";
    }
    
    void visit(DVD& dvd) override {
        double shipping = 3.0; // Fixed shipping for DVDs
        totalShipping += shipping;
        std::cout << "DVD '" << dvd.getTitle() << "': Shipping $" << shipping << "\n";
    }
    
    void visit(Software& software) override {
        double shipping = 0.0; // Digital download - no shipping
        totalShipping += shipping;
        std::cout << "Software '" << software.getName() << "': Shipping $" << shipping 
                  << " (Digital download)\n";
    }
    
    double getTotalShipping() const { return totalShipping; }
};

class InventoryVisitor : public Visitor {
private:
    int bookCount = 0;
    int dvdCount = 0;
    int softwareCount = 0;
    
public:
    void visit(Book& book) override {
        bookCount++;
        std::cout << "Inventory: Book '" << book.getTitle() << "' counted\n";
    }
    
    void visit(DVD& dvd) override {
        dvdCount++;
        std::cout << "Inventory: DVD '" << dvd.getTitle() << "' (" 
                  << dvd.getDuration() << " min) counted\n";
    }
    
    void visit(Software& software) override {
        softwareCount++;
        std::cout << "Inventory: Software '" << software.getName() 
                  << "' v" << software.getVersion() << " counted\n";
    }
    
    void printSummary() {
        std::cout << "\nInventory Summary:\n";
        std::cout << "Books: " << bookCount << "\n";
        std::cout << "DVDs: " << dvdCount << "\n";
        std::cout << "Software: " << softwareCount << "\n";
        std::cout << "Total items: " << (bookCount + dvdCount + softwareCount) << "\n";
    }
};

// Usage
int main() {
    // Create shopping cart
    std::vector<std::unique_ptr<ShoppingItem>> cart;
    cart.push_back(std::make_unique<Book>("Design Patterns", 45.99));
    cart.push_back(std::make_unique<DVD>("The Matrix", 19.99, 136));
    cart.push_back(std::make_unique<Software>("Visual Studio", 299.99, "2022"));
    cart.push_back(std::make_unique<Book>("Clean Code", 39.99));
    
    // Calculate taxes
    std::cout << "=== Tax Calculation ===\n";
    TaxCalculatorVisitor taxCalculator;
    for (auto& item : cart) {
        item->accept(taxCalculator);
    }
    std::cout << "Total Tax: $" << taxCalculator.getTotalTax() << "\n\n";
    
    // Calculate shipping
    std::cout << "=== Shipping Calculation ===\n";
    ShippingCalculatorVisitor shippingCalculator;
    for (auto& item : cart) {
        item->accept(shippingCalculator);
    }
    std::cout << "Total Shipping: $" << shippingCalculator.getTotalShipping() << "\n\n";
    
    // Inventory count
    std::cout << "=== Inventory Count ===\n";
    InventoryVisitor inventory;
    for (auto& item : cart) {
        item->accept(inventory);
    }
    inventory.printSummary();
    
    return 0;
}
```

---

## Summary

This document covers all 23 Gang of Four design patterns, organized into three categories:

### 🏗️ **Creational Patterns (5)**
Focus on object creation mechanisms:
- **Singleton** - One instance only
- **Factory Method** - Create objects without specifying exact classes
- **Abstract Factory** - Create families of related objects
- **Builder** - Construct complex objects step by step
- **Prototype** - Create objects by cloning existing instances

### 🧱 **Structural Patterns (7)**
Focus on object composition and relationships:
- **Adapter** - Make incompatible interfaces work together
- **Bridge** - Separate abstraction from implementation
- **Composite** - Treat individual objects and compositions uniformly
- **Decorator** - Add behavior to objects dynamically
- **Facade** - Provide simplified interface to complex subsystem
- **Flyweight** - Share common data efficiently among multiple objects
- **Proxy** - Provide placeholder/surrogate for another object

### 🔄 **Behavioral Patterns (11)**
Focus on communication between objects:
- **Chain of Responsibility** - Pass requests along a chain of handlers
- **Command** - Encapsulate requests as objects
- **Interpreter** - Define grammar representation and interpreter
- **Iterator** - Access elements sequentially without exposing representation
- **Mediator** - Define how objects interact with each other
- **Memento** - Capture and restore object state
- **Observer** - Notify multiple objects about state changes
- **State** - Change object behavior when internal state changes
- **Strategy** - Make algorithms interchangeable
- **Template Method** - Define algorithm skeleton, let subclasses override steps
- **Visitor** - Define operations on object structures without changing classes

Each pattern includes:
- ✅ **Category** and **definition**
- ✅ **Real-world analogy** for easy understanding
- ✅ **Complete C++ implementation** with proper modern C++ practices
- ✅ **Usage examples** demonstrating the pattern in action
- ✅ **Bad usage examples** where applicable

These patterns provide proven solutions to common software design problems and help create more maintainable, flexible, and reusable code. 