#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

class Transport {
public:
  virtual ~Transport() = default;
  virtual void deliver() const = 0;
};

class Truck : public Transport {
public:
  void deliver() const override {
    std::cout << "Delivering by land in a truck.\n";
  }
};

class Ship : public Transport {
public:
  void deliver() const override {
    std::cout << "Delivering by sea in a ship.\n";
  }
};

class Airplane : public Transport {
public:
  void deliver() const override {
    std::cout << "Delivering by air in an airplane.\n";
  }
};

// Creator
class Logistics {
public:
  virtual ~Logistics() = default;

  virtual std::unique_ptr<Transport> createTransport() const = 0;

  void planDelivery() const {
    auto transport = createTransport();
    transport->deliver();
  }
};

class RoadLogistics : public Logistics {
public:
  std::unique_ptr<Transport> createTransport() const override {
    return std::make_unique<Truck>();
  }
};

class SeaLogistics : public Logistics {
public:
  std::unique_ptr<Transport> createTransport() const override {
    return std::make_unique<Ship>();
  }
};

class AirLogistics : public Logistics {
public:
  std::unique_ptr<Transport> createTransport() const override {
    return std::make_unique<Airplane>();
  }
};

int main() {
  std::unique_ptr<Logistics> logistics;

  std::string type = "road";
  if (type == "road") {
    logistics = std::make_unique<RoadLogistics>();
  } else if (type == "sea") {
    logistics = std::make_unique<SeaLogistics>();
  } else if (type == "air") {
    logistics = std::make_unique<AirLogistics>();
  }

  logistics->planDelivery();

  return 0;
}
