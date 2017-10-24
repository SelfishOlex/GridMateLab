#include "StdAfx.h"
#include "InterpolationComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMatePlayers/PlayerBodyBus.h>
#include <GridMatePlayers/NetworkTimeRequestBus.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

void InterpolationComponent::Reflect(ReflectContext* reflection)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflection))
    {
        sc->Class<InterpolationComponent, Component>()
            ->Version(1)
            ->Field("Enable Interpolation",
                &InterpolationComponent::m_enabled)
            ->Field("Delay For Others",
                &InterpolationComponent::m_delayForOthers);

        if (EditContext* ec = sc->GetEditContext())
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
                    &InterpolationComponent::m_delayForOthers,
                    "Delay For Others",
                    "Interpolation delay for other clients")
                    ->Attribute(Edit::Attributes::Suffix, " ms");
    }
}

void InterpolationComponent::Activate()
{
    InterpolationBus::Handler::BusConnect(GetEntityId());
    if (!NetQuery::IsEntityAuthoritative(GetEntityId())
        && m_enabled)
    {
        TickBus::Handler::BusConnect();
    }
}

void InterpolationComponent::Deactivate()
{
    InterpolationBus::Handler::BusDisconnect();
    if (!NetQuery::IsEntityAuthoritative(GetEntityId())
        && m_enabled)
    {
        TickBus::Handler::BusDisconnect();
    }
}

void InterpolationComponent::OnTick(float, ScriptTimePoint)
{
    bool localClient = false;
    EBUS_EVENT_ID_RESULT(localClient, GetEntityId(),
        PlayerBodyRequestBus, IsAttachedToLocalClient);

    if (!localClient && m_history.HasHistory())
    {
        AZ::u32 localTime = 0;
        EBUS_EVENT_RESULT(localTime, NetworkTimeRequestBus,
            GetLocalTime);
        Vector3 updated = m_history.GetPositionAt(
            localTime - m_delayForOthers);
        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
            SetWorldTranslation, updated);
    }
}

void InterpolationComponent::SetWorldTranslation(
    const Vector3& desired, AZ::u32 time)
{
    if (m_enabled)
        m_history.AddDataPoint(desired, time);
    else
        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
            SetWorldTranslation, desired);
}

AZ::Vector3 InterpolationComponent::GetWorldTranslation()
{
    if (m_enabled && m_history.HasHistory())
    {
        AZ::u32 localTime = 0;
        EBUS_EVENT_RESULT(localTime, NetworkTimeRequestBus,
            GetLocalTime);

        return m_history.GetPositionAt(localTime);
    }

    Vector3 v = Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(), AZ::TransformBus,
        GetWorldTranslation);
    return v;
}
