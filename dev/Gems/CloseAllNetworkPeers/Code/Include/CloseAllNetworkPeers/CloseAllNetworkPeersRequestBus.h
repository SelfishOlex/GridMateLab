#pragma once
#include <AzCore/EBus/EBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllNetworkPeersRequests
        : public AZ::EBusTraits
    {
    public:
        virtual ~CloseAllNetworkPeersRequests() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        virtual void CloseAllNetworkPeers() = 0;
    };
    using CloseAllNetworkPeersRequestBus =
        AZ::EBus<CloseAllNetworkPeersRequests>;
} // namespace CloseAllNetworkPeers
