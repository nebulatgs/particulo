#include <tuple>
#define GLFW_INCLUDE_NONE
#include <chrono>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using time_point = std::chrono::system_clock::time_point;

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

#include "shader.hpp"
#include "v2d.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
namespace Particulo
{
// Concepts
template <typename T>
concept BasicParticleXY = (is_arithmetic_v<typeof T::index> && is_arithmetic_v<typeof T::x> &&
                           same_as<typeof T::x, typeof T::y> && same_as<typeof T::radius, float>);

template <typename T>
concept BasicParticleV = (same_as<typeof T::pos, v2d::v2d> && same_as<typeof T::radius, float>);

template <typename T>
concept Particle = (BasicParticleXY<T> || BasicParticleV<T>) &&(constructible_from<T, int> ||
                                                                constructible_from<T, long>);

template <typename T>
concept ColorfulParticle = (Particle<T> && same_as<uint32_t, typeof T::color>);

template <typename T>
concept PlainParticle = (Particle<T> && !ColorfulParticle<T>);

// Structs
struct RGBA
{
   float r;
   float g;
   float b;
   float a;
};
// Main
template <ColorfulParticle T>
class Particulo
{
protected:
   const string& GetTitle() const { return p_title; }
   const glm::mat4& GetTransform() const { return p_transform; }
   const float GetWidth() const { return p_width; }
   const float GetHeight() const { return p_height; }
   const float GetMaxCount() const { return p_maxCount; }
   const time_point& GetInitialTime() const { return p_initialTime; }

protected:
   void SetTitle(string title)
   {
      p_title = title;
      glfwSetWindowTitle(window, title.c_str());
   }
   void SetTransform(glm::mat4 transform) { p_transform = transform; }
   T GetNewParticle() const { return T(++maxParticleIndex); }
   // You probably want to use GetNewParticle to get a particle with an assigned index before
   // calling this method
   void Add(const T& particle) { particles.push_back(make_shared<T>(particle)); }
   template <typename... _Args>
   void Add(_Args&&... __args)
   {
      particles.push_back(make_shared<T>(++maxParticleIndex, __args...));
   }

private:
   string p_title;
   glm::mat4 p_transform = glm::mat4(1.0f);
   int p_width;
   int p_height;
   int p_maxCount;
   time_point p_initialTime;

private:
   GLuint VAO;
   Shader shader;
   GLuint particles_position_buffer;
   GLuint particles_color_buffer;
   GLuint billboard_vertex_buffer;
   vector<GLfloat> particle_position_size_data;
   vector<GLfloat> particle_color_data;

private:
   vector<shared_ptr<T>> particles;
   GLFWwindow* window;
   bool isReady;
   milliseconds timeElapsed;
   RGBA bgColor = {0.0f, 0.0f, 0.0f, 1.0f};
   mutable int maxParticleIndex;
   glm::mat4 screenCorrectionTransform = glm::mat4(1.0f);
   glm::mat4 identity = glm::mat4(1.0f);

public:
   virtual void init() {}
   virtual void simulate(vector<shared_ptr<T>>& particles, milliseconds timeElapsed) = 0;
   virtual ~Particulo(){};

public:
   void SetBGColor(float r, float g, float b, float a = 1.0f) { bgColor = {r, g, b, a}; }
   void SetBGColor(RGBA color) { bgColor = color; }
   void SetBGColor(uint32_t color)
   {
      auto [r, g, b, a] = uint32ToFloatColor(color);
      bgColor = {
          r,
          g,
          b,
          a,
      };
   }
   void Create(int width, int height, int initialCount, string title = "Particulo",
               int maxCount = 1 << 14)
   {
      if (initialCount > maxCount)
         throw std::runtime_error("Attempted to exceed the max particle count during creation");
      p_initialTime = high_resolution_clock::now();
      p_maxCount = maxCount;
      p_title = title;
      for (int i = 0; i < initialCount; i++) { particles.push_back(make_shared<T>(i)); }
      maxParticleIndex = initialCount - 1;
      gfxInit(width, height);
      bufferInit();
      init();
   }

   void tick()
   {
      simulate(particles, timeElapsed);
      commonDraw();
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

private:
   void commonDraw()
   {
      glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
      glClear(GL_COLOR_BUFFER_BIT);
      glfwGetFramebufferSize(window, &p_width, &p_height);
      glViewport(0, 0, p_width, p_height);
      glOrtho(0, p_width, p_height, 0, 1, -1);
      screenCorrectionTransform =
          glm::scale(identity, {2.0f / static_cast<float>(p_width),
                                -2.0f / static_cast<float>(p_height), 1.0f});
      auto tempMatrix = screenCorrectionTransform * p_transform;
      tempMatrix = glm::translate(tempMatrix, {p_width / -2.0f, p_height / -2.0f, 0.0f});
      shader.SetMatrix4("transform", tempMatrix, true);
      setParticlePos();
      updateBuffers();
      draw();
      glfwSwapBuffers(window);
   }

   std::tuple<float, float, float, float> uint32ToFloatColor(uint32_t color)
   {
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

   void setParticlePos() requires(BasicParticleXY<T>)
   {
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

   void setParticlePos() requires(BasicParticleV<T>)
   {
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

   void draw() requires(ColorfulParticle<T>)
   {
      // 1st attribute buffer : vertices
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
      glVertexAttribPointer(
          0, // attribute. No particular reason for 0, but must match the layout in the shader.
          3, // size
          GL_FLOAT, // type
          GL_FALSE, // normalized?
          0,        // stride
          (void*) 0 // array buffer offset
      );

      // 2nd attribute buffer : positions of particles' centers
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
      glVertexAttribPointer(
          1, // attribute. No particular reason for 1, but must match the layout in the shader.
          4, // size : x + y + z + size => 4
          GL_FLOAT, // type
          GL_FALSE, // normalized?
          0,        // stride
          (void*) 0 // array buffer offset
      );

      // 3rd attribute buffer : particles' colors
      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      glVertexAttribPointer(
          2, // attribute. No particular reason for 2, but must match the layout in the shader.
          4, // size : r + g + b + a => 4
          GL_FLOAT, // type
          GL_TRUE,  // normalized? *** YES, this means that the unsigned char[4] will be accessible
                    // with a vec4 (floats) in the shader ***
          0,        // stride
          (void*) 0 // array buffer offset
      );

      // cout << glVertexAttribDivisor << endl;
      glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
      glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
      glVertexAttribDivisor(2, 1); // color : one per quad -> 1

      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, p_maxCount);
   }

private:
   void gfxInit(int width, int height)
   {
      p_width = width;
      p_height = height;
      // Try to initialize GLFW
      if (!glfwInit()) throw std::runtime_error("Unable to initialize GLFW");

      // Try to create a window
      glfwWindowHint(GLFW_SAMPLES, 4);
      glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
      // glfwWindowHint(GL_MAJOR_VERSION, 3);
      // glfwWindowHint(GL_MINOR_VERSION, 1);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
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
      glfwSwapInterval(1);
      isReady = true;
   }

   void updateBuffers()
   {
      glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL,
                   GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See
                                    // above link for details.
      glBufferSubData(GL_ARRAY_BUFFER, 0, p_maxCount * sizeof(GLfloat) * 4,
                      particle_position_size_data.data());

      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL,
                   GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See
                                    // above link for details.
      glBufferSubData(GL_ARRAY_BUFFER, 0, p_maxCount * sizeof(GLfloat) * 4,
                      particle_color_data.data());
   }

   void bufferInit() requires(ColorfulParticle<T>)
   {
      particle_position_size_data.resize(p_maxCount * 4);
      particle_color_data.resize(p_maxCount * 4);
      particles.reserve(p_maxCount);
      shader.Compile("../../src/particulo/circle.vert", "../../src/particulo/circle.frag");
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
      // Initialize with empty (NULL) buffer : it will be updated later, each frame.
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

      // The VBO containing the colors of the particles
      glGenBuffers(1, &particles_color_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
      // Initialize with empty (NULL) buffer : it will be updated later, each frame.
      glBufferData(GL_ARRAY_BUFFER, p_maxCount * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

      // particles.emplace_back(5000000, glm::vec4(0.0, 0.722, 0.58, 1.0));
      // g_particule_color_data.push_back(particles.back().color.r);
      // g_particule_color_data.push_back(particles.back().color.g);
      // g_particule_color_data.push_back(particles.back().color.b);
      // g_particule_color_data.push_back(particles.back().color.a);
      // for (int i = 0; i < maxCount - 1; i++)
      // {
      //    particles.emplace_back((i % 3 > 1 ? glm::vec4(0.424, 0.361, 0.906, 0.75)
      //                                      : glm::vec4(0.035, 0.518, 0.89, 0.75)));
      //    g_particule_color_data.push_back(particles.back().color.r);
      //    g_particule_color_data.push_back(particles.back().color.g);
      //    g_particule_color_data.push_back(particles.back().color.b);
      //    g_particule_color_data.push_back(particles.back().color.a);
      // }
   }

private:
   template <typename _Rep, typename _Period>
   void loop(duration<_Rep, _Period> sleepInterval)
   {
      tick();
      sleep_for(sleepInterval);
      timeElapsed = duration_cast<milliseconds>(high_resolution_clock::now() - p_initialTime);
   }
};
} // namespace Particulo