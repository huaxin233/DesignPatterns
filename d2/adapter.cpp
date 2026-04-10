#include <iostream>
#include <memory>
#include <string>

// ========== 目标接口 ==========
// 项目内部统一的支付接口——所有支付方式都要符合这个接口
class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;
    virtual bool pay(const std::string& orderId, double amount) = 0;
    virtual std::string getName() const = 0;
};

// ========== 第三方 SDK（无法修改） ==========

// 支付宝 SDK——接口是 sendPayment(订单号, 金额, 币种)
class AlipaySDK {
public:
    int sendPayment(const std::string& tradeNo, double yuan, const std::string& currency) {
        std::cout << "Alipay: Processing trade " << tradeNo
                  << ", amount=" << yuan << " " << currency << "\n";
        return 0;  // 0 表示成功
    }
};

// 微信支付 SDK——接口是 unifiedOrder(json 字符串)
class WechatPaySDK {
public:
    bool unifiedOrder(const std::string& jsonBody) {
        std::cout << "WechatPay: Unified order with body: " << jsonBody << "\n";
        return true;  // true 表示成功
    }
};

// Stripe SDK——接口是 createCharge(金额以"分"为单位, 币种, 描述)
class StripeSDK {
public:
    std::string createCharge(int amountInCents, const std::string& currency,
                             const std::string& description) {
        std::cout << "Stripe: Charging " << amountInCents << " cents ("
                  << currency << ") for: " << description << "\n";
        return "ch_success_123";  // 返回 charge ID
    }
};

// ========== 适配器 ==========

// 支付宝适配器
class AlipayAdapter : public PaymentProcessor {
private:
    std::unique_ptr<AlipaySDK> sdk_;

public:
    AlipayAdapter() : sdk_(std::unique_ptr<AlipaySDK>(new AlipaySDK())) {}

    bool pay(const std::string& orderId, double amount) override {
        // 将统一接口转换为支付宝 SDK 的接口
        int result = sdk_->sendPayment(orderId, amount, "CNY");
        return result == 0;
    }

    std::string getName() const override { return "Alipay"; }
};

// 微信支付适配器
class WechatPayAdapter : public PaymentProcessor {
private:
    std::unique_ptr<WechatPaySDK> sdk_;

public:
    WechatPayAdapter() : sdk_(std::unique_ptr<WechatPaySDK>(new WechatPaySDK())) {}

    bool pay(const std::string& orderId, double amount) override {
        // 构造微信支付需要的 JSON 格式
        std::string json = "{\"order_id\":\"" + orderId
                         + "\",\"amount\":" + std::to_string(amount) + "}";
        return sdk_->unifiedOrder(json);
    }

    std::string getName() const override { return "WechatPay"; }
};

// Stripe 适配器
class StripeAdapter : public PaymentProcessor {
private:
    std::unique_ptr<StripeSDK> sdk_;

public:
    StripeAdapter() : sdk_(std::unique_ptr<StripeSDK>(new StripeSDK())) {}

    bool pay(const std::string& orderId, double amount) override {
        // Stripe 要求金额以"分"为单位
        int cents = static_cast<int>(amount * 100);
        std::string chargeId = sdk_->createCharge(cents, "USD", "Order: " + orderId);
        return !chargeId.empty();
    }

    std::string getName() const override { return "Stripe"; }
};

// ========== 客户端代码 ==========

// 客户端只依赖 PaymentProcessor 接口——不知道背后是哪个 SDK
void checkout(PaymentProcessor& processor,
              const std::string& orderId, double amount) {
    std::cout << "Using " << processor.getName() << " to pay...\n";
    bool success = processor.pay(orderId, amount);
    std::cout << "Result: " << (success ? "SUCCESS" : "FAILED") << "\n\n";
}

int main() {
    AlipayAdapter alipay;
    WechatPayAdapter wechat;
    StripeAdapter stripe;

    checkout(alipay, "ORD-001", 99.9);
    // Output:
    // Using Alipay to pay...
    // Alipay: Processing trade ORD-001, amount=99.9 CNY
    // Result: SUCCESS

    checkout(wechat, "ORD-002", 49.5);
    // Output:
    // Using WechatPay to pay...
    // WechatPay: Unified order with body: {"order_id":"ORD-002","amount":49.500000}
    // Result: SUCCESS

    checkout(stripe, "ORD-003", 29.99);
    // Output:
    // Using Stripe to pay...
    // Stripe: Charging 2999 cents (USD) for: Order: ORD-003
    // Result: SUCCESS
}