#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>

template <typename T> struct Node {
  T data;
  Node *next = nullptr;
  explicit Node(const T& val) : data(val) {}
};

template <typename T> class LinkedList {
private:
  Node<T> *head_ = nullptr;
  Node<T> *tail_ = nullptr;
  size_t size_ = 0;

public:
  ~LinkedList() {
    while (head_) {
      Node<T> *temp = head_;
      head_ = head_->next;
      delete temp;
    }
  }

  void append(const T &value) {
    auto *node = new Node<T>(value);
    if (!tail_) {
      head_ = tail_ = node;
    } else {
      tail_->next = node;
      tail_ = node;
    }
    ++size_;
  }

  // STL 风格迭代器--内部类
  class Iterator {
  private:
    Node<T> *current_;

  public:
    explicit Iterator(Node<T>* node) : current_(node) {}

    // 解引用
    T& operator*() { return current_->data; }

    // 前进
    Iterator &operator++() {
      current_ = current_->next;
      return *this;
    }

    // 比较
    bool operator!=(const Iterator &other) const {
      return current_ != other.current_;
    }
  };

  // begin/end支持 range-based for
  Iterator begin() { return Iterator(head_); }
  Iterator end() { return Iterator(nullptr); }
};

int main() {
  LinkedList<std::string> list;
  list.append("Hello");
  list.append("Iterator");
  list.append("Pattern");

  for (const auto &item : list) {
    std::cout << item << " ";
  }

  std::cout << "\n";

  return 0;
}
