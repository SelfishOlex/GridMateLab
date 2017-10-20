#pragma once
#include <AzCore/EBus/EBus.h>
#include <GridMate/Session/Session.h>
#include <AzCore/Component/ComponentBus.h>

namespace GridMatePlayers
{
    class PlayerBodyRequestInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~PlayerBodyRequestInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void SetAssociatedPlayerId(
            const GridMate::MemberIDCompact& player) = 0;

        virtual bool IsAttachedToLocalClient() { return false; }
    };

    using PlayerBodyRequestBus =
        AZ::EBus<PlayerBodyRequestInterface>;

    class PlayerBodyNotificationInterface
        : public AZ::ComponentBus
    {
    public:
        virtual ~PlayerBodyNotificationInterface() = default;

        virtual void OnLocalClientAttached(
            const GridMate::MemberIDCompact& player) = 0;
    };

    using PlayerBodyNotificationBus =
        AZ::EBus<PlayerBodyNotificationInterface>;
} // namespace GridMatePlayers