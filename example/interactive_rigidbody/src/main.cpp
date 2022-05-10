#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>
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
   Particle(int index, int width, int height, float mass) : index(index), pos(0, width, 0, height), mass(mass), radius(cbrt(mass)) {}
   Particle(int index, v2d::v2d pos, float mass, bool disabled = true) : index(index), pos(pos), mass(mass), radius(cbrt(mass)), disabled(disabled) {}

   const int index;
   v2d::v2d pos;
   v2d::v2d vel;
   v2d::v2d force;
   uint32_t color = 0x00b894bb;
   float mass;
   float radius;
   bool disabled = false;
};
class Example : public Particulo::Particulo<Particle, 8>
{
protected:
   void init() override {
      SetBGColor(0x222f3eFF);
      // line = AddPolyLine({{0, 0}, {0, 100}, {100, 100}}, 0xFF00FFFF, 2.0);
      // auto line2 = AddPolyLine({{100, 100}, {100, 0}, {0, 0}}, 0x00FFFFFF, 3.0);
      // bezier = AddBezier(
      //     {
      //         {0, 0},
      //         {0, 100},
      //         {100, 0},
      //         {100, 100},
      //         {100, 100},
      //         {200, 0},
      //         {0, 200},
      //         {0, 100},
      //     },
      //     0xFFFFFFFF, 1.0);

      // line->GraphicsInit();
      // line2->GraphicsInit();
      // line3->GraphicsInit();
   }
   void simulate(const vector<shared_ptr<Particle>>& snapshot, const span<shared_ptr<Particle>> section, milliseconds timeElapsed) override {
      if (snapshot.size() == 0 || paused) return;
      for (auto& each : section)
      {
         if (each->disabled) { continue; }
         for (auto& particle : snapshot)
         {
            if (particle->disabled) { continue; }
            if (each->index == particle->index) { continue; }
            auto p1 = *each;
            auto p2 = *particle;
            p1.pos += p1.vel * TIMESTEP;
            p2.pos += p2.vel * TIMESTEP;
            float distance = std::sqrt(p1.pos.sqrDist(p2.pos));
            if (std::abs(distance) < p1.radius + p2.radius)
            {
               auto [p1vel, p2vel] = CollideElastic(p1, p2);
               each->vel = p1vel;
               particle->vel = p2vel;
            }
         }
      }
   }
   void update(const vector<shared_ptr<Particle>>& particles, milliseconds timeElapsed) override {
      if (!paused)
      {
         for (auto& each : particles)
         {
            if (each->disabled) { continue; }
            each->pos += each->vel * TIMESTEP;
         }
         // line->SetColor(timeElapsed.count() * 1000);
         // auto controlPoints = bezier->GetControlPoints();
         // for (auto& controlPoint : controlPoints) { controlPoint.x++; }
         // bezier->SetControlPoints(std::move(controlPoints));
      }
      if (newParticle && rightMouseDown) newParticle->radius += 1.0;
      SetTransform(scale * translation);
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
      auto [x, y] = GetMousePos(::Particulo::ScreenSpace);
      auto [wX, wY] = GetMousePos(::Particulo::WorldSpace);

      leftMouseDown = button == 0 && action;
      leftMouseUp = button == 0 && !action;
      rightMouseDown = button == 1 && action;
      rightMouseUp = button == 1 && !action;
      if (rightMouseDown) { newParticle = Add(v2d::v2d(wX, wY), 1.0); }
      if (rightMouseUp)
      {
         newParticle->mass = std::pow(newParticle->radius, 3.0f);
         newParticle->disabled = false;
         newParticle = nullptr;
      }
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
      if (rightMouseDown)
      {
         rmx = x;
         rmy = y;
      }
   }
   void onMouseMove(double x, double y) override {
      if (leftMouseDown) { translation = glm::translate(glm::mat4(1.0f), {px + ((x - lmx)) / sf, py + ((y - lmy)) / sf, 0.0f}); }
      if (rightMouseDown)
      {
         newParticle->vel.set((x - rmx) / sf, (y - rmy) / sf);
         newParticle->vel /= -50.0f;
      }
   }
   void onType(char32_t codepoint) override {
      if (codepoint == 'r') { Clear(); }
      if (codepoint == 'f') { ToggleFullscreen(); }
      if (codepoint == 'm') { AddParticles(); }
   }
   void onKey(int key, int scancode, int action, int mods) override {
      if (action)
      {
         if (key == GLFW_KEY_F11) { ToggleFullscreen(); }
         if (key == GLFW_KEY_SPACE) { paused = !paused; }
      }
   }
   void AddParticles() {
      std::uniform_real_distribution<float> massDistr(0, 10);
      vector<shared_ptr<Particle>> newParticles = DangerouslyGet();
      for (int i = 0; i < 1000; i++) { newParticles.push_back(NewParticle(GetWidth(), GetHeight(), massDistr(gen))); }
      DangerouslySet(std::move(newParticles));
   }

private:
   bool leftMouseDown = false;
   bool leftMouseUp = false;
   bool rightMouseDown = false;
   bool rightMouseUp = false;
   bool paused = false;
   double lmx, lmy, rmx, rmy = 0.0;
   double px, py = 0.0;
   glm::mat4 translation = glm::mat4(1.0f);
   glm::mat4 scale = glm::mat4(1.0f);
   double sf = 1.0;
   shared_ptr<Particle> newParticle;
   shared_ptr<::Particulo::PolyLine> line;
   // shared_ptr<::Particulo::Bezier> bezier;
};

int main() {
   auto a = Example();
   a.Create<10000>(1000, 1000, "Particulo Example: Rigidbody");
   a.Start(8ms, 8ms);
}