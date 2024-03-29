#include <algorithm>
#include <chrono>
#include <cstdint>
#include <math.h>
#include <random>
namespace v2d
{
inline float randMapped() { return (std::rand() / static_cast<float>(std::numeric_limits<int>::max())); }

inline float inv_sqrt(float x) {
   union
   {
      float f;
      uint32_t u;
   } y = {x};
   y.u = 0x5F1FFFF9ul - (y.u >> 1);
   return 0.703952253f * y.f * (2.38924456f - x * y.f * y.f);
}
static std::random_device device;
static std::mt19937 gen = std::mt19937(device());
struct v2d
{
   // Create an empty vector
   v2d() {
      x = 0.0f;
      y = 0.0f;
   }
   // Create a vector from _x and _y
   v2d(const float _x, const float _y) : x(_x), y(_y) {}
   // Create a random vector
   v2d(const float xMin, const float xMax, const float yMin, const float yMax) {
      std::uniform_real_distribution<float> xDistr(xMin, xMax);
      std::uniform_real_distribution<float> yDistr(yMin, yMax);
      x = xDistr(gen);
      y = yDistr(gen);
   }
   // Set vector to _x and _y
   v2d set(float _x, float _y) {
      x = _x;
      y = _y;
      return (*this);
   }
   // Randomize this vector
   v2d randomize(float scale = 1) {
      float r = randMapped() * 2 * 3.141592f;
      set(cos(r) * scale, sin(r) * scale);
      return (*this);
   }

   // Set vector to zero
   void zero() {
      x = 0.0f;
      y = 0.0f;
   }

   v2d operator+(const v2d& r) const { return v2d(this->x + r.x, this->y + r.y); }
   v2d operator+(const float r) const { return v2d(this->x + r, this->y + r); }
   v2d operator+=(const v2d& r) {
      this->x += r.x;
      this->y += r.y;
      return (*this);
   }

   v2d operator-(const v2d& r) const { return v2d(this->x - r.x, this->y - r.y); }
   v2d operator-(const float r) const { return v2d(this->x - r, this->y - r); }
   v2d operator-=(const v2d& r) {
      this->x -= r.x;
      this->y -= r.y;
      return (*this);
   }

   v2d operator*(const v2d& r) const { return v2d(this->x * r.x, this->y * r.y); }
   v2d operator*(const float r) const { return v2d(this->x * r, this->y * r); }
   v2d operator*=(const v2d& r) {
      this->x *= r.x;
      this->y *= r.y;
      return (*this);
   }
   v2d operator*=(const float r) {
      this->x *= r;
      this->y *= r;
      return (*this);
   }

   v2d operator/(const v2d& r) const { return v2d(this->x / r.x, this->y / r.y); }
   v2d operator/(const float r) const { return v2d(this->x / r, this->y / r); }
   v2d operator/=(const v2d& r) {
      this->x /= r.x;
      this->y /= r.y;
      return (*this);
   }
   v2d operator/=(const float r) {
      this->x /= r;
      this->y /= r;
      return (*this);
   }

   // Dot product
   double dot(const v2d& right) const { return (x * right.x + y * right.y); }

   v2d norm() {
      *this = *this * invLen();
      return (*this);
   }
   v2d setLen(float mag) {
      // this->operator/=(len());
      this->operator*=(invLen() * mag);
      //*this = this / len() * mag;
      return (*this);
   }
   float len() {
      return sqrtf((x * x) + (y * y));

      // return(hypotf(x, y));
   }

   float invLen() { return inv_sqrt(sqrLen()); }
   float sqrLen() { return (x * x + y * y); }
   float sqrDist(const v2d& b) {
      v2d a = *this - b;
      return (a.x * a.x + a.y * a.y);
   }

   v2d limit(float b) {
      if (sqrLen() <= b * b) { return (*this); }
      setLen(b);
      return (*this);
   }

   float x;
   float y;
};
} // namespace v2d