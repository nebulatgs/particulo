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
   Particle(int index)
       : index(index)
       , color(0xbf44fcff)
       , radius(5)
       , pos(0, 100, 0, 100)
       , vel(0, 10, 0, 10)
       , acc(0, 10, 0, 10)
   {}
   Particle(int index, int width, int height)
       : index(index)
       , color(0xbf44fcff)
       , radius(5)
       , pos(0, width, 0, height)
       , vel(0, 10, 0, 10)
       , acc(0, 10, 0, 10)
   {}
   const int index;
   v2d::v2d pos;
   v2d::v2d vel;
   v2d::v2d acc;
   uint32_t color;
   float radius;
};
class Example : public Particulo::Particulo<Particle>
{
   void init() override { SetBGColor(0x222f3eFF); }
   void simulate(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override
   {
      for (auto& p : particles)
      {
         p->color = p->index % 2 == 0 ? 0xbf44fcff : 0x007ACCff;
         p->vel += p->acc;
         p->acc *= 0.1;
         p->pos += p->vel;
         if (p->pos.x > GetWidth() || p->pos.x < 0) { p->vel.x *= -1; }
         if (p->pos.y > GetHeight() || p->pos.y < 0) { p->vel.y *= -1; }
      }
   }
};

int main()
{
   auto a = Example();
   a.Create(1000, 1000, 1000, "Particulo Example: Bouncing Balls", 100000);
   a.Start(16ms);
}