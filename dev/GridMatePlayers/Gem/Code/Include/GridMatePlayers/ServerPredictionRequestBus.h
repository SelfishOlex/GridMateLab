#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>

namespace GridMatePlayers
{
    class ServerPredictionRequestInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~ServerPredictionRequestInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void OnCharacterMoveForward(
            const AZ::Vector3& speed,
            AZ::u32 clientTime,
            AZ::u32 serverTime) = 0;

        virtual void OnCharacterStop(
            AZ::u32 clientTime,
            AZ::u32 serverTime) = 0;

        virtual void OnCharacterMoved(
            const AZ::Vector3& world, AZ::u32 serverTime) = 0;
    };

    using ServerPredictionRequestBus =
        AZ::EBus<ServerPredictionRequestInterface>;
}
