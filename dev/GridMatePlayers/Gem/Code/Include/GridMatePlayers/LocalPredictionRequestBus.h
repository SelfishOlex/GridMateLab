#pragma once
#include <AzCore/EBus/EBus.h>
#include "AzCore/Component/EntityId.h"

namespace GridMatePlayers
{
    class LocalPredictionRequestInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~LocalPredictionRequestInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void OnCharacterMoveForward(
            float speed, AZ::u32 time) = 0;

        virtual void OnCharacterStop(AZ::u32 time) = 0;
    };

    using LocalPredictionRequestBus =
        AZ::EBus<LocalPredictionRequestInterface>;
}
