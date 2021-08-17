#include <chrono>
using namespace std::chrono_literals;

#include <iostream>
using std::cout;
using std::endl;

#include <memory>
using std::make_unique;

#include <vector>
using std::vector;

#include <particulo/particulo.hpp>
struct Particle
{
   double x;
   double y;
   uint32_t color;
};
class Example : public Particulo::Particulo<Particle>
{
   void simulate(vector<Particle>& particles, milliseconds timeElapsed) override
   {
      for (auto& x : particles)
      {
         x.x += 0.1;
         x.y += 0.1;
      }
      cout << timeElapsed.count() << endl;
   }
};

int main()
{
   auto a = make_unique<Example>();
   a->Create(300, 300, 10, "Particulo Example");
   a->Start(33ms);
   //  std::this_thread::sleep_for(std::chrono::seconds(10));
}