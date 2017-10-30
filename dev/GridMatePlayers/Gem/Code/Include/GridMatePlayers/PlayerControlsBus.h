#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>

namespace GridMatePlayers
{
    class PlayerControlsInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerControlsInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void ForwardKeyReleased() = 0;
        virtual void ForwardKeyPressed() = 0;
        virtual void FireKeyReleased() = 0;
    };

    using PlayerControlsBus = AZ::EBus<PlayerControlsInterface>;
} // namespace GridMatePlayers