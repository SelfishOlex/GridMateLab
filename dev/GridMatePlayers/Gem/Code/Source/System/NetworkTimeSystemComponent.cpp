#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "NetworkTimeSystemComponent.h"
#include <ISystem.h>
#include <INetwork.h>
#include <GridMatePlayers/NetworkTimeRequestBus.h>

namespace GridMatePlayers
{
    void NetworkTimeSystemComponent::Reflect(
        AZ::ReflectContext* context)
    {
        using namespace AZ;
        if (auto s = azrtti_cast<SerializeContext*>(context))
        {
            s->Class<NetworkTimeSystemComponent, Component>()
                ->Version(1)
                ->Field("Server Clock Delay",
                    &NetworkTimeSystemComponent::m_serverLag);

            if (auto ec = s->GetEditContext())
            {
                ec->Class<NetworkTimeSystemComponent>(
                    "Network Time",
                    "[Provides network time]")
                    ->ClassElement(
                        Edit::ClassElements::EditorData, "")
                    ->Attribute(Edit::Attributes::
                        AppearsInAddComponentMenu,
                        AZ_CRC("System"))
                    ->Attribute(Edit::Attributes::AutoExpand,
                        true)
                    ->DataElement(nullptr,
                        &NetworkTimeSystemComponent::m_serverLag,
                        "Server Clock Delay", "");
            }
        }
    }

    AZ::u32 NetworkTimeSystemComponent::GetLocalTime()
    {
        if (!m_session) return 0;

        const auto localTime = m_session->GetTime();
        if (m_session->IsHost())
            return AZStd::max(localTime - m_serverLag, 0u);

        return localTime;
    }

    void NetworkTimeSystemComponent::OnSessionHosted(
        GridMate::GridSession* session)
    {
        m_session = session;
    }

    void NetworkTimeSystemComponent::OnSessionJoined(
        GridMate::GridSession* session)
    {
        m_session = session;
    }

    void NetworkTimeSystemComponent::OnSessionDelete(
        GridMate::GridSession* session)
    {
        m_session = nullptr;
    }

    void NetworkTimeSystemComponent::Activate()
    {
        NetworkTimeRequestBus::Handler::BusConnect();
        CrySystemEventBus::Handler::BusConnect();
    }

    void NetworkTimeSystemComponent::Deactivate()
    {
        NetworkTimeRequestBus::Handler::BusDisconnect();
        GridMate::SessionEventBus::Handler::BusDisconnect();
        CrySystemEventBus::Handler::BusDisconnect();
        m_session = nullptr;
    }

    void NetworkTimeSystemComponent::OnCrySystemInitialized(
        ISystem& system, const SSystemInitParams&)
    {
        GridMate::SessionEventBus::Handler::BusConnect(
            system.GetINetwork()->GetGridMate());
        CrySystemEventBus::Handler::BusDisconnect();
    }
}
