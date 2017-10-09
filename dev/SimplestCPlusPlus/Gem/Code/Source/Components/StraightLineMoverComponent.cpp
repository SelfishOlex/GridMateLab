#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "StraightLineMoverComponent.h"
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace SimplestCPlusPlus;

void StraightLineMoverComponent::Reflect(ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<StraightLineMoverComponent, Component>()
                 ->Version(1)
                 ->Field("Speed",
                         &StraightLineMoverComponent::m_speed);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<StraightLineMoverComponent>(
                  "Straight Line Mover",
                  "[Moves transform in a straight line]")
              ->ClassElement(Edit::ClassElements::EditorData, "")
              ->Attribute(Edit::Attributes::Category,
                          "Simplest C++")
              ->Attribute(
                  Edit::Attributes::AppearsInAddComponentMenu,
                  AZ_CRC("Game"))
              ->Attribute(Edit::Attributes::AutoExpand, true)
              ->DataElement(nullptr,
                            &StraightLineMoverComponent::m_speed,
                            "Speed",
                            "Speed of movement along the axis.");
        }
    }
}

void StraightLineMoverComponent::Activate()
{
    m_isAuthoritative = NetQuery::IsEntityAuthoritative(
        GetEntityId());
    TickBus::Handler::BusConnect();
}

void StraightLineMoverComponent::Deactivate()
{
    TickBus::Handler::BusDisconnect();
}

void StraightLineMoverComponent::OnTick(
    float deltaTime, ScriptTimePoint)
{
    if (!m_isAuthoritative) return;

    float currentX;
    EBUS_EVENT_ID_RESULT(currentX, GetEntityId(),
        TransformBus, GetWorldX);

    auto newX = currentX + deltaTime * m_speed;
    EBUS_EVENT_ID(GetEntityId(), TransformBus,
        SetWorldX, newX);
}
