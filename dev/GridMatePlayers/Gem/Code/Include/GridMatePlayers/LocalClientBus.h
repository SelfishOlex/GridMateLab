#pragma once
#include <AzCore/EBus/EBus.h>
#include <GridMate/Session/Session.h>

namespace AZ
{
    class EntityId;
}

namespace GridMatePlayers
{
    class LocalClientInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~LocalClientInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy
            = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy
            = AZ::EBusAddressPolicy::Single;

        // Put your public methods here
        virtual void AttachToBody(
            GridMate::MemberIDCompact playerId,
            const AZ::EntityId& body) = 0;
    };

    using LocalClientBus = AZ::EBus<LocalClientInterface>;
} // namespace GridMatePlayers