
#pragma once

#include <AzCore/Component/Component.h>

#include <CloseAllNetworkPeers/CloseAllNetworkPeersBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllNetworkPeersSystemComponent
        : public AZ::Component
        , protected CloseAllNetworkPeersRequestBus::Handler
    {
    public:
        AZ_COMPONENT(CloseAllNetworkPeersSystemComponent, "{B34B496A-264C-4D68-90C3-BCCBBA5E6E60}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // CloseAllNetworkPeersRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
