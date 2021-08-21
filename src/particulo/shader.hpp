#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using std::ifstream;
using std::istreambuf_iterator;
using std::string;

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Particulo
{
struct Shader
{
   unsigned int ID;
   Shader& Use()
   {
      glUseProgram(this->ID);
      return *this;
   }

   const string ReadFile(const string& filename)
   {
      ifstream input_file(filename);
      if (!input_file.is_open()) throw "Unable to open file " + filename;
      return string((istreambuf_iterator<char>(input_file)), istreambuf_iterator<char>());
   }

   void CompileStrings(const string& vertexShader, const string& fragmentShader)
   {
      // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

      GLint Result = GL_FALSE;
      int InfoLogLength;

      // Compile Vertex Shader
      printf("Compiling Vertex Shader\n");
      char const* VertexSourcePointer = vertexShader.c_str();
      glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
      glCompileShader(VertexShaderID);

      // Check Vertex Shader
      glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
         glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
         printf("%s\n", &VertexShaderErrorMessage[0]);
      }

      // Compile Fragment Shader
      printf("Compiling Fragment Shader\n");
      char const* FragmentSourcePointer = fragmentShader.c_str();
      glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
      glCompileShader(FragmentShaderID);

      // Check Fragment Shader
      glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
         glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
         printf("%s\n", &FragmentShaderErrorMessage[0]);
      }

      // Link the program
      printf("Linking program\n");
      GLuint ProgramID = glCreateProgram();
      glAttachShader(ProgramID, VertexShaderID);
      glAttachShader(ProgramID, FragmentShaderID);
      glLinkProgram(ProgramID);

      // Check the program
      glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
      glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
         glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
         printf("%s\n", &ProgramErrorMessage[0]);
      }

      glDetachShader(ProgramID, VertexShaderID);
      glDetachShader(ProgramID, FragmentShaderID);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);

      ID = ProgramID;
   }

   void Compile(const char* vertex_file_path, const char* fragment_file_path)
   {

      // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

      // Read the Vertex Shader code from the file
      std::string VertexShaderCode;
      std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
      if (VertexShaderStream.is_open())
      {
         std::stringstream sstr;
         sstr << VertexShaderStream.rdbuf();
         VertexShaderCode = sstr.str();
         VertexShaderStream.close();
      }
      else
      {
         printf(
             "Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ "
             "!\n",
             vertex_file_path);
         getchar();
         return;
      }

      // Read the Fragment Shader code from the file
      std::string FragmentShaderCode;
      std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
      if (FragmentShaderStream.is_open())
      {
         std::stringstream sstr;
         sstr << FragmentShaderStream.rdbuf();
         FragmentShaderCode = sstr.str();
         FragmentShaderStream.close();
      }
      else
      {
         printf(
             "Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ "
             "!\n",
             vertex_file_path);
         getchar();
         return;
      }

      GLint Result = GL_FALSE;
      int InfoLogLength;

      // Compile Vertex Shader
      printf("Compiling shader : %s\n", vertex_file_path);
      char const* VertexSourcePointer = VertexShaderCode.c_str();
      glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
      glCompileShader(VertexShaderID);

      // Check Vertex Shader
      glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
         glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
         printf("%s\n", &VertexShaderErrorMessage[0]);
      }

      // Compile Fragment Shader
      printf("Compiling shader : %s\n", fragment_file_path);
      char const* FragmentSourcePointer = FragmentShaderCode.c_str();
      glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
      glCompileShader(FragmentShaderID);

      // Check Fragment Shader
      glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
         glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
         printf("%s\n", &FragmentShaderErrorMessage[0]);
      }

      // Link the program
      printf("Linking program\n");
      GLuint ProgramID = glCreateProgram();
      glAttachShader(ProgramID, VertexShaderID);
      glAttachShader(ProgramID, FragmentShaderID);
      glLinkProgram(ProgramID);

      // Check the program
      glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
      glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0)
      {
         std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
         glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
         printf("%s\n", &ProgramErrorMessage[0]);
      }

      glDetachShader(ProgramID, VertexShaderID);
      glDetachShader(ProgramID, FragmentShaderID);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);

      ID = ProgramID;
   }

   void SetFloat(const char* name, float value, bool useShader)
   {
      if (useShader) this->Use();
      glUniform1f(glGetUniformLocation(this->ID, name), value);
   }
   void SetInteger(const char* name, int value, bool useShader)
   {
      if (useShader) this->Use();
      glUniform1i(glGetUniformLocation(this->ID, name), value);
   }
   void SetVector2f(const char* name, float x, float y, bool useShader)
   {
      if (useShader) this->Use();
      glUniform2f(glGetUniformLocation(this->ID, name), x, y);
   }
   void SetVector2f(const char* name, const glm::vec2& value, bool useShader)
   {
      if (useShader) this->Use();
      glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
   }
   void SetVector3f(const char* name, float x, float y, float z, bool useShader)
   {
      if (useShader) this->Use();
      glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
   }
   void SetVector3f(const char* name, const glm::vec3& value, bool useShader)
   {
      if (useShader) this->Use();
      glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
   }
   void SetVector4f(const char* name, float x, float y, float z, float w, bool useShader)
   {
      if (useShader) this->Use();
      glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
   }
   void SetVector4f(const char* name, const glm::vec4& value, bool useShader)
   {
      if (useShader) this->Use();
      glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
   }
   void SetMatrix4(const char* name, const glm::mat4& matrix, bool useShader)
   {
      if (useShader) this->Use();
      glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
   }

   void checkCompileErrors(unsigned int object, std::string type)
   {
      int success;
      char infoLog[1024];
      if (type != "PROGRAM")
      {
         glGetShaderiv(object, GL_COMPILE_STATUS, &success);
         if (!success)
         {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
         }
      }
      else
      {
         glGetProgramiv(object, GL_LINK_STATUS, &success);
         if (!success)
         {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
         }
      }
   }
};
} // namespace Particulo