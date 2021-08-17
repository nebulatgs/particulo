#define GLFW_INCLUDE_NONE

#include <chrono>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using time_point = std::chrono::system_clock::time_point;

#include <thread>
using std::this_thread::sleep_for;

#include <functional>
using std::function;

#include <iostream>
using std::cout;
using std::endl;

#include <concepts>
using std::same_as;

#include <string>
using std::string;

#include <type_traits>
using std::is_arithmetic_v;

#include <vector>
using std::vector;

#include <GLFW/glfw3.h>
#include <glad/glad.h>
namespace Particulo
{

template <typename T>
concept Particle = ((is_arithmetic_v<typeof T::x> && same_as<typeof T::x, typeof T::y>) );

template <typename T>
concept ColorfulParticle = (Particle<T> && same_as<uint32_t, typeof T::color>);

template <typename T>
concept PlainParticle = (Particle<T> && !ColorfulParticle<T>);

template <Particle T>
class Particulo
{
protected:
   string _title;

private:
   vector<T> particles;
   int _width;
   int _height;
   GLFWwindow* _window;
   bool isReady;
   milliseconds timeElapsed;
   time_point initialTime;

public:
   virtual void init() {}
   virtual void simulate(vector<T>& particles, milliseconds timeElapsed) = 0;
   virtual ~Particulo(){};

public:
   void Create(int width, int height, int particleCount, string title = "Particulo")
   {
      initialTime = high_resolution_clock::now();
      _title = title;
      for (int i = 0; i < particleCount; i++) { particles.emplace_back(); }
      // gfxInit(width, height);
      init();
   }

   void tick()
   {
      simulate(particles, timeElapsed);
      draw();
   }

   template <typename _Rep, typename _Period>
   void Start(duration<_Rep, _Period> sleepInterval, function<bool()> haltingCondition)
   {
      while (!haltingCondition()) { loop(sleepInterval); }
   }

   template <typename _Rep, typename _Period>
   void Start(duration<_Rep, _Period> sleepInterval, function<bool(milliseconds)> haltingCondition)
   {
      while (!haltingCondition(timeElapsed)) { loop(sleepInterval); }
   }

   template <typename _Rep, typename _Period>
   void Start(duration<_Rep, _Period> sleepInterval)
   {
      while (true) { loop(sleepInterval); }
   }

protected:
   void draw() requires(ColorfulParticle<T>)
   {
      cout << "ColorfulParticle Draw called" << endl;
      for (auto& particle : particles) { cout << particle.x << endl; }
   }

   void draw() requires(PlainParticle<T>)
   {
      cout << "PlainParticle Draw called" << endl;
      for (auto& particle : particles) { cout << particle.x << std::endl; }
   }

private:
   void gfxInit(int width, int height)
   {
      _width = width;
      _height = height;
      // Try to initialize GLFW
      if (!glfwInit()) exit(EXIT_FAILURE);

      // Try to create a window
      glfwWindowHint(GLFW_SAMPLES, 4);
      glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
      glfwWindowHint(GL_MAJOR_VERSION, 3);
      glfwWindowHint(GL_MINOR_VERSION, 1);
      _window = glfwCreateWindow(_width, _height, "Particulo", NULL, NULL);
      if (!_window) exit(EXIT_FAILURE);

      // Load GL
      glfwMakeContextCurrent(_window);
      gladLoadGL();

      // Set viewport
      glfwGetFramebufferSize(_window, &this->_width, &this->_height);
      glViewport(0, 0, this->_width, this->_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, this->_width, this->_height, 0, 1, -1);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
      glfwSwapInterval(1);
      isReady = true;
   }

private:
   template <typename _Rep, typename _Period>
   void loop(duration<_Rep, _Period> sleepInterval)
   {
      tick();
      sleep_for(sleepInterval);
      timeElapsed = duration_cast<milliseconds>(high_resolution_clock::now() - initialTime);
   }
};
} // namespace Particulo