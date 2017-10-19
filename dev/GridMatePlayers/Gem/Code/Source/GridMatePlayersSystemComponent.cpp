#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMatePlayersSystemComponent.h"
#include <ISystem.h>
#include <INetwork.h>

namespace GridMatePlayers
{
    void GridMatePlayersSystemComponent::Reflect(
        AZ::ReflectContext* context)
    {
        using namespace AZ;
        if (auto s = azrtti_cast<SerializeContext*>(context))
        {
            s->Class<GridMatePlayersSystemComponent, Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (auto ec = s->GetEditContext())
            {
                ec->Class<GridMatePlayersSystemComponent>(
                    "GridMatePlayers",
                    "[Provides network time]")
                    ->ClassElement(
                        Edit::ClassElements::EditorData, "")
                        ->Attribute(Edit::Attributes::
                            AppearsInAddComponentMenu,
                            AZ_CRC("System"))
                        ->Attribute(Edit::Attributes::AutoExpand,
                            true);
            }
        }
    }

    AZ::u32 GridMatePlayersSystemComponent::GetLocalTime()
    {
        if (!m_session) return 0;

        const auto localTime = m_session->GetTime();
        if (m_session->IsHost())
            return AZStd::max(localTime - m_serverBehindTime, 0u);

        return localTime;
    }

    void GridMatePlayersSystemComponent::OnSessionHosted(
        GridMate::GridSession* session)
    {
        m_session = session;
    }

    void GridMatePlayersSystemComponent::Activate()
    {
        GridMatePlayersRequestBus::Handler::BusConnect();
        CrySystemEventBus::Handler::BusConnect();
    }

    void GridMatePlayersSystemComponent::Deactivate()
    {
        GridMatePlayersRequestBus::Handler::BusDisconnect();
        GridMate::SessionEventBus::Handler::BusDisconnect();
        CrySystemEventBus::Handler::BusDisconnect();
        m_session = nullptr;
    }

    void GridMatePlayersSystemComponent::OnCrySystemInitialized(
        ISystem& system, const SSystemInitParams&)
    {
        GridMate::SessionEventBus::Handler::BusConnect(
            system.GetINetwork()->GetGridMate());
        CrySystemEventBus::Handler::BusDisconnect();
    }
}
