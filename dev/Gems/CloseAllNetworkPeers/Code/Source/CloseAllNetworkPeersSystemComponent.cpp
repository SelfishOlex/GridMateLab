
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "CloseAllNetworkPeersSystemComponent.h"

namespace CloseAllNetworkPeers
{
    void CloseAllNetworkPeersSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<CloseAllNetworkPeersSystemComponent, AZ::Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<CloseAllNetworkPeersSystemComponent>("CloseAllNetworkPeers", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void CloseAllNetworkPeersSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("CloseAllNetworkPeersService"));
    }

    void CloseAllNetworkPeersSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("CloseAllNetworkPeersService"));
    }

    void CloseAllNetworkPeersSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void CloseAllNetworkPeersSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void CloseAllNetworkPeersSystemComponent::Init()
    {
    }

    void CloseAllNetworkPeersSystemComponent::Activate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusConnect();
    }

    void CloseAllNetworkPeersSystemComponent::Deactivate()
    {
        CloseAllNetworkPeersRequestBus::Handler::BusDisconnect();
    }
}
