#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "CloseAllNetworkPeersSystemComponent.h"
#include <INetwork.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <CloseNetworkPeersComponent.h>

using namespace AZ;
using namespace GridMate;

namespace CloseAllNetworkPeers
{
    void CloseAllNetworkPeersSystemComponent::Reflect(ReflectContext* context)
    {
        if (auto sc = azrtti_cast<SerializeContext*>(context))
        {
            sc->Class<CloseAllNetworkPeersSystemComponent,
                    Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (auto ec = sc->GetEditContext())
            {
                ec->Class<CloseAllNetworkPeersSystemComponent>("CloseAllNetworkPeers",
                    "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void CloseAllNetworkPeersSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("CloseAllNetworkPeersService"));
    }

    void CloseAllNetworkPeersSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("CloseAllNetworkPeersService"));
    }

    void CloseAllNetworkPeersSystemComponent::GetRequiredServices(
        AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void CloseAllNetworkPeersSystemComponent::GetDependentServices(
        AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void CloseAllNetworkPeersSystemComponent::CloseAllNetworkPeers()
    {
        AZ_Printf("Gem", "CloseAllNetworkPeers called");

        EBUS_EVENT(CloseAllRequestBus, CloseAll);
    }

    void CloseAllNetworkPeersSystemComponent::OnSessionHosted(
        GridMate::GridSession* session)
    {
        m_session = session;

        if (m_workerEntity == nullptr)
        {
            m_workerEntity = aznew AZ::Entity("Game Player");
            if (m_workerEntity)
            {
                m_workerEntity->CreateComponent<
                    AzFramework::NetBindingComponent>();
                m_workerEntity->CreateComponent<
                    CloseNetworkPeersComponent>();
                m_workerEntity->Init();
                m_workerEntity->Activate();

                AZ::EntityBus::MultiHandler::BusConnect(
                    m_workerEntity->GetId());
            }

            AZ_Assert(m_workerEntity != nullptr,
                "Could not create worker Entity.");
            AZ_Assert(m_workerEntity->GetState() ==
                AZ::Entity::ES_ACTIVE,
                "Worker entity could not be activated.");
        }
    }

    void CloseAllNetworkPeersSystemComponent::OnSessionDelete(
        GridMate::GridSession* session)
    {
        m_session = nullptr;
    }

    void CloseAllNetworkPeersSystemComponent::Activate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusConnect();
        CrySystemEventBus::Handler::BusConnect();
    }

    void CloseAllNetworkPeersSystemComponent::Deactivate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusDisconnect();
        GridMate::SessionEventBus::Handler::BusDisconnect();
        CrySystemEventBus::Handler::BusDisconnect();
        m_session = nullptr;
    }

    void CloseAllNetworkPeersSystemComponent::
    OnCrySystemInitialized(ISystem& system, const SSystemInitParams&)
    {
        GridMate::SessionEventBus::Handler::BusConnect(
            system.GetINetwork()->GetGridMate());
        CrySystemEventBus::Handler::BusDisconnect();
    }
}
