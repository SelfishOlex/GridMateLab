#include "StdAfx.h"
#include "TimedProjectileComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Physics/PhysicsComponentBus.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMatePlayers;

void TimedProjectileComponent::Reflect(
    ReflectContext* context)
{
    auto sc = azrtti_cast<SerializeContext*>(context);
    if (sc)
    {
        sc->Class<TimedProjectileComponent>()
            ->Field("Initial Impulse",
                &TimedProjectileComponent::m_impulse)
            ->Field("Max Life",
                &TimedProjectileComponent::m_maxLifetime)
            ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<TimedProjectileComponent>(
                "Timed Projectile",
                "Moves the object then deletes it")
                ->ClassElement(Edit::ClassElements::EditorData,
                    "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"))
                ->DataElement(nullptr,
                    &TimedProjectileComponent::m_impulse,
                    "Initial Impulse", "")
                ->DataElement(nullptr,
                    &TimedProjectileComponent::m_maxLifetime,
                    "Max Life", "")
                ;
        }
    }
}

void TimedProjectileComponent::Activate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TickBus::Handler::BusConnect();
}

void TimedProjectileComponent::Deactivate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        if (TickBus::Handler::BusIsConnected())
            TickBus::Handler::BusDisconnect();
}

void TimedProjectileComponent::OnTick(
    float deltaTime, ScriptTimePoint)
{
    if (m_lifetime < 1.f)
    {
        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::PhysicsComponentRequestBus,
            AddImpulse, AZ::Vector3::CreateAxisY(
                m_impulse * deltaTime));
    }

    m_lifetime += deltaTime;
    if (m_lifetime > m_maxLifetime)
    {
        TickBus::Handler::BusDisconnect();

        EBUS_EVENT(GameEntityContextRequestBus,
            DestroyGameEntity, GetEntityId());
    }
}