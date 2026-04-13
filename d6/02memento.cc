#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>

// Memento--不可变的状态快照
class EditorMemento {
private:
  std::string content_;
  int cursorPosition_;

  // 只有Editor可以访问内部状态
  friend class Editor;

  EditorMemento(const std::string& content, int cursorPos) : content_(content), cursorPosition_(cursorPos) {}

public:
  // 对外只暴露描述信息，不暴露内部状态
  std::string getDescription() const {
    return "Snapshot [" + std::to_string(content_.size()) + " chars]";
  }
};

// Originator--文本编辑器
class Editor {
private:
  std::string content_;
  int cursorPosition_ = 0;

public:
  void type(const std::string &text) {
    content_.insert(cursorPosition_, text);
    cursorPosition_ += text.size();
  }

  void moveCursor(int position) {
    cursorPosition_ =
        std::max(0, std::min(position, static_cast<int>(content_.size())));
  }

  void deleteBack(int count) {
    int start = std::max(0, cursorPosition_ - count);
    content_.erase(start, cursorPosition_ - start);
    cursorPosition_ = start;
  }

  // 创建备忘录（存档）
  std::unique_ptr<EditorMemento> save() const {
    return std::unique_ptr<EditorMemento>(
        new EditorMemento(content_, cursorPosition_));
  }

  // 从备忘录恢复（读档）
  void restore(const EditorMemento &memento) {
    content_ = memento.content_;
    cursorPosition_ = memento.cursorPosition_;
  }

  void display() const {
    std::cout << "Content: \"" << content_ << "\" | Cursor: " << cursorPosition_
              << "\n";
  }
};

// Caretaker--管理备忘录栈
class UndoManager {
private:
  std::vector<std::unique_ptr<EditorMemento>> history_;
  int maxHistory_;

public:
  explicit UndoManager(int maxHistory = 50) : maxHistory_(maxHistory) {}

  void save(std::unique_ptr<EditorMemento> memento) {
    if (static_cast<int>(history_.size()) >= maxHistory_) {
      history_.erase(history_.begin());
    }
    history_.push_back(std::move(memento));
  }

  // 弹出最近一次备忘录
  std::unique_ptr<EditorMemento> undo() {
    if (history_.empty()) {
      std::cout << "Nothing to undo.\n";
      return nullptr;
    }
    auto memento = std::move(history_.back());
    history_.pop_back();
    return memento;
  }

  bool canUndo() const { return !history_.empty(); }

  void showHistory() const {
    std::cout << "History (" << history_.size() << " snapshots):\n";
    for (size_t i = 0; i < history_.size(); ++i) {
      std::cout << "  [" << i << "] " << history_[i]->getDescription() << "\n";
    }
  }
};

int main() {
  Editor editor;
  UndoManager undoMgr;

  // 输入文本并保存快照
  undoMgr.save(editor.save());
  editor.type("Hello");
  editor.display();

  undoMgr.save(editor.save());
  editor.type(" World");
  editor.display();

  undoMgr.save(editor.save());
  editor.type("!!!");
  editor.display();

  // 撤销操作
  undoMgr.showHistory();

  if (auto m = undoMgr.undo()) {
    editor.restore(*m);
    editor.display();
  }

  if (auto m = undoMgr.undo()) {
    editor.restore(*m);
    editor.display();
  }

  return 0;
}
