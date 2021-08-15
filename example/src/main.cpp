#include <iostream>
#include <memory>
#include <particulo/particulo.hpp>

class Example : public Particulo {
  void init() override {
    std::cout << "void init() override called!" << std::endl;
    std::cout << _title << std::endl;
  }
  void simulate(float timeElapsed) override {}
};

int main() {
  auto a = std::make_unique<Example>();
  a->Create(300, 300);
  std::cout << "Hello, World!" << std::endl;
}