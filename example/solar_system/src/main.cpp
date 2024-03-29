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
static float constexpr TIMESTEP = 0.0001;

struct Particle
{
   Particle(int index) : index(index) {}
   Particle(int index, int width, int height, float mass) : index(index), pos(0, width, 0, height), mass(mass), radius(mass / SizeRatio * 1.0f) {}

   const int index;
   v2d::v2d pos;
   v2d::v2d vel;
   v2d::v2d force;
   uint32_t color = 0x00b894ff;
   float mass;
   float radius;
};
class Example : public Particulo::Particulo<Particle, 8>
{
public:
   void init() override {
      SetBGColor(0x222f3eFF);
      AddParticles();
   }
   void simulate(const vector<shared_ptr<Particle>>& snapshot, const span<shared_ptr<Particle>> section, milliseconds timeElapsed) override {
      if (snapshot.size() == 0 || paused) return;
      for (auto& each : section)
      {
         for (auto& particle : snapshot)
         {
            float distance = sqrtf(each->pos.sqrDist(particle->pos));
            if (distance > 4.0)
            {
               float pairForce = ((particle->mass * each->mass) / (GCONSTANT * (distance * distance)));
               auto F = ((particle->pos - each->pos) / distance) * pairForce;
               each->force += F;
               // particle->force -= F;
            }
         }
      }
   }
   void update(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override {
      for (auto& each : particles)
      {
         each->vel += each->force / each->mass;
         each->pos += each->vel * TIMESTEP;
         each->force = {0, 0};
      }
      SetTransform(glm::mat4(1.0f) * scale * translation);
   }
   void onScroll(double x, double y) override {
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
   void onMouseClick(int button, int action, int mods) override {
      auto [x, y] = GetMousePos();
      leftMouseDown = button == 0 && action;
      leftMouseUp = button == 0 && !action;
      if (leftMouseUp)
      {
         px = px + ((x - lmx)) / sf;
         py = py + ((y - lmy)) / sf;
      }
      if (leftMouseDown)
      {
         lmx = x;
         lmy = y;
      }
   }
   void onMouseMove(double x, double y) override {
      if (leftMouseDown) { translation = glm::translate(glm::mat4(1.0f), {px + ((x - lmx)) / sf, py + ((y - lmy)) / sf, 0.0f}); }
   }
   void onType(char32_t codepoint) override {
      if (codepoint == 'r')
      {
         Clear();
         AddParticles();
      }
      if (codepoint == 'f') { ToggleFullscreen(); }
   }
   void onKey(int key, int scancode, int action, int mods) override {
      if (action)
      {
         if (key == GLFW_KEY_F11) { ToggleFullscreen(); }
         if (key == GLFW_KEY_SPACE) { paused = !paused; }
      }
   }

private:
   bool leftMouseDown = false;
   bool leftMouseUp = false;
   double lmx, lmy = 0.0;
   double px, py = 0.0;
   bool paused = false;
   glm::mat4 translation = glm::mat4(1.0f);
   glm::mat4 scale = glm::mat4(1.0f);
   double sf = 1.0;

private:
   void AddParticles() {
      std::uniform_real_distribution<float> massDistr(0, SizeRatio);
      for (int i = 0; i < 1000; i++) { Add(GetWidth(), GetHeight(), massDistr(gen)); }
   }
};

int main() {
   auto a = Example();
   a.Create(1000, 2000, "Particulo Example: Solar System", 10000);
   a.Start(8ms, 0ms);
}