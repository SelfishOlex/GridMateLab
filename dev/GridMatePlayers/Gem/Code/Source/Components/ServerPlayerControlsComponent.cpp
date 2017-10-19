#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include <Components/ServerPlayerControlsComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <LmbrCentral/Physics/CryCharacterPhysicsBus.h>
#include <GridMatePlayers/PlayerActionsBus.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMatePlayers/LocalPredictionRequestBus.h>
#include <GridMate/Replica/ReplicaMgr.h>
#include <GridMatePlayers/NetworkTimeRequestBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class ServerPlayerControlsComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);

    Chunk()
        : m_startForward("Start Forward")
        , m_stopForward("Stop Forward")
        , m_fireCommand("Fire Command") {}

    static const char* GetChunkName()
    {
        return "ServerPlayerControlsComponent::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    GridMate::Rpc<RpcArg<u32>>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnStartForward>
        m_startForward;

    GridMate::Rpc<RpcArg<u32>>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnStopForward>
        m_stopForward;

    GridMate::Rpc<>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnFireCommand>
        m_fireCommand;
};

void ServerPlayerControlsComponent::Reflect(
    AZ::ReflectContext* reflection)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflection))
    {
        sc->Class<ServerPlayerControlsComponent, Component>()
            ->Version(1)
            ->Field("Movement Speed",
                &ServerPlayerControlsComponent::m_speed);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<ServerPlayerControlsComponent>(
                "Server Player Controls",
                "[Passes execution of control to the server]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(
                    Edit::Attributes::AppearsInAddComponentMenu,
                    AZ_CRC("Game"))
                ->DataElement(nullptr,
                    &ServerPlayerControlsComponent::m_speed,
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

void ServerPlayerControlsComponent::Activate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        TickBus::Handler::BusConnect();
    }
    else
    {
        PlayerControlsBus::Handler::BusConnect(GetEntityId());
    }
}

void ServerPlayerControlsComponent::Deactivate()
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        TickBus::Handler::BusDisconnect();
    }
    else
    {
        PlayerControlsBus::Handler::BusDisconnect();
    }
}

ReplicaChunkPtr ServerPlayerControlsComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void ServerPlayerControlsComponent::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void ServerPlayerControlsComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

AZ::u32 ServerPlayerControlsComponent::GetLocalTime() const
{
    AZ::u32 t = 0;
    EBUS_EVENT_RESULT(t, NetworkTimeRequestBus, GetLocalTime);
    return t;
}

void ServerPlayerControlsComponent::ForwardKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_stopForward(GetLocalTime());

        EBUS_EVENT_ID(GetEntityId(),
            LocalPredictionRequestBus,
            OnCharacterStop, GetLocalTime());
    }
}

void ServerPlayerControlsComponent::ForwardKeyDown()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_startForward(GetLocalTime());

        EBUS_EVENT_ID(GetEntityId(),
            LocalPredictionRequestBus,
            OnCharacterMoveForward, m_speed, GetLocalTime());
    }
}

void ServerPlayerControlsComponent::FireKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_fireCommand();
}

void ServerPlayerControlsComponent::OnTick(float deltaTime,
    ScriptTimePoint)
{
    const auto localTime = GetLocalTime();
    while(m_futureActions.size() > 0 &&
        m_futureActions.top().m_time <= localTime)
    {
        auto& action = m_futureActions.top();
        PerformAction(action);
        m_futureActions.pop();
    }

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

bool ServerPlayerControlsComponent::OnStartForward(
    AZ::u32 time, const GridMate::RpcContext&)
{
    const PlayerActionInTime action{
        PlayerActionInTime::ActionType::MoveForward, time };

    if (time > GetLocalTime())
        m_futureActions.push(action);
    else
        PerformAction(action);
    return false;
}

bool ServerPlayerControlsComponent::OnStopForward(
    AZ::u32 time, const GridMate::RpcContext&)
{
    const PlayerActionInTime action{
        PlayerActionInTime::ActionType::Stop, time };

    if (time > GetLocalTime())
        m_futureActions.push(action);
    else
        PerformAction(action);
    return false;
}

void ServerPlayerControlsComponent::PerformAction(
    const PlayerActionInTime& action)
{
    switch(action.m_action)
    {
    case PlayerActionInTime::ActionType::MoveForward:
        m_movingForward = true;
        break;
    case PlayerActionInTime::ActionType::Stop:
        m_movingForward = false;
        break;
    default:
        break;
    }
}

bool ServerPlayerControlsComponent::OnFireCommand(
    const GridMate::RpcContext&)
{
    Vector3 position;
    EBUS_EVENT_ID_RESULT(position, GetEntityId(),
        AZ::TransformBus, GetWorldTranslation);
    position += Vector3::CreateAxisY(1.f);
    EBUS_EVENT(PlayerActionsBus, PlayerFired, position);
    return false;
}
