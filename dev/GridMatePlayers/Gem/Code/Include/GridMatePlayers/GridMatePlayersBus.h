
#pragma once

#include <AzCore/EBus/EBus.h>

namespace GridMatePlayers
{
    class GridMatePlayersRequests
        : public AZ::EBusTraits
    {
    public:
        virtual ~GridMatePlayersRequests() = default;
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        // Put your public methods here
        virtual AZ::u32 GetLocalTime() = 0;
    };
    using GridMatePlayersRequestBus =
        AZ::EBus<GridMatePlayersRequests>;
} // namespace GridMatePlayers
