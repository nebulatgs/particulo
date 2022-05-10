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
   Particle(int index) : index(index) {}
   Particle(int index, int width, int height) : index(index), pos(0, width, 0, height) {}
   const int index;
   v2d::v2d pos = v2d::v2d(0, 100, 0, 100);
   v2d::v2d prev_pos = pos;
   v2d::v2d acc = v2d::v2d(0, 1, 0, 1);
   uint32_t color = 0xbf44fcff;
   float radius = 5.0f;
};
class Example : public Particulo::Particulo<Particle>
{
   void init() override { SetBGColor(0x222f3eFF); }
   void simulate(const vector<shared_ptr<Particle>>& snapshot, const span<shared_ptr<Particle>> section, milliseconds timeElapsed) override {
      for (auto& p : section)
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

int main() {
   auto a = Example();
   a.Create<100000, 1000>(1000, 1000, "Particulo Example: Bouncing Balls");
   a.Start(8ms, 0ms);
}