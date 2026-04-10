#include <iostream>
#include <iterator>
#include <string>
#include <fstream>

// 抽象类--定义数据挖掘的算法骨架
class DataMiner {
public:
  virtual ~DataMiner() = default;

  // 模板方法--定义算法骨架，final禁止子类覆盖
  virtual void mine(const std::string &path) final {
    std::string rawData = openFile(path);
    std::string data = extractData(rawData);
    std::string analysis = analyzeData(data);
    sendReport(analysis);
    std::cout << "---\n";
  }

protected:
  // 需要子类实现的抽象步骤
  virtual std::string openFile(const std::string &path) = 0;
  virtual std::string extractData(const std::string &rawData) = 0;

  // 可选步骤（钩子）--子类可以覆盖，但不强制
  virtual std::string analyzeData(const std::string &data) {
    std::cout << "Default analysis on: " << data << "\n";
    return "Analysis of [" + data + "]";
  }

  // 固定步骤--子类不需要覆盖
  void sendReport(const std::string &analysis) {
    std::cout << "Report sent: " << analysis << "\n";
  }
};

// 具体子类 A：PDF数据挖掘
class PDFMiner : public DataMiner {
protected:
  std::string openFile(const std::string &path) override {
    std::cout << "Opening PDF file: " << path << "\n";
    return "PDF raw content from " + path;
  }

  std::string extractData(const std::string &rawData) override {
    std::cout << "Extracting text from PDF...\n";
    return "PDF extracted data";
  }

  // 覆盖钩子方法--自定义分析逻辑
  std::string analyzeData(const std::string &data) override {
    std::cout << "PDF-specific analysis on: " << data << "\n";
    return "PDF Analysis of [" + data + "]";
  }
};

// 具体子类 B：CSV数据挖掘
class CSVMiner : public DataMiner {
protected:
  std::string openFile(const std::string &path) override {
    std::cout << "Opening CSV file: " << path << "\n";
    return "CSV raw content from " + path;
  }

  std::string extractData(const std::string &rawData) override {
    std::cout << "Parsing CSV rows...\n";
    return "CSV extracted data";
  }

  // 使用默认的 analyzeData
};


// 具体子类 C：JSON数据挖掘
class JSONMiner : public DataMiner {
protected:
  std::string openFile(const std::string &path) override {
    std::cout << "Opening JSON file: " << path << "\n";
    return "JSON raw content from " + path;
  }

  std::string extractData(const std::string &rawData) override {
    std::cout << "Parsing JSON rows...\n";
    return "JSON extracted data";
  }

  // 使用默认的 analyzeData
};

int main() {
  PDFMiner pdf;
  CSVMiner csv;
  JSONMiner json;

  std::cout << "=== PDF Mining ===\n";
  pdf.mine("report.pdf");

  std::cout << "=== CSV Mining ===\n";
  csv.mine("data.csv");

  std::cout << "=== JSON Mining ===\n";
  json.mine("config.json");

  return 0;
}

