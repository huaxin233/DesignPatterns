#include <iostream>
#include <memory>
#include <string>

// ========== 组件接口 ==========
class MessageSender {
public:
    virtual ~MessageSender() = default;
    virtual void send(const std::string& message) = 0;
    virtual std::string describe() const = 0;
};

// ========== 具体组件：基础发送器 ==========
class PlainSender : public MessageSender {
public:
    void send(const std::string& message) override {
        std::cout << "  Sending: " << message << "\n";
    }

    std::string describe() const override { return "PlainSender"; }
};

// ========== 装饰器基类 ==========
// 关键：装饰器本身也是 MessageSender（is-a），同时持有另一个 MessageSender（has-a）
class MessageDecorator : public MessageSender {
protected:
    std::unique_ptr<MessageSender> wrapped_;

public:
    explicit MessageDecorator(std::unique_ptr<MessageSender> sender)
        : wrapped_(std::move(sender)) {}

    void send(const std::string& message) override {
        wrapped_->send(message);  // 默认：委托给被包装对象
    }

    std::string describe() const override {
        return wrapped_->describe();
    }
};

// ========== 具体装饰器 ==========

// 装饰器 A：加密
class EncryptionDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        // 在发送前加密（模拟：每个字符 +1）
        std::string encrypted;
        for (char c : message) encrypted += static_cast<char>(c + 1);
        std::cout << "  [Encrypt] " << message << " -> " << encrypted << "\n";
        wrapped_->send(encrypted);  // 将加密后的消息传给下一层
    }

    std::string describe() const override {
        return "Encrypted(" + wrapped_->describe() + ")";
    }
};

// 装饰器 B：压缩
class CompressionDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        // 模拟压缩：只保留前 10 个字符 + "..."
        std::string compressed = message;
        if (compressed.size() > 10) {
            compressed = compressed.substr(0, 10) + "...";
        }
        std::cout << "  [Compress] " << message.size()
                  << " bytes -> " << compressed.size() << " bytes\n";
        wrapped_->send(compressed);
    }

    std::string describe() const override {
        return "Compressed(" + wrapped_->describe() + ")";
    }
};

// 装饰器 C：签名
class SignatureDecorator : public MessageDecorator {
private:
    std::string signer_;

public:
    SignatureDecorator(std::unique_ptr<MessageSender> sender,
                      const std::string& signer)
        : MessageDecorator(std::move(sender)), signer_(signer) {}

    void send(const std::string& message) override {
        std::string signed_msg = message + "\n  -- Signed by " + signer_;
        std::cout << "  [Sign] Added signature of " << signer_ << "\n";
        wrapped_->send(signed_msg);
    }

    std::string describe() const override {
        return "Signed(" + wrapped_->describe() + ")";
    }
};

// 装饰器 D：日志
class LoggingDecorator : public MessageDecorator {
public:
    using MessageDecorator::MessageDecorator;

    void send(const std::string& message) override {
        std::cout << "  [Log] Message length: " << message.size() << " chars\n";
        wrapped_->send(message);
        std::cout << "  [Log] Message sent successfully\n";
    }

    std::string describe() const override {
        return "Logged(" + wrapped_->describe() + ")";
    }
};

// ========== 客户端代码 ==========

int main() {
    // === 场景 1：最简单——纯文本发送 ===
    std::cout << "--- Plain sender ---\n";
    std::unique_ptr<MessageSender> plain(new PlainSender());
    plain->send("Hello World");
    // Output:
    //   Sending: Hello World

    // === 场景 2：加密 + 发送 ===
    std::cout << "\n--- Encrypted sender ---\n";
    std::unique_ptr<MessageSender> encrypted(new EncryptionDecorator(
        std::unique_ptr<MessageSender>(new PlainSender())));
    encrypted->send("Secret");
    // Output:
    //   [Encrypt] Secret -> Tfdsfu
    //   Sending: Tfdsfu

    // === 场景 3：日志 + 加密 + 签名 + 发送（多层装饰） ===
    std::cout << "\n--- Full pipeline ---\n";
    // 从内到外构建：PlainSender -> SignatureDecorator -> EncryptionDecorator -> LoggingDecorator
    std::unique_ptr<MessageSender> step1(new PlainSender());
    std::unique_ptr<MessageSender> step2(new SignatureDecorator(std::move(step1), "Alice"));
    std::unique_ptr<MessageSender> step3(new EncryptionDecorator(std::move(step2)));
    std::unique_ptr<MessageSender> fullPipeline(new LoggingDecorator(std::move(step3)));

    std::cout << "Pipeline: " << fullPipeline->describe() << "\n";
    // Output: Pipeline: Logged(Encrypted(Signed(PlainSender)))

    fullPipeline->send("Important data");
    // Output:
    //   [Log] Message length: 14 chars
    //   [Encrypt] Important data -> Jnqpsubou!ebub
    //   [Sign] Added signature of Alice
    //   Sending: Jnqpsubou!ebub
    //     -- Signed by Alice
    //   [Log] Message sent successfully
}