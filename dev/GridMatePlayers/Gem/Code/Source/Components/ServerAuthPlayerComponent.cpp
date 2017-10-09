#include "StdAfx.h"
#include "ServerAuthPlayerComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/RemoteProcedureCall.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMatePlayers/PlayerActionsBus.h>
#include <LmbrCentral/Physics/CryCharacterPhysicsBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMatePlayers/LocalClientBus.h>

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
        : m_startForward("Start Forward")
        , m_stopForward("Stop Forward")
        , m_fireCommand("Fire Command")
        , m_owningPlayer("Owning Player") {}

    static const char* GetChunkName()
    {
        return "ServerAuthPlayerComponent::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    GridMate::Rpc<>::BindInterface<
        ServerAuthPlayerComponent,
        &ServerAuthPlayerComponent::OnStartForward>
    m_startForward;

    GridMate::Rpc<>::BindInterface<
        ServerAuthPlayerComponent,
        &ServerAuthPlayerComponent::OnStopForward>
    m_stopForward;

    GridMate::Rpc<>::BindInterface<
        ServerAuthPlayerComponent,
        &ServerAuthPlayerComponent::OnFireCommand>
        m_fireCommand;

    DataSet<MemberIDCompact>::BindInterface<
        ServerAuthPlayerComponent,
        &ServerAuthPlayerComponent::OnOwningPlayerChanged>
    m_owningPlayer;
};

void ServerAuthPlayerComponent::Reflect(
    AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<ServerAuthPlayerComponent, Component>()
          ->Version(1)
          ->Field("Movement Speed",
                  &ServerAuthPlayerComponent::m_speed);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<ServerAuthPlayerComponent>(
                  "Server Auth Player Controls",
                  "Server Authoritative")
              ->ClassElement(
                  Edit::ClassElements::EditorData, "")
              ->Attribute(Edit::Attributes::Category,
                          "GridMate Players")
              ->Attribute(
                  Edit::Attributes::
                  AppearsInAddComponentMenu,
                  AZ_CRC("Game"))
              ->DataElement(nullptr,
                    &ServerAuthPlayerComponent::m_speed,
                    "Movement Speed", "");
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
        chunk->m_owningPlayer.Set(player);
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
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        TickBus::Handler::BusConnect();
        ServerPlayerBodyBus::Handler::BusConnect(GetEntityId());
    }

    PlayerControlsBus::Handler::BusConnect(GetEntityId());

    m_readyToConnectToBody = true;
    BroadcastNewBody();
}

void ServerAuthPlayerComponent::Deactivate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        TickBus::Handler::BusDisconnect();
        ServerPlayerBodyBus::Handler::BusDisconnect();
    }

    PlayerControlsBus::Handler::BusDisconnect();
    m_readyToConnectToBody = false;
}

void ServerAuthPlayerComponent::ForwardKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_stopForward();
}

void ServerAuthPlayerComponent::ForwardKeyDown()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_startForward();
}

void ServerAuthPlayerComponent::FireKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_fireCommand();
}

void ServerAuthPlayerComponent::OnTick(float deltaTime,
                                       ScriptTimePoint)
{
    if (!NetQuery::IsEntityAuthoritative(GetEntityId()))
        return;

    Vector3 moveDirection;
    if (m_movingForward)
        moveDirection = Vector3::CreateAxisY(m_speed);
    else
        moveDirection = Vector3::CreateAxisY(0);

    EBUS_EVENT_ID(GetEntityId(),
        LmbrCentral::CryCharacterPhysicsRequestBus,
        RequestVelocity,
        moveDirection, 0);
}

bool ServerAuthPlayerComponent::OnStartForward(
    const GridMate::RpcContext& rc)
{
    m_movingForward = true;
    return false;
}

bool ServerAuthPlayerComponent::OnStopForward(
    const GridMate::RpcContext& rc)
{
    m_movingForward = false;
    return false;
}

bool ServerAuthPlayerComponent::OnFireCommand(
    const GridMate::RpcContext& rc)
{
    Vector3 position;
    EBUS_EVENT_ID_RESULT(position, GetEntityId(),
        AZ::TransformBus, GetWorldTranslation);
    position += Vector3::CreateAxisY(1.f);
    EBUS_EVENT(PlayerActionsBus, PlayerFired, position);
    return false;
}

void ServerAuthPlayerComponent::OnOwningPlayerChanged(
    const GridMate::MemberIDCompact& value,
    const GridMate::TimeContext& tc)
{
    BroadcastNewBody();
}

void ServerAuthPlayerComponent::BroadcastNewBody()
{
    if (!m_readyToConnectToBody) return;

    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        if (chunk->m_owningPlayer.Get() != 0)
            EBUS_EVENT(LocalClientBus, AttachToBody,
                chunk->m_owningPlayer.Get(), GetEntityId());
    }
}