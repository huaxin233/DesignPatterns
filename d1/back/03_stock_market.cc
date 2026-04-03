// 课后练习 3：股票价格监控系统
// g++ -std=c++11 -Wall -Wextra -o 03_stock_market 03_stock_market.cc

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

// ========== 观察者接口 ==========

class StockObserver {
public:
    virtual ~StockObserver() = default;
    virtual void onPriceChange(const std::string& symbol,
                               double oldPrice, double newPrice) = 0;
    virtual std::string getName() const = 0;
};

// ========== Subject：按股票代码订阅 ==========

class StockMarket {
private:
    std::unordered_map<std::string, double> prices_;
    std::unordered_map<std::string, std::vector<StockObserver*>> observers_;

public:
    void subscribe(const std::string& symbol, StockObserver* observer) {
        observers_[symbol].push_back(observer);
        std::cout << observer->getName() << " subscribed to " << symbol << "\n";
    }

    void unsubscribe(const std::string& symbol, StockObserver* observer) {
        auto& list = observers_[symbol];
        list.erase(std::remove(list.begin(), list.end(), observer), list.end());
        std::cout << observer->getName() << " unsubscribed from " << symbol << "\n";
    }

    void setPrice(const std::string& symbol, double price) {
        double oldPrice = prices_.count(symbol) ? prices_[symbol] : 0.0;
        prices_[symbol] = price;
        std::cout << "\n--- " << symbol << ": $" << oldPrice
                  << " -> $" << price << " ---\n";

        auto it = observers_.find(symbol);
        if (it != observers_.end()) {
            for (auto* obs : it->second) {
                obs->onPriceChange(symbol, oldPrice, price);
            }
        }
    }
};

// ========== Observer A：价格显示 ==========

class PriceDisplay : public StockObserver {
    std::string name_;
public:
    explicit PriceDisplay(const std::string& name) : name_(name) {}

    void onPriceChange(const std::string& symbol,
                       double /*oldPrice*/, double newPrice) override {
        std::cout << "[Display:" << name_ << "] "
                  << symbol << " = $" << newPrice << "\n";
    }

    std::string getName() const override { return "Display:" + name_; }
};

// ========== Observer B：自动交易——价格低于阈值时买入 ==========

class TradeBot : public StockObserver {
    std::string name_;
    double buyThreshold_;
public:
    TradeBot(const std::string& name, double threshold)
        : name_(name), buyThreshold_(threshold) {}

    void onPriceChange(const std::string& symbol,
                       double /*oldPrice*/, double newPrice) override {
        if (newPrice < buyThreshold_) {
            std::cout << "[TradeBot:" << name_ << "] BUY " << symbol
                      << " at $" << newPrice
                      << " (threshold: $" << buyThreshold_ << ")\n";
        }
    }

    std::string getName() const override { return "TradeBot:" + name_; }
};

// ========== Observer C：价格变化超过 5% 时邮件告警 ==========

class EmailAlert : public StockObserver {
    std::string email_;
public:
    explicit EmailAlert(const std::string& email) : email_(email) {}

    void onPriceChange(const std::string& symbol,
                       double oldPrice, double newPrice) override {
        if (oldPrice > 0.0) {
            double pct = std::abs(newPrice - oldPrice) / oldPrice * 100.0;
            if (pct > 5.0) {
                std::cout << "[Email:" << email_ << "] ALERT " << symbol
                          << " changed " << pct << "% ($"
                          << oldPrice << " -> $" << newPrice << ")\n";
            }
        }
    }

    std::string getName() const override { return "Email:" + email_; }
};

// ========== 客户端代码 ==========

int main() {
    StockMarket market;

    PriceDisplay display("Terminal");
    TradeBot     bot("AlphaBot", 150.0);
    EmailAlert   alert("alice@example.com");

    // 按股票代码订阅——不同观察者关注不同股票
    market.subscribe("AAPL", &display);
    market.subscribe("AAPL", &bot);
    market.subscribe("AAPL", &alert);
    market.subscribe("GOOG", &display);
    market.subscribe("GOOG", &alert);
    // bot 不关注 GOOG

    // 初始价格
    market.setPrice("AAPL", 180.0);
    market.setPrice("GOOG", 2800.0);

    // AAPL 下跌 > 5%，触发 EmailAlert + TradeBot BUY
    market.setPrice("AAPL", 145.0);

    // GOOG 小幅变动 < 5%，仅 display 输出
    market.setPrice("GOOG", 2750.0);

    // 取消 bot 对 AAPL 的订阅
    market.unsubscribe("AAPL", &bot);

    // 再次下跌——bot 不再收到通知
    market.setPrice("AAPL", 140.0);
}
