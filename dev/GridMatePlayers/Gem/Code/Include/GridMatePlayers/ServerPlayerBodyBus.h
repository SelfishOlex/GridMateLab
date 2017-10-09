#pragma once
#include <AzCore/EBus/EBus.h>
#include <GridMate/Session/Session.h>

namespace AZ
{
    class EntityId;
}

namespace GridMatePlayers
{
    class ServerPlayerBodyInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~ServerPlayerBodyInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        // Put your public methods here
        virtual void SetAssociatedPlayerId(
            const GridMate::MemberIDCompact& player) = 0;
    };

    using ServerPlayerBodyBus = AZ::EBus<ServerPlayerBodyInterface>;
} // namespace GridMatePlayers