#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "CloseAllNetworkPeersSystemComponent.h"
#include <INetwork.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <CloseNetworkPeersComponent.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;

namespace CloseAllNetworkPeers
{
    void CloseAllPeersSystemComponent::Reflect(ReflectContext* c)
    {
        if (auto sc = azrtti_cast<SerializeContext*>(c))
        {
            sc->Class<CloseAllPeersSystemComponent, Component>()
                ->Version(0)->SerializerForEmptyClass();

            if (auto ec = sc->GetEditContext())
            {
                ec->Class<CloseAllPeersSystemComponent>(
                    "Close All Network Peers",
                    "[Closes all connected network peers]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,AZ_CRC("System"))
                ->Attribute(Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void CloseAllPeersSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("CloseAllPeersService"));
    }

    void CloseAllPeersSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompat)
    {
        incompat.push_back(AZ_CRC("CloseAllPeersService"));
    }

    void CloseAllPeersSystemComponent::GetRequiredServices(
        AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void CloseAllPeersSystemComponent::GetDependentServices(
        AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void CloseAllPeersSystemComponent::CloseAllNetworkPeers()
    {
        AZ_Printf("Gem", "CloseAllNetworkPeers called");

        EBUS_EVENT(CloseAllRequestBus, CloseAll);
    }

    void CloseAllPeersSystemComponent::Activate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusConnect();
        GameEntityContextEventBus::Handler::BusConnect();
    }

    void CloseAllPeersSystemComponent::Deactivate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusDisconnect();
        GameEntityContextEventBus::Handler::BusDisconnect();
    }

    void CloseAllPeersSystemComponent::OnGameEntitiesStarted()
    {
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

    void CloseAllPeersSystemComponent::OnGameEntitiesReset()
    {
        m_workerEntity = nullptr;
    }
}
