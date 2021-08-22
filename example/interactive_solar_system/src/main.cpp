#include <chrono>
using namespace std::chrono_literals;

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

#include <memory>
using std::shared_ptr;

#include <vector>
using std::vector;

#include <particulo/particulo.hpp>

static std::random_device device;
static std::mt19937 gen = std::mt19937(device());

static float constexpr G = 6.6742E-11;
static float constexpr SizeRatio = 750000.0;
static float constexpr GCONSTANT = 1.00E3;
static float constexpr TIMESTEP = 1;

struct Particle
{
   Particle(int index) : index(index) {}
   Particle(int index, int width, int height, float mass)
       : index(index), pos(0, width, 0, height), mass(mass), radius(cbrt(mass))
   {}
   Particle(int index, v2d::v2d pos, float mass, bool disabled = true)
       : index(index), pos(pos), mass(mass), radius(cbrt(mass)), disabled(disabled)
   {}

   const int index;
   v2d::v2d pos;
   v2d::v2d vel;
   v2d::v2d force;
   uint32_t color = 0x00b894bb;
   float mass;
   float radius;
   bool disabled = false;
};
class Example : public Particulo::Particulo<Particle>
{
public:
   void init() override { SetBGColor(0x222f3eFF); }
   void simulate(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override
   {
      for (auto& each : particles)
      {
         if (each->disabled) { continue; }
         for (auto& particle : particles)
         {
            if (particle->disabled) { continue; }
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
      SetTransform(scale * translation);
      if (newParticle && rightMouseDown) newParticle->radius += 1.0;
   }
   void onScroll(double x, double y) override
   {
      auto transform = GetTransform();
      if (y > 0)
      {
         scale = glm::scale(scale, {1.333f, 1.333f, 1.0f});
         sf *= 1.333;
      }
      else
      {
         scale = glm::scale(scale, {0.75f, 0.75f, 1.0f});
         sf *= 0.75;
      }
   }
   void onMouseClick(int button, int action, int mods) override
   {
      auto [x, y] = GetMousePos();
      auto vec = glm::dvec4(x, y, 1.0, 1.0);
      vec = glm::affineInverse(translation * scale) * vec;
      // vec = glm::affineInverse(translation) * vec;
      leftMouseDown = button == 0 && action;
      leftMouseUp = button == 0 && !action;
      rightMouseDown = button == 1 && action;
      rightMouseUp = button == 1 && !action;
      if (rightMouseDown) { newParticle = Add(v2d::v2d(vec.x, vec.y), 1.0); }
      if (rightMouseUp)
      {
         newParticle->mass = newParticle->radius * newParticle->radius * newParticle->radius;
         newParticle->disabled = false;
      }
      if (leftMouseUp)
      {
         px = px + ((x - mx)) / sf;
         py = py + ((y - my)) / sf;
      }
      if (leftMouseDown)
      {
         mx = x;
         my = y;
      }
   }
   void onMouseMove(double x, double y) override
   {
      if (leftMouseDown)
      {
         translation =
             glm::translate(glm::mat4(1.0f), {px + ((x - mx)) / sf, py + ((y - my)) / sf, 0.0f});
      }
   }
   void onType(char32_t codepoint) override
   {
      if (codepoint == 'r') { Clear(); }
      if (codepoint == 'f') { ToggleFullscreen(); }
   }

private:
   bool leftMouseDown = false;
   bool leftMouseUp = false;
   bool rightMouseDown = false;
   bool rightMouseUp = false;
   double mx, my = 0.0;
   double px, py = 0.0;
   glm::mat4 translation = glm::mat4(1.0f);
   glm::mat4 scale = glm::mat4(1.0f);
   double sf = 1.0;
   shared_ptr<Particle> newParticle;
};

int main()
{
   auto a = Example();
   a.Create(1000, 2000, "Particulo Example: Interactive Solar System", 10000);
   a.Start(0.5ms);
}