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
  : m_pos(pos)
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
    m_focusDist = (target - m_pos).Length();
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
    m_right = Cross(m_forward, s_worldUp);
    m_up = Cross(m_right, m_forward);
}

void
Camera::LookAt(const Vector3f& target)
{
    SetViewDir(target - m_pos);
}

Ray
Camera::GenerateRay(
  int i, int j, int imageWidth, int imageHeight, ISampler2D& sampler2D) const
{
    // センサー上の点をランダムに選ぶ
    const Vector3f pointOnSensor = [&]() {
        const auto [rand0, rand1] = sampler2D.Next();
        const float aspect = static_cast<float>(imageWidth) / imageHeight;
        const float u = (i + rand0) / imageWidth - 0.5f;
        const float v = (j + rand1) / imageHeight - 0.5f;

        return m_pos + m_right * u * m_hPerf * m_focusDist * aspect +
               -m_up * v * m_hPerf * m_focusDist + m_forward * m_focusDist;
    }();

    // レンズ上の点をランダムに選ぶ
    const Vector3f pointOnLens = [&]() {
        const auto [rand0, rand1] = sampler2D.Next();
        const float r = sqrt(rand0);
        const float theta = 2.0f * Math::kPi * rand1;

        return m_pos + m_right * 0.5f * m_apeture * r * cos(theta) +
               -m_up * 0.5f * m_apeture * r * sin(theta);
    }();

    // レイの方向
    const Vector3f rayDir = (pointOnSensor - pointOnLens).Normalized();

    // レイが保持する色
    const Vector3f weight = [&]() {
        const float dot = Dot(rayDir, Forward());

        return Color3f::One() * dot * dot * m_focusDist * m_focusDist /
               (pointOnSensor - pointOnLens).SquaredLength();
    }();

    Ray cameraRay = Ray(pointOnLens, rayDir, RayTypes::Camera);
    cameraRay.throughput = weight;

    return cameraRay;
}
} // namespace Core
} // namespace Petrichor
