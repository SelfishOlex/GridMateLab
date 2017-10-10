#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/LocalClientBus.h>
#include <GridMate/Session/Session.h>

namespace GridMatePlayers
{
    class LocalClientComponent
        : public AZ::Component
        , public LocalClientBus::Handler
        , public GridMate::SessionEventBus::Handler
    {
    public:
        AZ_COMPONENT(LocalClientComponent,
            "{A088C8A8-8568-405A-A409-1576F0D2BCCF}");

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // LocalClientBus implementation
        void AttachToBody(
            GridMate::MemberIDCompact playerId,
            const AZ::EntityId& body) override;

        // GridMate::SessionEventBus implementation
        void OnMemberJoined(
            GridMate::GridSession* session,
            GridMate::GridMember* member) override;

    private:
        GridMate::MemberIDCompact m_selfId = 0;
        AZ::Vector3 m_cameraOffset = AZ::Vector3::CreateZero();
    };
}