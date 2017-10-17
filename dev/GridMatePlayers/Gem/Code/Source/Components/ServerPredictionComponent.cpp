#include "StdAfx.h"
#include "ServerPredictionComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>

using namespace AZ;
using namespace GridMatePlayers;

void ServerPredictionComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<ServerPredictionComponent, Component>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<ServerPredictionComponent>(
                "Server Prediction",
                "[Adjusts client input to server time]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }
}

void ServerPredictionComponent::Activate()
{
    ServerPredictionRequestBus::Handler::BusConnect(GetEntityId());
    m_history.DeleteAll();
}

void ServerPredictionComponent::Deactivate()
{
    ServerPredictionRequestBus::Handler::BusDisconnect();
    m_history.DeleteAll();
}

void ServerPredictionComponent::OnCharacterMoveForward(
    const Vector3& speed, u32 clientTime, u32 serverTime)
{
    if (m_history.HasHistory())
    {
        if (serverTime >= clientTime)
        {
            const auto posInPast = m_history.GetPositionAt(clientTime);
            const auto adjusted = posInPast + speed * aznumeric_cast<float>(serverTime - clientTime) * 0.001f;
            m_history.DeleteAfter(clientTime);
            m_history.AddDataPoint(adjusted, serverTime);

            AZ_Printf("BOOK", "rewind back to (-- %f --) to %d up to (-- %f --)",
                static_cast<float>(posInPast.GetY()), clientTime,
                static_cast<float>(adjusted.GetY()));

            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM, AZ::Transform::CreateTranslation(adjusted));
        }
        else
        {
            AZ_Warning("BOOK", false, "Don't let clients run behind the server?");
            auto t = AZ::Transform::CreateIdentity();
            EBUS_EVENT_ID_RESULT(t, GetEntityId(),
                AZ::TransformBus, GetWorldTM);

            m_history.AddDataPoint(t.GetTranslation(), serverTime);
        }
    }
    else
    {
        auto t = AZ::Transform::CreateIdentity();
        EBUS_EVENT_ID_RESULT(t, GetEntityId(),
            AZ::TransformBus, GetWorldTM);

        const auto posInPast = t.GetTranslation();
        const auto adjusted = posInPast + speed * aznumeric_cast<float>(serverTime - clientTime) * 0.001f;

        m_history.AddDataPoint(posInPast, clientTime);
        m_history.AddDataPoint(adjusted, serverTime);

        AZ_Printf("BOOK", "starting history (-- %f --) @ %d up to (-- %f --) @ %d",
            static_cast<float>(posInPast.GetY()), clientTime,
            static_cast<float>(adjusted.GetY()), serverTime);

        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
            SetWorldTM, AZ::Transform::CreateTranslation(adjusted));
    }
}

void ServerPredictionComponent::OnCharacterStop(
    u32 clientTime, u32 serverTime)
{
    if (m_history.HasHistory())
    {
        if (serverTime >= clientTime)
        {
            const auto posInPast = m_history.GetPositionAt(clientTime);
            m_history.DeleteAfter(clientTime);
            m_history.AddDataPoint(posInPast, clientTime);

            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM, AZ::Transform::CreateTranslation(posInPast));

            AZ_Printf("BOOK", "rewind back to (-- %f --) to %d",
                static_cast<float>(posInPast.GetY()), clientTime);
        }
        else
        {
            AZ_Warning("BOOK", false, "Don't let clients run behind the server?");
            auto t = AZ::Transform::CreateIdentity();
            EBUS_EVENT_ID_RESULT(t, GetEntityId(),
                AZ::TransformBus, GetWorldTM);

            m_history.AddDataPoint(t.GetTranslation(), serverTime);
        }
    }
    else
    {
        auto t = AZ::Transform::CreateIdentity();
        EBUS_EVENT_ID_RESULT(t, GetEntityId(),
            AZ::TransformBus, GetWorldTM);

        const auto posInPast = t.GetTranslation();
        m_history.AddDataPoint(posInPast, clientTime);

        AZ_Printf("BOOK", "starting history (-- %f --) @ %d to %d",
            static_cast<float>(posInPast.GetY()), clientTime, serverTime);

        EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
            SetWorldTM, AZ::Transform::CreateTranslation(posInPast));
    }
}

void ServerPredictionComponent::OnCharacterMoved(
    const AZ::Vector3& world, AZ::u32 serverTime)
{
    m_history.AddDataPoint(world, serverTime);
}
