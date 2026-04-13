#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stack>

// 接收者--文本编辑器（实际执行操作的对象）
class TextEditor {
private:
  std::string text_;

public:
  void insertText(std::size_t pos, const std::string &text) {
    if (pos > text_.size())
      pos = text_.size();
    text_.insert(pos, text);
    std::cout << "Insert '" << text << "' at pos " << pos << "\n";
  }

  void deleteText(size_t pos, size_t length) {
    if (pos >= text_.size())
      return;
    length = std::min(length, text_.size() - pos);
    std::cout << "Delete '" << text_.substr(pos, length) << "' at pos " << pos
              << "\n";
    text_.erase(pos, length);
  }

  const std::string& getText() const { return text_; }

  void display() const {
    std::cout << "Text: \"" << text_ << "\"\n";
  }
};

// 命令抽象基类
class Command {
public:
  virtual ~Command() = default;
  virtual void execute() = 0;
  virtual void undo() = 0;
};

// 具体命令：插入文本
class InsertCommand : public Command {
private:
  TextEditor &editor_;
  size_t pos_;
  std::string text_;

public:
  InsertCommand(TextEditor &editor, size_t pos, const std::string &text)
      : editor_(editor), pos_(pos), text_(text) {}

  void execute() override {
    editor_.insertText(pos_, text_);
  }

  void undo() override {
    editor_.deleteText(pos_, text_.size());
  }
};

// 具体命令：删除文本
class DeleteCommand : public Command {
private:
  TextEditor &editor_;
  size_t pos_;
  size_t length_;
  std::string deleteText_; // 保存被删除的文本，用于撤销

public:
  DeleteCommand(TextEditor &editor, size_t pos, size_t length)
      : editor_(editor), pos_(pos), length_(length) {}

  void execute() override {
    // 保存即将被删除的文本
    const auto &text = editor_.getText();
    size_t actualLen =
        std::min(length_, text.size() - std::min(pos_, text.size()));
    if (pos_ < text.size()) {
      deleteText_ = text.substr(pos_, actualLen);
    }
    editor_.deleteText(pos_, length_);
  }

  void undo() override {
    editor_.insertText(pos_, deleteText_);
  }
};

// 调用者--命令历史管理器
class CommandManager {
private:
  std::stack <std::unique_ptr<Command>> undoStack_;
  std::stack<std::unique_ptr<Command>> redoStack_;

public:
  void executeCommad(std::unique_ptr<Command> cmd) {
    cmd->execute();
    undoStack_.push(std::move(cmd));
    // 执行命令后清空重做栈
    while(!redoStack_.empty()) redoStack_.pop();
  }

  void undo() {
    if (undoStack_.empty()) {
      std::cout << "Nothing to undo.\n";
      return;
    }
    auto &cmd = undoStack_.top();
    cmd->undo();
    redoStack_.push(std::move(cmd));
    undoStack_.pop();
  }

  void redo() {
    if (redoStack_.empty()) {
      std::cout << "Nothing to redo.\n";
      return;
    }
    auto &cmd = redoStack_.top();
    cmd->execute();
    undoStack_.push(std::move(cmd));
    redoStack_.pop();
  }
};

int main() {
  TextEditor editor;
  CommandManager manager;

  // 插入文本
  manager.executeCommad(
      std::unique_ptr<InsertCommand>(new InsertCommand(editor, 0, "Hello")));
  editor.display();

  manager.executeCommad(
      std::unique_ptr<InsertCommand>(new InsertCommand(editor, 5, " World")));
  editor.display();

  // 删除文本
  manager.executeCommad(
      std::unique_ptr<DeleteCommand>(new DeleteCommand(editor, 5, 6)));
  editor.display();

  // 撤销
  std::cout << "\n--- Undo ---\n";
  manager.undo(); // 撤销删除
  editor.display();

  manager.undo(); // 撤销第二次插入
  editor.display();

  // 重做
  std::cout << "\n--- Redo ---\n";
  manager.redo(); // 重做第二次插入
  editor.display();

  return 0;
}
