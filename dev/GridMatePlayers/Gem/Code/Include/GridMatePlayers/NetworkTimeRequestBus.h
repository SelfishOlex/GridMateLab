#pragma once
#include <AzCore/EBus/EBus.h>

namespace GridMatePlayers
{
    class NetworkTimeRequestInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~NetworkTimeRequestInterface() = default;
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        virtual AZ::u32 GetLocalTime() = 0;
    };
    using NetworkTimeRequestBus =
        AZ::EBus<NetworkTimeRequestInterface>;
} // namespace GridMatePlayers
