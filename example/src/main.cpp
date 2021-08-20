#include <chrono>
using namespace std::chrono_literals;

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <memory>
using std::make_unique;
using std::shared_ptr;

#include <vector>
using std::vector;

#include <particulo/particulo.hpp>

struct Particle
{
   Particle(int index) : index(index), color(0x2998C5FF), size(1) {}
   const int index;
   v2d::v2d pos;
   uint32_t color = 0xFFFFFFFF;
   float size = 1000000;
};
class Example : public Particulo::Particulo<Particle>
{
   void simulate(vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override
   {
      for (auto& x : particles)
      {
         // x->pos.x += 0.1;
         // x->pos.y += 0.1;
         cout << x->pos.x << endl;
      }
      SetBGColor(0x222f3eFF);
      // SetBGColor(0xFFFFFFFF);
   }
};

int main()
{
   auto a = make_unique<Example>();
   a->Create(600, 600, 10, "Particulo Example", 10);
   a->Start(33ms);
}