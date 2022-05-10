#pragma once

#define GLFW_INCLUDE_NONE
#include <tuple>

#include <chrono>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using time_point = std::chrono::high_resolution_clock::time_point;

#include <memory>
using std::make_shared;
using std::shared_ptr;

#include <thread>
using std::this_thread::sleep_for;

#include <functional>
using std::function;

#include <iostream>
using std::cout;
using std::endl;

#include <concepts>
using std::constructible_from;
using std::same_as;

#include <string>
using std::string;

#include <type_traits>
using std::is_arithmetic_v;

#include <vector>
using std::vector;

#include <span>
using std::span;

#include <thread>
using std::thread;

#include <mutex>
using std::mutex;

#include <shared_mutex>
using std::shared_lock;
using std::shared_mutex;
using std::unique_lock;

#include "shader.hpp"
#include "v2d.hpp"
#include <GLFW/glfw3.h>
#include <Polyline2D.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_inverse.hpp>
namespace Particulo
{
enum CoordSpace
{
   ScreenSpace,
   WorldSpace
};
// Shaders

static inline const string LineVertexShader = R"(
   #version 450 core
   precision highp float;
   layout (location = 0) in highp vec2 pPos;
   layout (location = 1) in highp vec4 pCol;
   uniform mat4 transform;
   out vec4 col;
   void main() {
      gl_Position = transform * vec4(pPos, 0.0, 1.0);
      col = pCol;
   }
)";

static inline const string LineFragmentShader = R"(
   #version 450 core
   out vec4 FragColor;
   in vec4 col;
   void main() {
      FragColor = col;
   }
)";

static inline const string ParticleVertexShader = R"(
   #version 450 core
   precision highp float;
   layout (location = 0) in highp vec3 aPos;
   layout (location = 1) in highp vec4 pPos;
   layout (location = 2) in highp vec4 pCol;
   uniform mat4 transform;
   out vec4 coord;
   out vec4 col;
   void main()
   {
      vec4 pPost = (pPos + (vec4(aPos, 1.0) * pPos.w));
      gl_Position = transform * vec4(pPost.x, pPost.y, 1.0, 1.0);
      coord = vec4(aPos, 1.0);
      col = pCol;
   }
)";

static inline const string ParticleFragmentShader = R"(
   #version 450 core
   precision highp float;
   out vec4 FragColor;
   in vec4 coord;
   in vec4 col;

   float aastep(float threshold, float value) {
   float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
   return smoothstep(threshold-afwidth, threshold+afwidth, value);
   }

   void main()
   {
      if(col.a == 0.0)
      {
         discard;
      }
      FragColor.rgb = col.rgb;
      
      FragColor.a = aastep(-1.0, -1.0 * length(coord.xyz));
      FragColor.a *= col.a;
      
      if(FragColor.a == 0.0)
      {
         discard;
      }
   }
)";
// Concepts
template <typename T>
concept is_arithmetic = is_arithmetic_v<T>;

#ifdef clang
template <typename T>
concept BasicParticleXY = requires(T a) {
   { a.index } -> is_arithmetic;
   { a.x } -> is_arithmetic;
   { a.x } -> same_as<typeof a.y>;
   { a.radius } -> same_as<float>;
};

template <typename T>
concept BasicParticleV = requires(T a) {
   { a.pos } -> same_as<v2d::v2d>;
   { a.radius } -> same_as<float>;
};

template <typename T>
concept Particle = (BasicParticleXY<T> || BasicParticleV<T>) &&(constructible_from<int> || constructible_from<long>);

template <typename T>
concept ColorfulParticle = requires(T a) {
   { a } -> Particle;
   { a.color } -> same_as<uint32_t>;
};
#else
template <typename T>
concept BasicParticleXY = (is_arithmetic_v<typeof T::index> && is_arithmetic_v<typeof T::x> && same_as<typeof T::x, typeof T::y> &&
                           same_as<typeof T::radius, float>);

template <typename T>
concept BasicParticleV = (same_as<typeof T::pos, v2d::v2d> && same_as<typeof T::radius, float>);

template <typename T>
concept Particle = (BasicParticleXY<T> || BasicParticleV<T>) &&(constructible_from<T, int> || constructible_from<T, long>);

template <typename T>
concept ColorfulParticle = (Particle<T> && same_as<uint32_t, typeof T::color>);

template <typename T>
concept PlainParticle = (Particle<T> && !ColorfulParticle<T>);
#endif
// Structs
inline std::tuple<float, float, float, float> uint32ToFloatColor(uint32_t color) {
   union
   {
      uint32_t hex;
      struct
      {
         uint8_t a;
         uint8_t b;
         uint8_t g;
         uint8_t r;
      };
   } temp;
   temp.hex = color;
   return {
       temp.r / 255.0f,
       temp.g / 255.0f,
       temp.b / 255.0f,
       temp.a / 255.0f,
   };
}
struct RGBA
{
   float r;
   float g;
   float b;
   float a;
};

class GraphicsPrimitive
{
public:
   virtual void Draw() = 0;
   virtual void UpdateBuffers() = 0;
};

class PolyLine : public GraphicsPrimitive
{
public:
   PolyLine() = default;
   PolyLine(int index, vector<crushedpixel::Vec2> points, double thickness, uint32_t color)
       : index(index), points(std::move(points)), thickness(thickness) {
      auto [r, g, b, a] = uint32ToFloatColor(color);
      this->color = {r, g, b, a};
      vertices = crushedpixel::Polyline2D::create(this->points, thickness, crushedpixel::Polyline2D::JointStyle::ROUND,
                                                  crushedpixel::Polyline2D::EndCapStyle::ROUND);
      for (auto v : vertices)
      {
         glVerts.push_back(v.x);
         glVerts.push_back(v.y);

         glCols.push_back(this->color.r);
         glCols.push_back(this->color.g);
         glCols.push_back(this->color.b);
         glCols.push_back(this->color.a);
      }
      Init();
   };

private:
   void Update() {
      vertices = crushedpixel::Polyline2D::create(this->points, thickness, crushedpixel::Polyline2D::JointStyle::ROUND,
                                                  crushedpixel::Polyline2D::EndCapStyle::ROUND);
      glVerts.clear();
      glCols.clear();
      for (auto v : vertices)
      {
         glVerts.push_back(v.x);
         glVerts.push_back(v.y);

         glCols.push_back(this->color.r);
         glCols.push_back(this->color.g);
         glCols.push_back(this->color.b);
         glCols.push_back(this->color.a);
      }
   }
   void Init() {
      glGenBuffers(1, &buffer);
      glGenBuffers(1, &colorBuffer);
      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);
      UpdateBuffers();
   }

public:
   void UpdateBuffers() override {
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, glVerts.size() * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, glVerts.size() * sizeof(GLfloat), glVerts.data());

      glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
      glBufferData(GL_ARRAY_BUFFER, glCols.size() * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, glCols.size() * sizeof(GLfloat), glCols.data());
   }
   void Draw() override {
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
                               // match the layout in the shader.
                            2,        // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void*) 0 // array buffer offset
      );
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
      glVertexAttribPointer(1, // attribute. No particular reason for 0, but must
                               // match the layout in the shader.
                            4,        // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void*) 0 // array buffer offset
      );
      glDrawArrays(GL_TRIANGLES, 0, glVerts.size() / 2);
   }

public:
   void SetColor(uint32_t color) {
      auto [r, g, b, a] = uint32ToFloatColor(color);
      this->color = {r, g, b, a};
      Update();
   }
   void SetThickness(double thickness) {
      this->thickness = thickness;
      Update();
   }
   void SetPoints(vector<crushedpixel::Vec2> points) {
      this->points = points;
      Update();
   }
   void SetPoints(vector<crushedpixel::Vec2>&& points) {
      this->points = points;
      Update();
   }

public:
   const RGBA& GetColor() const { return color; }
   const int GetIndex() const { return index; }
   const double GetThickness() const { return thickness; }
   const vector<crushedpixel::Vec2>& GetPoints() const { return points; }

private:
   int index;
   double thickness;
   vector<crushedpixel::Vec2> points;
   vector<crushedpixel::Vec2> vertices;
   vector<GLfloat> glVerts;
   vector<GLfloat> glCols;
   RGBA color;
   GLuint buffer;
   GLuint colorBuffer;
   GLuint VAO;
};
class Bezier : public GraphicsPrimitive
{
public:
   Bezier(int index, vector<v2d::v2d> controlPoints, double thickness, uint32_t color)
       : index(index), controlPoints(std::move(controlPoints)), color(color), thickness(thickness) {
      Update();
   }

public:
   const vector<v2d::v2d>& GetControlPoints() const { return controlPoints; }
   const uint32_t GetColor() const { return color; }
   const double GetThickness() const { return thickness; }

public:
   void SetControlPoints(vector<v2d::v2d> controlPoints) {
      this->controlPoints = std::move(controlPoints);
      Update();
   }
   void SetColor(uint32_t color) {
      this->color = color;
      UpdateColor();
   }
   void SetThickness(double thickness) {
      this->thickness = thickness;
      Update();
   }

public:
   void Draw() override { polyLine.Draw(); }
   void UpdateBuffers() override { polyLine.UpdateBuffers(); }

private:
   void Update() {
      vector<crushedpixel::Vec2> points;
      for (int j = 0; j < this->controlPoints.size(); j += 4)
      {
         float xu = 0.0, yu = 0.0, u = 0.0;
         auto& c1 = this->controlPoints[j + 0];
         auto& c2 = this->controlPoints[j + 1];
         auto& c3 = this->controlPoints[j + 2];
         auto& c4 = this->controlPoints[j + 3];
         for (u = 0.0; u <= 1.0; u += 0.0001)
         {
            xu = pow(1 - u, 3) * c1.x + 3 * u * pow(1 - u, 2) * c2.x + 3 * pow(u, 2) * (1 - u) * c3.x + pow(u, 3) * c4.x;
            yu = pow(1 - u, 3) * c1.y + 3 * u * pow(1 - u, 2) * c2.y + 3 * pow(u, 2) * (1 - u) * c3.y + pow(u, 3) * c4.y;
            points.push_back({xu, yu});
         }
      }
      polyLine = PolyLine(index, std::move(points), thickness, color);
   }
   void UpdateColor() { polyLine.SetColor(color); }

private:
   int index;
   uint32_t color;
   double thickness;
   vector<v2d::v2d> controlPoints;
   PolyLine polyLine;
};
// Main
template <ColorfulParticle T, int threadCount = 1>
class Particulo
{
private:
   static void s_ScrollCallback(GLFWwindow* window, double x, double y) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->onScroll(x, y);
   }
   static void s_MouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->onMouseClick(button, action, mods);
   }
   static void s_MouseMoveCallback(GLFWwindow* window, double x, double y) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->onMouseMove(x, y);
   }
   static void s_KeyboardTypeCallback(GLFWwindow* window, unsigned int codepoint) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->onType(codepoint);
   }
   static void s_KeypressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->onKey(key, scancode, action, mods);
   }
   static void s_WindowClose(GLFWwindow* window) {
      auto instance = reinterpret_cast<Particulo*>(glfwGetWindowUserPointer(window));
      instance->isClosing = true;
      instance->onClose();
      glfwDestroyWindow(window);
   }

protected:
   virtual void onScroll(double x, double y) {}
   virtual void onMouseClick(int button, int action, int mods) {}
   virtual void onMouseMove(double x, double y) {}
   virtual void onType(char32_t codepoint) {}
   virtual void onKey(int key, int scancode, int action, int mods) {}
   // Called if the user closes the graphics window but before the program exits
   virtual void onClose() {}

protected:
   const string& GetTitle() const { return p_title; }
   const glm::mat4& GetTransform() const { return p_transform; }
   const float GetWidth() const { return p_width; }
   const float GetHeight() const { return p_height; }
   const float GetMaxCount() const { return p_maxCount; }
   const time_point& GetInitialTime() const { return p_initialTime; }
   const std::tuple<double, double> GetMousePos(CoordSpace coordSpace = ScreenSpace) const {
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      if (coordSpace == ScreenSpace) { return {x, y}; }
      else if (coordSpace == WorldSpace)
      {
         auto vec = glm::dvec4(x, y, 1.0, 1.0);
         vec = glm::affineInverse(p_transform) * vec;
         return {vec.x, vec.y};
      }
      else
      { throw std::logic_error("Not implemented: GetMousePos(" + std::to_string(coordSpace) + ")"); }
   }
   const bool GetFullscreenState() const { return p_fullscreen; }

protected:
   void SetTitle(string title) {
      p_title = title;
      glfwSetWindowTitle(window, title.c_str());
   }
   void SetTransform(glm::mat4 transform) { p_transform = transform; }
   template <typename... _Args>
   shared_ptr<T> Add(_Args&&... __args) {
      unique_lock lock(mtx);
      if (particles.size() == p_maxCount)
      {
         throw std::logic_error("Attempted to exceed the max particle count in instance method "
                                "Add(_Args&&... __args)");
      }
      auto particle = make_shared<T>(++maxParticleIndex, __args...);
      particles.push_back(particle);
      snapshot = particles;
      return particle;
   }
   shared_ptr<PolyLine> AddPolyLine(vector<crushedpixel::Vec2>&& points, uint32_t color, double thickness) {
      unique_lock lock(mtx);
      auto line = make_shared<PolyLine>(++maxParticleIndex, std::move(points), thickness, color);
      primitives.push_back(line);
      return line;
   }

   // static vector<crushedpixel::Vec2> BezierToPoints(vector<v2d::v2d> points, double t = 0.1) {
   //    vector<crushedpixel::Vec2> result;
   //    for (double u = 0.0; u <= 1.0; u += t)
   //    {
   //       int i = points.size() - 1;
   //       while (i > 0)
   //       {
   //          for (int k = 0; k < i; k++)
   //          {
   //             points[k] = (points[k + 1] - points[k]) * points[k] + t;
   //             cout << points[k].x << ", " << points[k].y << endl;
   //          }
   //          i--;
   //       }
   //       result.push_back({points[0].x, points[0].y});
   //    }
   //    return result;
   // }

   shared_ptr<Bezier> AddBezier(vector<v2d::v2d> controlPoints, uint32_t color, double thickness) {
      unique_lock lock(mtx);
      auto bezier = make_shared<Bezier>(++maxParticleIndex, std::move(controlPoints), thickness, color);
      primitives.push_back(bezier);
      return bezier;
   }
   template <typename... _Args>
   shared_ptr<T> NewParticle(_Args&&... __args) {
      return make_shared<T>(++maxParticleIndex, __args...);
   }
   void Swap(vector<shared_ptr<T>>&& newParticles) {
      if (newParticles.size() > p_maxCount)
      {
         throw std::logic_error("Attempted to exceed the max particle count in instance method "
                                "Add(_Args&&... __args)");
      }
      particles = newParticles;
      snapshot = particles;
   }
   void DangerouslySet(vector<shared_ptr<T>>&& newParticles) {
      if (newParticles.size() > p_maxCount)
      {
         throw std::logic_error("Attempted to exceed the max particle count in instance method "
                                "Add(_Args&&... __args)");
      }
      particles = newParticles;
      snapshot = particles;
      mtx.unlock();
   }
   vector<shared_ptr<T>>&& DangerouslyGet() {
      mtx.lock();
      return std::move(particles);
   }
   void Remove() {
      unique_lock lock(mtx);
      particles.pop_back();
   }
   void Remove(int i) {
      unique_lock lock(mtx);
      particles.erase(particles.front() + i);
   }
   void Clear() {
      unique_lock lock(mtx);
      cout << "Removing " << particles.size() << " particles" << endl;
      particles.clear();
      std::fill(particle_position_size_data.begin(), particle_position_size_data.end(), 0);
      std::fill(particle_color_data.begin(), particle_color_data.end(), 0);
   }

private:
   string p_title;
   glm::mat4 p_transform = glm::mat4(1.0f);
   int p_width;
   int p_height;
   int p_maxCount;
   time_point p_initialTime;
   bool p_fullscreen = false;

private:
   GLuint VAO;
   Shader particleShader;
   Shader lineShader;
   GLuint particles_position_buffer;
   GLuint particles_color_buffer;
   GLuint billboard_vertex_buffer;
   vector<GLfloat> particle_position_size_data;
   vector<GLfloat> particle_color_data;

private:
   vector<shared_ptr<GraphicsPrimitive>> primitives;

private:
   vector<shared_ptr<T>> particles;
   vector<shared_ptr<T>> snapshot;
   GLFWwindow* window;
   bool isReady;
   milliseconds timeElapsed;
   RGBA bgColor = {0.0f, 0.0f, 0.0f, 1.0f};
   mutable int maxParticleIndex;
   glm::mat4 screenCorrectionTransform = glm::mat4(1.0f);
   glm::mat4 identity = glm::mat4(1.0f);
   glm::mat4 tempMatrix = glm::mat4(1.0f);
   bool isClosing = false;
   int wPosX, wPosY;
   int wSizeX, wSizeY;

private:
   vector<thread> simThreads;
   thread drawThread;
   mutable shared_mutex mtx;
   bool swapInterval = false;

public:
   virtual void init() {}
   // virtual void simulate(const vector<shared_ptr<T>>& particles, milliseconds
   // timeElapsed, int thread) = 0;
   virtual void simulate(const vector<shared_ptr<T>>& snapshot, const span<shared_ptr<T>> section, milliseconds timeElapsed) = 0;
   virtual void update(const vector<shared_ptr<T>>& particles, milliseconds timeElapsed){};
   virtual ~Particulo(){};

protected:
   void SetBGColor(float r, float g, float b, float a = 1.0f) { bgColor = {r, g, b, a}; }
   void SetBGColor(RGBA color) { bgColor = color; }
   void SetBGColor(uint32_t color) {
      auto [r, g, b, a] = uint32ToFloatColor(color);
      bgColor = {
          r,
          g,
          b,
          a,
      };
   }
   void DisableCursor() {
      if (!isReady) { throw std::logic_error("Cannot disable cursor before initialization"); }
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   }
   void EnableCursor() {
      if (!isReady) { throw std::logic_error("Cannot enable cursor before initialization"); }
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
   }
   static std::tuple<v2d::v2d, v2d::v2d> CollideElastic(shared_ptr<T> particle1, shared_ptr<T> particle2) {
      auto v_n = particle2->pos - particle1->pos;
      auto v_un = v_n.norm();
      auto v_ut = v2d::v2d(-v_un.y, v_un.x);

      double v1n = v_un.dot(particle1->vel);
      double v1t = v_ut.dot(particle1->vel);
      double v2n = v_un.dot(particle2->vel);
      double v2t = v_ut.dot(particle2->vel);

      double v1tPrime = v1t;
      double v2tPrime = v2t;

      double v1nPrime = (v1n * (particle1->mass - particle2->mass) + 2.0 * particle2->mass * v2n) / (particle1->mass + particle2->mass);
      double v2nPrime = (v2n * (particle2->mass - particle1->mass) + 2.0 * particle1->mass * v1n) / (particle1->mass + particle2->mass);

      auto v_v1nPrime = v_un * v1nPrime;
      auto v_v1tPrime = v_ut * v1tPrime;
      auto v_v2nPrime = v_un * v2nPrime;
      auto v_v2tPrime = v_ut * v2tPrime;

      return {{v_v1nPrime.x + v_v1tPrime.x, v_v1nPrime.y + v_v1tPrime.y}, {v_v2nPrime.x + v_v2tPrime.x, v_v2nPrime.y + v_v2tPrime.y}};
   }
   static std::tuple<v2d::v2d, v2d::v2d> CollideElastic(T& particle1, T& particle2) {
      auto v_n = particle2.pos - particle1.pos;
      auto v_un = v_n;
      v_un.norm();
      auto v_ut = v2d::v2d(-v_un.y, v_un.x);

      double v1n = v_un.dot(particle1.vel);
      double v1t = v_ut.dot(particle1.vel);
      double v2n = v_un.dot(particle2.vel);
      double v2t = v_ut.dot(particle2.vel);

      double v1tPrime = v1t;
      double v2tPrime = v2t;

      double v1nPrime = (v1n * (particle1.mass - particle2.mass) + 2.0 * particle2.mass * v2n) / (particle1.mass + particle2.mass);
      double v2nPrime = (v2n * (particle2.mass - particle1.mass) + 2.0 * particle1.mass * v1n) / (particle1.mass + particle2.mass);

      auto v_v1nPrime = v_un * v1nPrime;
      auto v_v1tPrime = v_ut * v1tPrime;
      auto v_v2nPrime = v_un * v2nPrime;
      auto v_v2tPrime = v_ut * v2tPrime;

      return {{v_v1nPrime.x + v_v1tPrime.x, v_v1nPrime.y + v_v1tPrime.y}, {v_v2nPrime.x + v_v2tPrime.x, v_v2nPrime.y + v_v2tPrime.y}};
   }

   static v2d::v2d CollideInelastic(T& particle1, T& particle2) {
      return ((particle1.vel * particle1.mass) + (particle2.vel * particle2.mass)) / (particle1.mass + particle2.mass);
   }
   static v2d::v2d CollideInelastic(shared_ptr<T> particle1, shared_ptr<T> particle2) {
      return ((particle1->vel * particle1->mass) + (particle2->vel * particle2->mass)) / (particle1->mass + particle2->mass);
   }

   void ToggleFullscreen() {
      if (!p_fullscreen)
      {
         glfwGetWindowPos(window, &wPosX, &wPosY);
         glfwGetWindowSize(window, &wSizeX, &wSizeY);
         auto* monitor = glfwGetPrimaryMonitor();
         auto* vidMode = glfwGetVideoMode(monitor);
         glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, GLFW_DONT_CARE);
         swapInterval = true;
      }
      else
      {
         glfwSetWindowMonitor(window, nullptr, wPosX, wPosY, wSizeX, wSizeY, GLFW_DONT_CARE);
         swapInterval = true;
      }
      p_fullscreen = !p_fullscreen;
   }
   void tick() { commonDraw(); }

public:
   template <int maxCount = 1 << 14, int initialCount = 0>
   void Create(int width, int height, string title = "Particulo") {
      static_assert(initialCount < maxCount, "Attempted to exceed the max particle count during creation");
      p_initialTime = high_resolution_clock::now();
      p_maxCount = maxCount;
      p_title = title;
      for (int i = 0; i < initialCount; i++) { particles.push_back(make_shared<T>(i)); }
      maxParticleIndex = initialCount - 1;
      gfxInit(width, height);
      bufferInit();
      init();
      snapshot = particles;
      glfwMakeContextCurrent(NULL);
      isReady = true;
   }
   template <typename _DrawRep, typename _DrawPeriod, typename _SimRep, typename _SimPeriod>
   void Start(duration<_DrawRep, _DrawPeriod> drawSleepInterval, duration<_SimRep, _SimPeriod> simSleepInterval, function<bool()> haltingCondition) {
      startThreads(simSleepInterval, haltingCondition);
      startDrawThread(drawSleepInterval, haltingCondition);
      while (!haltingCondition() && !isClosing) { mainThreadLoop(drawSleepInterval); }
   }

   template <typename _DrawRep, typename _DrawPeriod, typename _SimRep, typename _SimPeriod>
   void Start(duration<_DrawRep, _DrawPeriod> drawSleepInterval, duration<_SimRep, _SimPeriod> simSleepInterval,
              function<bool(milliseconds)> haltingCondition) {
      startThreads(simSleepInterval, haltingCondition);
      startDrawThread(drawSleepInterval, haltingCondition);
      while (!haltingCondition(timeElapsed) && !isClosing) { mainThreadLoop(drawSleepInterval); }
   }

   template <typename _DrawRep, typename _DrawPeriod, typename _SimRep, typename _SimPeriod>
   void Start(duration<_DrawRep, _DrawPeriod> drawSleepInterval, duration<_SimRep, _SimPeriod> simSleepInterval) {
      startThreads(simSleepInterval);
      startDrawThread(drawSleepInterval);
      while (!isClosing) { mainThreadLoop(drawSleepInterval); }
   }

private:
   template <typename _Rep, typename _Period>
   void simLoop(int thread, duration<_Rep, _Period> sleepInterval, function<bool()> haltingCondition) {
      while (!haltingCondition() && !isClosing)
      {
         {
            shared_lock lock(mtx);
            if (particles.size() != 0)
            {
               const int step = particles.size() / threadCount;
               const int start_index = thread * step;
               const int end_index = (thread < threadCount - 1) ? start_index + step : particles.size() - 1;
               auto section = span{particles.begin() + start_index, particles.begin() + end_index};
               simulate(snapshot, section, timeElapsed);
            }
         }
         if (thread == 0)
         {
            unique_lock lock(mtx);
            update(particles, timeElapsed);
            snapshot = particles;
         }
         sleep_for(sleepInterval);
      }
   }

   template <typename _Rep, typename _Period>
   void simLoop(int thread, duration<_Rep, _Period> sleepInterval, function<bool(milliseconds)> haltingCondition) {
      while (!haltingCondition(timeElapsed) && !isClosing)
      {
         {
            shared_lock lock(mtx);
            if (particles.size() != 0)
            {
               const int step = particles.size() / threadCount;
               const int start_index = thread * step;
               const int end_index = (thread < threadCount - 1) ? start_index + step : particles.size() - 1;
               auto section = span{particles.begin() + start_index, particles.begin() + end_index};
               simulate(snapshot, section, timeElapsed);
            }
         }
         if (thread == 0)
         {
            unique_lock lock(mtx);
            update(particles, timeElapsed);
            snapshot = particles;
         }
         sleep_for(sleepInterval);
      }
   }

   template <typename _Rep, typename _Period>
   void simLoop(int thread, duration<_Rep, _Period> sleepInterval) {
      while (!isClosing)
      {
         {
            shared_lock lock(mtx);
            if (particles.size() != 0)
            {
               const int step = particles.size() / threadCount;
               const int start_index = thread * step;
               const int end_index = (thread < threadCount - 1) ? start_index + step : particles.size() - 1;
               auto section = span{particles.begin() + start_index, particles.begin() + end_index};
               simulate(snapshot, section, timeElapsed);
            }
         }
         if (thread == 0)
         {
            unique_lock lock(mtx);
            update(particles, timeElapsed);
            snapshot = particles;
         }
         sleep_for(sleepInterval);
      }
   }

   void commonDraw() {
      glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, p_width, p_height);
      glOrtho(0, p_width, p_height, 0, 1, -1);
      screenCorrectionTransform = glm::scale(identity, {2.0f / static_cast<float>(p_width), -2.0f / static_cast<float>(p_height), 1.0f});
      tempMatrix = screenCorrectionTransform * p_transform;
      tempMatrix = glm::translate(tempMatrix, {p_width / -2.0f, p_height / -2.0f, 0.0f});
      particleShader.SetMatrix4("transform", tempMatrix, true);
      setParticlePos();
      updateBuffers();
      draw();
      lineShader.SetMatrix4("transform", tempMatrix, true);
      drawLines();
      glfwSwapBuffers(window);
   }

   void setParticlePos() requires(BasicParticleXY<T>) {
      int i = 0;
      for (auto& particle : particles)
      {
         particle_position_size_data[i] = particle->x;
         particle_position_size_data[i + 1] = particle->y;
         particle_position_size_data[i + 2] = 0;
         particle_position_size_data[i + 3] = particle->radius;

         auto [r, g, b, a] = uint32ToFloatColor(particle->color);
         particle_color_data[i] = r;
         particle_color_data[i + 1] = g;
         particle_color_data[i + 2] = b;
         particle_color_data[i + 3] = a;

         i += 4;
      }
   }

   void setParticlePos() requires(BasicParticleV<T>) {
      int i = 0;
      for (auto& particle : particles)
      {
         particle_position_size_data[i] = particle->pos.x;
         particle_position_size_data[i + 1] = particle->pos.y;
         particle_position_size_data[i + 2] = 0;
         particle_position_size_data[i + 3] = particle->radius;

         auto [r, g, b, a] = uint32ToFloatColor(particle->color);
         particle_color_data[i] = r;
         particle_color_data[i + 1] = g;
         particle_color_data[i + 2] = b;
         particle_color_data[i + 3] = a;

         i += 4;
      }
   }

   void draw() requires(ColorfulParticle<T>) {
      // 1st attribute buffer : vertices
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
      glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
                               // match the layout in the shader.
                            3,        // size
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void*) 0 // array buffer offset
      );

      // 2nd attribute buffer : positions of particles' centers
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
      glVertexAttribPointer(1, // attribute. No particular reason for 1, but must
                               // match the layout in the shader.
                            4,        // size : x + y + z + size => 4
                            GL_FLOAT, // type
                            GL_FALSE, // normalized?
                            0,        // stride
                            (void*) 0 // array buffer offset
      );

      // 3rd attribute buffer : particles' colors
      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      glVertexAttribPointer(2, // attribute. No particular reason for 2, but must match the layout
                               // in the shader.
                            4,        // size : r + g + b + a => 4
                            GL_FLOAT, // type
                            GL_TRUE,  // normalized? *** YES, this means that the unsigned char[4]
                                      // will be accessible with a vec4 (floats) in the shader ***
                            0,        // stride
                            (void*) 0 // array buffer offset
      );

      // cout << glVertexAttribDivisor << endl;
      glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
      glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
      glVertexAttribDivisor(2, 1); // color : one per quad -> 1

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, p_maxCount);
   }

   void drawLines() {
      for (auto& primitive : primitives) { primitive->Draw(); }
   }

   void gfxInit(int width, int height) {
      p_width = width;
      p_height = height;
      // Try to initialize GLFW
      if (!glfwInit()) throw std::runtime_error("Unable to initialize GLFW");

      // Try to create a window
      glfwWindowHint(GLFW_SAMPLES, 4);
      glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      window = glfwCreateWindow(p_width, p_height, p_title.c_str(), NULL, NULL);
      if (!window) throw std::runtime_error("Unable to create GLFW Window");

      // Load GL
      glfwMakeContextCurrent(window);
      gladLoadGL();

      // Set viewport
      printf("%s\n", glGetString(GL_VERSION));
      glfwGetFramebufferSize(window, &p_width, &p_height);
      glViewport(0, 0, p_width, p_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, p_width, p_height, 0, 1, -1);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glfwSetWindowUserPointer(window, this);
      glfwSetScrollCallback(window, s_ScrollCallback);
      glfwSetMouseButtonCallback(window, s_MouseClickCallback);
      glfwSetWindowCloseCallback(window, s_WindowClose);
      glfwSetCursorPosCallback(window, s_MouseMoveCallback);
      glfwSetCharCallback(window, s_KeyboardTypeCallback);
      glfwSetKeyCallback(window, s_KeypressCallback);
      glfwSwapInterval(1);
   }

   void updateBuffers() {
      glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL,
                   GL_STREAM_DRAW); // Buffer orphaning, a common way to improve
                                    // streaming perf. See above link for details.
      glBufferSubData(GL_ARRAY_BUFFER, 0, p_maxCount * sizeof(GLfloat) * 4, particle_position_size_data.data());

      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL,
                   GL_STREAM_DRAW); // Buffer orphaning, a common way to improve
                                    // streaming perf. See above link for details.
      glBufferSubData(GL_ARRAY_BUFFER, 0, p_maxCount * sizeof(GLfloat) * 4, particle_color_data.data());

      for (auto& primitive : primitives) { primitive->UpdateBuffers(); }
   }

   void bufferInit() requires(ColorfulParticle<T>) {
      particle_position_size_data.resize(p_maxCount * 4);
      particle_color_data.resize(p_maxCount * 4);
      particles.reserve(p_maxCount);
      particleShader.CompileStrings(ParticleVertexShader, ParticleFragmentShader);
      lineShader.CompileStrings(LineVertexShader, LineFragmentShader);
      static const GLfloat vertices[] = {
          -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      };
      GLuint VBO;
      glGenBuffers(1, &VBO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
      glEnableVertexAttribArray(0);

      glGenVertexArrays(1, &VAO);
      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
      glEnableVertexAttribArray(0);
      glBindVertexArray(VAO);

      glGenBuffers(1, &billboard_vertex_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      // The VBO containing the positions and sizes of the particles
      glGenBuffers(1, &particles_position_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
      // Initialize with empty (NULL) buffer : it will be updated later, each
      // frame.
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

      // The VBO containing the colors of the particles
      glGenBuffers(1, &particles_color_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      // Initialize with empty (NULL) buffer : it will be updated later, each
      // frame.
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
   }

   template <typename _Rep, typename _Period>
   void startThreads(duration<_Rep, _Period> sleepInterval, function<bool()> haltingCondition) {
      for (int i = 0; i < threadCount; i++)
      {
         simThreads.emplace_back([this, sleepInterval, &haltingCondition, i] { simLoop(i, sleepInterval, haltingCondition); });
      }
   }

   template <typename _Rep, typename _Period>
   void startThreads(duration<_Rep, _Period> sleepInterval, function<bool(milliseconds)> haltingCondition) {
      for (int i = 0; i < threadCount; i++)
      {
         simThreads.emplace_back([this, sleepInterval, &haltingCondition, i] { simLoop(i, sleepInterval, haltingCondition); });
      }
   }

   template <typename _Rep, typename _Period>
   void startThreads(duration<_Rep, _Period> sleepInterval) {
      for (int i = 0; i < threadCount; i++)
      {
         simThreads.emplace_back([this, sleepInterval, i] { simLoop(i, sleepInterval); });
      }
   }
   template <typename _Rep, typename _Period>
   void startDrawThread(duration<_Rep, _Period> sleepInterval) {
      drawThread = thread([this, sleepInterval] {
         glfwMakeContextCurrent(window);
         while (!isClosing) { loop(sleepInterval); }
      });
   }
   template <typename _Rep, typename _Period>
   void startDrawThread(duration<_Rep, _Period> sleepInterval, function<bool()> haltingCondition) {
      drawThread = thread([this, sleepInterval, &haltingCondition] {
         glfwMakeContextCurrent(window);
         while (!haltingCondition() && !isClosing) { loop(sleepInterval); }
      });
   }
   template <typename _Rep, typename _Period>
   void startDrawThread(duration<_Rep, _Period> sleepInterval, function<bool(milliseconds)> haltingCondition) {
      drawThread = thread([this, sleepInterval, &haltingCondition] {
         glfwMakeContextCurrent(window);
         while (!haltingCondition(timeElapsed) && !isClosing) { loop(sleepInterval); }
      });
   }

   template <typename _Rep, typename _Period>
   void loop(duration<_Rep, _Period> sleepInterval) {
      if (swapInterval)
      {
         unique_lock lock(mtx);
         swapInterval = false;
         glfwSwapInterval(1);
      }
      {
         shared_lock lock(mtx);
         tick();
      }
      sleep_for(sleepInterval);
   }

   template <typename _Rep, typename _Period>
   void mainThreadLoop(duration<_Rep, _Period> sleepInterval) {
      glfwPollEvents();
      {
         unique_lock lock(mtx);
         glfwGetFramebufferSize(window, &p_width, &p_height);
         timeElapsed = duration_cast<milliseconds>(high_resolution_clock::now() - p_initialTime);
      }
      sleep_for(sleepInterval);
   }
};

template <typename T, int threadCount>
class Interactivity : public Particulo<T, threadCount>
{
public:
   virtual void onScrollUser(double x, double y) {}
   virtual void onMouseClickUser(int button, int action, int mods) {}
   virtual void onMouseMoveUser(double x, double y) {}
   virtual void onTypeUser(char32_t codepoint) {}
   virtual void onKeyUser(int key, int scancode, int action, int mods) {}
   // Called if the user closes the graphics window but before the program exits
   virtual void onClose() {}

private:
   void onScroll(double x, double y) override {
      auto transform = this->GetTransform();
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
      onScrollUser(x, y);
   }
   void onMouseClick(int button, int action, int mods) override {
      auto [x, y] = this->GetMousePos(ScreenSpace);

      leftMouseDown = button == 0 && action;
      leftMouseUp = button == 0 && !action;
      rightMouseDown = button == 1 && action;
      rightMouseUp = button == 1 && !action;

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
      onMouseClickUser(button, action, mods);
   }
   void onMouseMove(double x, double y) override {
      if (leftMouseDown) { translation = glm::translate(glm::mat4(1.0f), {px + ((x - lmx)) / sf, py + ((y - lmy)) / sf, 0.0f}); }
      onMouseMoveUser(x, y);
   }
   void onType(char32_t codepoint) override {
      if (codepoint == 'f') { this->ToggleFullscreen(); }
      onTypeUser(codepoint);
   }
   void onKey(int key, int scancode, int action, int mods) override {
      if (action)
      {
         if (key == GLFW_KEY_F11) { this->ToggleFullscreen(); }
      }
      onKeyUser(key, scancode, action, mods);
   }

public:
   bool leftMouseDown = false;
   bool leftMouseUp = false;
   bool rightMouseDown = false;
   bool rightMouseUp = false;
   double lmx, lmy, rmx, rmy = 0.0;
   double px, py = 0.0;
   glm::mat4 translation = glm::mat4(1.0f);
   glm::mat4 scale = glm::mat4(1.0f);
   double sf = 1.0;
};
} // namespace Particulo