#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

// 前向声明
class Paragraph;
class Heading;
class CodeBlock;
class Image;

// Visitor接口--每种元素类型对应一个visit方法
class DocumentVisitor {
public:
  virtual ~DocumentVisitor() = default;
  virtual void visit(const Paragraph &p) = 0;
  virtual void visit(const Heading &h) = 0;
  virtual void visit(const CodeBlock &c) = 0;
  virtual void visit(const Image &img) = 0;
};

// Element接口
class DocumentElement {
public:
  virtual ~DocumentElement() = default;
  virtual void accept(DocumentVisitor &visitor) const = 0;
};

// 段落
class Paragraph : public DocumentElement {
private:
  std::string text_;

public:
  explicit Paragraph(const std::string &text) : text_(text) {}

  void accept(DocumentVisitor &visitor) const override {
    visitor.visit(*this); // 第二次分派
  }

  const std::string& getText() const { return text_; }
};

// 标题
class Heading : public DocumentElement {
private:
  std::string text_;
  int level_;

public:
  Heading(const std::string &text, int level) : text_(text), level_(level) {}

  void accept(DocumentVisitor &visitor) const override {
    visitor.visit(*this);
  }

  const std::string& getText() const { return text_; }
  int getLevel() const { return level_; }
};

// 代码块
class CodeBlock : public DocumentElement {
private:
  std::string code_;
  std::string language_;

public:
  CodeBlock(const std::string &code, const std::string &language)
      : code_(code), language_(language) {}

  void accept(DocumentVisitor &visitor) const override {
    visitor.visit(*this);
  }

  const std::string& getCode() const { return code_; }
  const std::string& getLanguage() const { return language_; }
};

// 图片
class Image : public DocumentElement {
private:
  std::string url_;
  std::string alt_;

public:
  Image(const std::string &url, const std::string &alt)
      : url_(url), alt_(alt) {}

  void accept(DocumentVisitor &visitor) const override {
    visitor.visit(*this);
  }

  const std::string& getUrl() const { return url_; }
  const std::string& getAlt() const { return alt_; }
};

// Visitor 1:导出为HTML
class HtmlExportVisitor : public DocumentVisitor {
private:
  std::ostringstream output_;

public:
  void visit(const Paragraph &p) override {
    output_ << "<p>" << p.getText() << "</p>\n";
  }

  void visit(const Heading &h) override {
    output_ << "<h" << h.getLevel() << ">" << h.getText() << "</h"
            << h.getLevel() << ">\n";
  }

  void visit(const CodeBlock &c) override {
    output_ << "<pre><code class=\"" << c.getLanguage() << "\">" << c.getCode()
            << "</code></pre>\n";
  }

  void visit(const Image &img) override {
    output_ << "<img src=\"" << img.getUrl() << "\" alt=\"" << img.getAlt()
            << "\" />\n";
  }

  std::string getResult() const { return output_.str(); }
};

// Visitor2：导出为Markdown
class MarkdownExportVisitor : public DocumentVisitor {
private:
  std::ostringstream output_;

public:
  void visit(const Paragraph &p) override {
    output_ << p.getText() << "\n\n";
  }

  void visit(const Heading &h) override {
    output_ << std::string(h.getLevel(), '#') << " " << h.getText() << "\n\n";
  }

  void visit(const CodeBlock &c) override {
    output_ << "```" << c.getLanguage() << "\n" << c.getCode() << "\n```\n\n";
  }

  void visit(const Image &img) override {
    output_ << "![" << img.getAlt() << "](" << img.getUrl() << ")\n\n";
  }

  std::string getResult() const { return output_.str(); }
};

// Visitor3：统计文档信息
class StatissticSVisitor : public DocumentVisitor {
private:
  int paragraphCount_ = 0;
  int headingCount_ = 0;
  int codeBlockCount_ = 0;
  int imageCount_ = 0;
  int totalChars_ = 0;

public:
  void visit(const Paragraph &p) override {
    paragraphCount_++;
    totalChars_ += p.getText().size();
  }

  void visit(const Heading &h) override {
    headingCount_++;
    totalChars_ += h.getText().size();
  }

  void visit(const CodeBlock &c) override {
    codeBlockCount_++;
    totalChars_ += c.getCode().size();
  }

  void visit(const Image &img) override { imageCount_++; }

  void printReport() const {
    std::cout << "=== Document Statistics ===\n"
              << "Paragraphs: " << paragraphCount_ << "\n"
              << "Headings:   " << headingCount_ << "\n"
              << "Code blocks:" << codeBlockCount_ << "\n"
              << "Images:     " << imageCount_ << "\n"
              << "Total chars:" << totalChars_ << "\n";
  }
};

int main() {
  // 构建文档结构
  std::vector<std::unique_ptr<DocumentElement>> doc;
  doc.push_back(std::unique_ptr<Heading>(new Heading("Design Patterns", 1)));
  doc.push_back(std::unique_ptr<Paragraph>(
      new Paragraph("Design patterns are reusable solutions.")));
  doc.push_back(std::unique_ptr<Heading>(new Heading("Visitor Pattern", 2)));
  doc.push_back(std::unique_ptr<Paragraph>(
      new Paragraph("Visitor separates algorighms from objects.")));
  doc.push_back(std::unique_ptr<CodeBlock>(
      new CodeBlock("element->accept(visitor);", "cpp")));
  doc.push_back(
      std::unique_ptr<Image>(new Image("visitor.png", "visitor UML diagram")));

  // 导出为html
  HtmlExportVisitor htmlExporter;
  for (const auto &elem : doc) {
    elem->accept(htmlExporter);
  }
  std::cout << "=== HTML ===\n" << htmlExporter.getResult() << "\n";

  // 导出为Markdown
  MarkdownExportVisitor mdExporter;
  for (const auto &elem : doc) {
    elem->accept(mdExporter);
  }
  std::cout << "=== Markdown ===\n" << mdExporter.getResult() << "\n";

  // 统计信息
  StatissticSVisitor stats;
  for (const auto &elem : doc) {
    elem->accept(stats);
  }
  stats.printReport();

  return 0;
}
