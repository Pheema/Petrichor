#pragma once

#include "Core/Color3f.h"
#include "Core/Constants.h"
#include "Core/Ray.h"
#include "Core/Texture2D.h"
#include "Math/Vector3f.h"

namespace Petrichor
{
namespace Core
{

class ISampler2D;

class Camera
{
public:
    Math::Vector3f pos;

    Camera();
    Camera(const Math::Vector3f& pos, const Math::Vector3f& dir);

    //! レンズの焦点距離を設定する
    void
    SetLens(float focalLength);

    //! 与えられた座標に焦点をあわせる
    void
    FocusTo(const Math::Vector3f& target);

    //! 垂直画角から焦点距離を指定する
    void
    SetFOV(float fov);

    //! カメラの方向を指定する
    void
    SetViewDir(const Math::Vector3f& dir);

    //! targetの方向に向かせる
    void
    LookAt(const Math::Vector3f& target);

    //! カメラ原点から画素(i, j)に向かう正規化されたベクトルを求める
    Ray
    PixelToRay(int i,
               int j,
               int imageWidth,
               int imageHeight,
               ISampler2D& sampler2D) const;

    //! 焦点距離を返す
    float
    GetFocalLength() const
    {
        return m_sensorHeight / m_hPerf;
    }

    //! 有効口径を返す
    float
    GetApeture() const
    {
        return m_apeture;
    }

    //! 有効口径を設定する
    inline void
    SetApeture(float apeture)
    {
        m_apeture = apeture;
    }

    // 有効口径をF値で指定する
    inline void
    SetFNumber(float fNumber)
    {
        m_apeture = GetFocalLength() / fNumber;
    }

    //! 垂直画角を返す
    float
    GetFOV()
    {
        return 2.0f * atan(0.5f * m_sensorHeight / m_focusDist);
    }

    //! レンズの面積を返す
    float
    GetLensArea() const
    {
        return Math::kPi * (0.5f * m_apeture) * (0.5f * m_apeture);
    }

    //! カメラ右方を示すベクトルを返す
    Math::Vector3f
    Right() const
    {
        return m_right;
    }

    //! カメラ上方を示すベクトルを返す
    Math::Vector3f
    Up() const
    {
        return m_up;
    }

    //! カメラ前方を示すベクトルを返す
    Math::Vector3f
    Forward() const
    {
        return m_forward;
    }

private:
    // カメラの各ローカル軸(正規化されている必要有り)
    Math::Vector3f m_right;
    Math::Vector3f m_up;
    Math::Vector3f m_forward;

    float m_focusDist = 1.0f;
    float m_sensorHeight = 24e-3f;           //! 標準センサーサイズ
    float m_hPerf = m_sensorHeight / 55e-3f; //! 初期レンズは55mm
    float m_apeture = 25e-3f; //! 有効口径(焦点距離(55mm)/F値(2.2))

    constexpr static Math::Vector3f m_worldUp = Math::Vector3f::UnitZ();
};

} // namespace Core
} // namespace Petrichor
