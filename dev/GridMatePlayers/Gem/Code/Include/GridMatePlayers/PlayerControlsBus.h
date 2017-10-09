#pragma once
#include <AzCore/EBus/EBus.h>

namespace GridMatePlayers
{
    class PlayerControlsInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerControlsInterface() = default;

        ///////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
        ///////////////////////////////////////////////////

        // Put your public methods here
        virtual void ForwardKeyUp() = 0;
        virtual void ForwardKeyDown() = 0;
    };

    using PlayerControlsBus = AZ::EBus<PlayerControlsInterface>;
} // namespace GridMatePlayers