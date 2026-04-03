// 课后练习 2：文档解析器工厂
// g++ -std=c++11 -Wall -Wextra -o 02_document_factory 02_document_factory.cc

#include <iostream>
#include <memory>
#include <string>

// ========== 抽象产品 ==========

class Document {
public:
    virtual ~Document() = default;
    virtual void parse() = 0;
    virtual std::string getContent() const = 0;
};

// ========== 具体产品 ==========

class PDFDocument : public Document {
    std::string filename_;
    std::string content_;
public:
    explicit PDFDocument(const std::string& filename) : filename_(filename) {}

    void parse() override {
        content_ = "[PDF content from " + filename_ + "]";
        std::cout << "Parsing PDF: " << filename_ << "\n";
    }

    std::string getContent() const override { return content_; }
};

class WordDocument : public Document {
    std::string filename_;
    std::string content_;
public:
    explicit WordDocument(const std::string& filename) : filename_(filename) {}

    void parse() override {
        content_ = "[Word content from " + filename_ + "]";
        std::cout << "Parsing Word: " << filename_ << "\n";
    }

    std::string getContent() const override { return content_; }
};

class MarkdownDocument : public Document {
    std::string filename_;
    std::string content_;
public:
    explicit MarkdownDocument(const std::string& filename) : filename_(filename) {}

    void parse() override {
        content_ = "[Markdown content from " + filename_ + "]";
        std::cout << "Parsing Markdown: " << filename_ << "\n";
    }

    std::string getContent() const override { return content_; }
};

// ========== 抽象工厂 ==========

class DocumentFactory {
public:
    virtual ~DocumentFactory() = default;
    virtual std::unique_ptr<Document> createDocument(const std::string& filename) const = 0;
};

// ========== 具体工厂 ==========

class PDFFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(const std::string& filename) const override {
        return std::unique_ptr<Document>(new PDFDocument(filename));
    }
};

class WordFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(const std::string& filename) const override {
        return std::unique_ptr<Document>(new WordDocument(filename));
    }
};

class MarkdownFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(const std::string& filename) const override {
        return std::unique_ptr<Document>(new MarkdownDocument(filename));
    }
};

// 根据文件扩展名选择工厂（本质是一个简单工厂，生产的产品是工厂本身）
// "report.pdf" → rfind('.') 得到 dot=6 → substr(6) 得到 ".pdf" → 返回 PDFFactory
std::unique_ptr<DocumentFactory> getFactory(const std::string& filename) {
    auto dot = filename.rfind('.');          // 从右往左找 '.'，处理 "my.file.pdf" 的情况
    if (dot == std::string::npos) return nullptr;  // 无扩展名，无法判断类型

    auto ext = filename.substr(dot);         // 截取扩展名：".pdf"、".docx"、".md"
    if (ext == ".pdf")                return std::unique_ptr<DocumentFactory>(new PDFFactory());
    if (ext == ".docx" || ext == ".doc") return std::unique_ptr<DocumentFactory>(new WordFactory());
    if (ext == ".md")                 return std::unique_ptr<DocumentFactory>(new MarkdownFactory());
    return nullptr;                          // 不支持的格式
}

int main() {
    std::string files[] = {"report.pdf", "letter.docx", "README.md", "data.csv"};

    for (const auto& file : files) {
        auto factory = getFactory(file);
        if (!factory) {
            std::cout << "Unsupported format: " << file << "\n\n";
            continue;
        }
        auto doc = factory->createDocument(file);
        doc->parse();
        std::cout << "Content: " << doc->getContent() << "\n\n";
    }
}
