#include <iostream>
#include <string>
#include <vector>

class HttpRequest {
public:
  // 公开字段，由builder设置
  std::string method;
  std::string url;
  std::vector<std::pair<std::string, std::string>> headers;
  std::string body;
  bool hasBody_ = false;
  int timeout_ms = 30000;

  void send() const {
    std::cout << method << " " << url << "\n";
    for (const auto& [key, value] : headers) {
      std::cout << " " << key << ": " << value << "\n";
    }
    if (hasBody_) {
      std::cout << " Body: " << body << "\n";
    }
    std::cout << " Timeout: " << timeout_ms << "ms\n";
  }
};

// 链式builder-每个方法返回自身引用
class HttpRequestBuilder {
private:
  HttpRequest request_;

public:
  HttpRequestBuilder& setMethod(const std::string &method) {
    request_.method = method;
    return *this;
  }

  HttpRequestBuilder& setUrl(const std::string &url) {
    request_.url = url;
    return *this;
  }

  HttpRequestBuilder& addHeader(const std::string &key,
                                const std::string &value) {
    request_.headers.emplace_back(key, value);
    return *this;
  }

  HttpRequestBuilder &setBody(const std::string &b) {
    request_.body = b;
    request_.hasBody_ = true;
    return *this;
  }

  HttpRequestBuilder &setTimeout(int ms) {
    request_.timeout_ms = ms;
    return *this;
  }

  HttpRequest build() {
    // 可以在这里做参数校验
    if (request_.method.empty()) {
      request_.method = "GET";
    }
    return std::move(request_);
  }
};

int main() {
  auto request = HttpRequestBuilder()
                     .setMethod("POST")
                     .setUrl("https://www.baidu.com")
                     .addHeader("Content-Type", "application/json")
                     .addHeader("Authorization", "Bearer token123")
                     .setBody(R"({"name": "Alice"})")
                     .setTimeout(5000)
                     .build();

  request.send();

  return 0;
}
