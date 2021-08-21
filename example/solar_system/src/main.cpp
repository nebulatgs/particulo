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

static std::random_device device;
static std::mt19937 gen = std::mt19937(device());

static float constexpr G = 6.6742E-11;
static float constexpr SizeRatio = 750000.0;
static float constexpr GCONSTANT = 1.00E3;
static float constexpr TIMESTEP = 0.001;

struct Particle
{
   Particle(int index) : index(index) {}
   Particle(int index, int width, int height, float mass)
       : index(index), pos(0, width, 0, height), mass(mass), radius(mass / SizeRatio * 10.0f)
   {}

   const int index;
   v2d::v2d pos;
   v2d::v2d vel;
   v2d::v2d force;
   uint32_t color = 0x00b894ff;
   float mass;
   float radius;
};
class Example : public Particulo::Particulo<Particle>
{
public:
   void init() override
   {
      SetBGColor(0x222f3eFF);
      std::uniform_real_distribution<float> massDistr(0, SizeRatio);
      for (int i = 0; i < 1000; i++) { Add(GetWidth(), GetHeight(), massDistr(gen)); }
   }
   void simulate(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override
   {
      for (auto& each : particles)
      {
         for (auto& particle : particles)
         {
            float distance = sqrtf(particle->pos.sqrDist(each->pos));
            if (distance > 4.0)
            {
               float pairForce =
                   ((particle->mass * each->mass) / (GCONSTANT * (distance * distance)));
               auto F = ((particle->pos - each->pos) / distance) * pairForce;
               each->force += F;
               particle->force -= F;
            }
         }
      }
   }
   void update(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override
   {
      for (auto& each : particles)
      {
         each->vel += each->force / each->mass;
         each->pos += each->vel * TIMESTEP;
         each->force = {0, 0};
      }
   }
};

int main()
{
   auto a = Example();
   a.Create(1000, 1000, 0, "Particulo Example: Solar System", 100000);
   a.Start(16ms);
}