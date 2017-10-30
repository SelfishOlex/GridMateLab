#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Math/Vector3.h>

namespace GridMatePlayers
{
    class PlayerActionsInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerActionsInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy
            = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy
            = AZ::EBusAddressPolicy::Single;

        virtual void PlayerFired(
            const AZ::Vector3& source) = 0;
    };

    using PlayerActionsBus = AZ::EBus<PlayerActionsInterface>;
} // namespace GridMatePlayers