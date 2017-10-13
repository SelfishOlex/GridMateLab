#pragma once
#include <AzCore/EBus/EBus.h>
#include "AzCore/Component/EntityId.h"
#include "AzCore/Math/Transform.h"

namespace GridMatePlayers
{
    class CharacterMoveNotificationInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~CharacterMoveNotificationInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void OnCharacterMoveUpdate(
            const AZ::Vector3& serverPos,
            AZ::u32 time) = 0;
    };

    using CharacterMovementNotificationBus =
        AZ::EBus<CharacterMoveNotificationInterface>;
}
