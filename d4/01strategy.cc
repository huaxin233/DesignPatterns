#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 策略接口--定义算法的统一接口
class SortStrategy {
public:
  virtual ~SortStrategy() = default;
  virtual void sort(std::vector<int> &data) const = 0;
  virtual std::string name() const = 0;
};

// 具体策略 A：冒泡排序
class BubbleSort : public SortStrategy {
public:
  void sort(std::vector<int> &data) const override {
    for (size_t i = 0; i < data.size(); ++i) {
      for (size_t j = 0; j + 1 < data.size() - i; ++j) {
        if (data[j] > data[j + 1]) {
          std::swap(data[j], data[j + 1]);
        }
      }
    }
  }

  std::string name() const override { return "BubbleSort"; }
};

// 具体策略 B：选择排序
class SelectionSort : public SortStrategy {
public:
  void sort(std::vector<int> &data) const override {
    for (size_t i = 0; i < data.size(); ++i) {
      size_t minIdx = i;
      for (size_t j = i + 1; j < data.size(); ++j) {
        if (data[j] < data[minIdx]) {
          minIdx = j;
        }
      }
      std::swap(data[i], data[minIdx]);
    }
  }

  std::string name() const override { return "SelectionSort"; }
};

// 具体策略 C：快速排序
class QuickSort : public SortStrategy {
public:
  void sort(std::vector<int> &data) const override {
    quickSort(data, 0, static_cast<int>(data.size()) - 1);
  }

  std::string name() const override { return "QuickSort"; }

private:
  void quickSort(std::vector<int> &data, int low, int high) const {
    if (low >= high)
      return;
    int pivot = data[high];
    int i = low;
    for (int j = low; j < high; ++j) {
      if (data[j] < pivot) {
        std::swap(data[i], data[j]);
        ++i;
      }
    }
    std::swap(data[i], data[high]);
    quickSort(data, low, i - 1);
    quickSort(data, i + 1, high);
  }
};

// 上下文--持有策略，可运行时替换
class Sorter {
private:
  std::unique_ptr<SortStrategy> strategy_;

public:
  explicit Sorter(std::unique_ptr<SortStrategy> strategy)
      : strategy_(std::move(strategy)) {}

  // 运行时切换策略
  void setStrategy(std::unique_ptr<SortStrategy> strategy) {
    strategy_ = std::move(strategy);
  }

  void performSort(std::vector<int> &data) {
    std::cout << "Sorting with " << strategy_->name() << "\n";
    strategy_->sort(data);
    for (int val : data) {
      std::cout << val << " ";
    }
    std::cout << "\n";
  }
};

int main() {
  std::vector<int> data = {5, 2, 8, 1, 9, 3};

  // 使用冒泡排序
  Sorter sorter(std::unique_ptr<BubbleSort>(new BubbleSort()));
  sorter.performSort(data);

  // 运行时切换为快速排序
  data = {5, 2, 8, 1, 9, 3};
  sorter.setStrategy(std::unique_ptr<QuickSort>(new QuickSort()));
  sorter.performSort(data);

  return 0;
}
