#include "Camera.h"

#include "Core/Sampler/ISampler2D.h"
#include "Random/XorShift.h"

namespace Petrichor
{
namespace Core
{
using namespace Math;

Camera::Camera() {}

Camera::Camera(const Vector3f& pos, const Vector3f& dir)
  : pos(pos)
{
    SetViewDir(dir);
}

void
Camera::SetLens(float focalLength)
{
    m_hPerf = m_sensorHeight / focalLength;
}

void
Camera::FocusTo(const Vector3f& target)
{
    m_focusDist = (target - pos).Length();
}

void
Camera::SetFOV(float fov)
{
    m_hPerf = 2.0f * tan(0.5f * fov);
}

void
Camera::SetViewDir(const Vector3f& dir)
{
    m_forward = dir.Normalized();
    m_right = Cross(m_forward, m_worldUp);
    m_up = Cross(m_right, m_forward);
}

void
Camera::LookAt(const Vector3f& target)
{
    SetViewDir(target - pos);
}

Ray
Camera::PixelToRay(
  int i, int j, int imageWidth, int imageHeight, ISampler2D& sampler2D) const
{
    // Random
    auto samplingPoint = sampler2D.Next();
    const float aspect = static_cast<float>(imageWidth) / imageHeight;
    const float u = (i + std::get<0>(samplingPoint)) / imageWidth - 0.5f;
    const float v = (j + std::get<1>(samplingPoint)) / imageHeight - 0.5f;

    Vector3f pointOnSensor =
      pos + m_right * u * m_hPerf * m_focusDist * aspect +
      -m_up * v * m_hPerf * m_focusDist + m_forward * m_focusDist;

    samplingPoint = sampler2D.Next();
    float r = sqrt(std::get<0>(samplingPoint));
    float theta = 2.0f * Math::kPi * std::get<1>(samplingPoint);
    Vector3f pointOnLens = pos + m_right * 0.5f * m_apeture * r * cos(theta) +
                           -m_up * 0.5f * m_apeture * r * sin(theta);

    const Vector3f rayDir = (pointOnSensor - pointOnLens).Normalized();

    float dot = Dot(rayDir, Forward());
    auto weight = Color3f::One() * dot * dot * m_focusDist * m_focusDist /
                  (pointOnSensor - pointOnLens).SquaredLength();

    Ray cameraRay = Ray(pointOnLens, rayDir, RayTypes::Camera);
    cameraRay.weight = weight;

    return cameraRay;
}
} // namespace Core
} // namespace Petrichor
