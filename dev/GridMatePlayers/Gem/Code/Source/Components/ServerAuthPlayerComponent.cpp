#include "StdAfx.h"
#include "ServerAuthPlayerComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/RemoteProcedureCall.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMatePlayers/LocalClientBus.h>
#include "AzCore/Component/TransformBus.h"
#include <LmbrCentral/Physics/PhysicsComponentBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class ServerAuthPlayerComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);

    Chunk()
        : m_owningPlayer("Owning Player")
        , m_startingPosition("Starting Position")
    {
    }

    static const char* GetChunkName()
    {
        return "ServerAuthPlayerComponent::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    DataSet<MemberIDCompact>::BindInterface<
        ServerAuthPlayerComponent,
        &ServerAuthPlayerComponent::OnOwningPlayerChanged>
    m_owningPlayer;

    DataSet<Vector3> m_startingPosition;
};

void ServerAuthPlayerComponent::Reflect(
    AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<ServerAuthPlayerComponent, Component>()
          ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<ServerAuthPlayerComponent>(
                  "Server Auth Player Body",
                  "Server Authoritative")
              ->ClassElement(
                  Edit::ClassElements::EditorData, "")
              ->Attribute(Edit::Attributes::Category,
                          "GridMate Players")
              ->Attribute(
                  Edit::Attributes::
                  AppearsInAddComponentMenu,
                  AZ_CRC("Game"));
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void ServerAuthPlayerComponent::SetAssociatedPlayerId(
    const GridMate::MemberIDCompact& player)
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        auto t = Transform::CreateIdentity();
        EBUS_EVENT_ID_RESULT(t, GetEntityId(), AZ::TransformBus,
            GetWorldTM);
        chunk->m_startingPosition.Set(t.GetTranslation());

        chunk->m_owningPlayer.Set(player);
    }
}

bool ServerAuthPlayerComponent::IsAttachedToLocalClient()
{
    return m_connectedToLocalClient;
}

void ServerAuthPlayerComponent::OnLocalClientAttached(
    const GridMate::MemberIDCompact& player)
{
    m_connectedToLocalClient = true;
}

ReplicaChunkPtr ServerAuthPlayerComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void ServerAuthPlayerComponent::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void ServerAuthPlayerComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

void ServerAuthPlayerComponent::Activate()
{
    const auto self = GetEntityId();
    PlayerBodyNotificationBus::MultiHandler::BusConnect(self);
    PlayerBodyRequestBus::Handler::BusConnect(self);

    if (!NetQuery::IsEntityAuthoritative(self))
    {
        m_readyToConnectToBody = true;
        BroadcastNewBody();
    }
}

void ServerAuthPlayerComponent::Deactivate()
{
    PlayerBodyNotificationBus::MultiHandler::BusDisconnect();
    PlayerBodyRequestBus::Handler::BusDisconnect();

    if (!NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        m_readyToConnectToBody = false;
    }
}

void ServerAuthPlayerComponent::OnOwningPlayerChanged(
    const GridMate::MemberIDCompact& value,
    const GridMate::TimeContext& tc)
{
    BroadcastNewBody();
}

void ServerAuthPlayerComponent::BroadcastNewBody()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId())) return;
    if (!m_readyToConnectToBody) return;

    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        auto t = Transform::CreateTranslation(
            chunk->m_startingPosition.Get());
        EBUS_EVENT_ID(GetEntityId(), TransformBus,
            SetWorldTM, t);

        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::PhysicsComponentRequestBus,
            EnablePhysics);

        if (chunk->m_owningPlayer.Get() != 0)
            EBUS_EVENT(LocalClientBus, AttachToBody,
                chunk->m_owningPlayer.Get(), GetEntityId());
    }
}