#include "StdAfx.h"
#include "InterpolationComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Component/TransformBus.h>

using namespace AZ;
using namespace GridMatePlayers;

void InterpolationComponent::Reflect(ReflectContext* reflection)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflection))
    {
        sc->Class<InterpolationComponent, Component>()
            ->Version(1)
            ->Field("Enable Interpolation",
                &InterpolationComponent::m_enabled)
            ->Field("Interpolation Delay",
                &InterpolationComponent::m_totalTime);

        if (auto ec = sc->GetEditContext())
            ec->Class<InterpolationComponent>(
                "Local Interpolation",
                "Smoothes out network updates")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(
                    Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"))
                ->DataElement(nullptr,
                    &InterpolationComponent::m_enabled,
                    "Enable Interpolation", "")
                ->DataElement(nullptr,
                    &InterpolationComponent::m_totalTime,
                    "Interpolation Delay", "")
                    ->Attribute(Edit::Attributes::Suffix, " sec");
    }
}

void InterpolationComponent::Activate()
{
    m_hasDesired = false;
    m_leftTime = m_totalTime;
    InterpolationBus::Handler::BusConnect(GetEntityId());
    if (m_enabled)
    {
        TickBus::Handler::BusConnect();
    }
}

void InterpolationComponent::Deactivate()
{
    m_hasDesired = false;
    m_leftTime = m_totalTime;
    InterpolationBus::Handler::BusDisconnect();
    if (m_enabled)
    {
        TickBus::Handler::BusDisconnect();
    }
}

void InterpolationComponent::OnTick(float deltaTime,
    ScriptTimePoint)
{
    if (!m_hasDesired) return;

    auto v = Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(),
        AZ::TransformBus, GetWorldTranslation);

    const auto portion = (m_totalTime  - m_leftTime) / m_totalTime;
    const auto updated = v.Lerp(m_desired, portion);

    EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
        SetWorldTranslation, updated);

    m_leftTime -= deltaTime;
    if (m_leftTime <= 0)
    {
        m_leftTime = 0.f;
        m_hasDesired = false;
    }
}

void InterpolationComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform&)
{
}

void InterpolationComponent::SetWorldTranslation(
    const Vector3& desired)
{
    if (m_enabled)
    {
        m_hasDesired = true;
        m_desired = desired;
        m_leftTime = m_totalTime;
    }
    else
    {
        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
            SetWorldTranslation, desired);
    }
}

AZ::Vector3 InterpolationComponent::GetWorldTranslation()
{
    if (m_enabled && m_hasDesired)
    {
        return m_desired;
    }

    auto v = Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(), AZ::TransformBus,
        GetWorldTranslation);
    return v;
}
