#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <string>

class Particulo {
protected:
  std::string _title;

private:
  int _width;
  int _height;
  GLFWwindow *_window;
  bool isReady;

public:
  virtual void init() = 0;
  virtual void simulate(float timeElapsed) = 0;
  virtual ~Particulo(){};

public:
  void Create(int width, int height, std::string title = "Particulo") {
    _title = title;
    init();
    gfxInit(width, height);
  }

private:
  void gfxInit(int width, int height) {
    _width = width;
    _height = height;
    // Try to initialize GLFW
    if (!glfwInit())
      exit(EXIT_FAILURE);

    // Try to create a window
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GL_MAJOR_VERSION, 3);
    glfwWindowHint(GL_MINOR_VERSION, 1);
    _window = glfwCreateWindow(_width, _height, "Particulo", NULL, NULL);
    if (!_window)
      exit(EXIT_FAILURE);

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
};
