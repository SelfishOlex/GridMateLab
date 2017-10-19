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

class ServerPlayerControls::Chunk
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
        return "ServerPlayerControls::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    GridMate::Rpc<RpcArg<u32>>::BindInterface<
        ServerPlayerControls,
        &ServerPlayerControls::OnStartForward>
        m_startForward;

    GridMate::Rpc<RpcArg<u32>>::BindInterface<
        ServerPlayerControls,
        &ServerPlayerControls::OnStopForward>
        m_stopForward;

    GridMate::Rpc<>::BindInterface<
        ServerPlayerControls,
        &ServerPlayerControls::OnFireCommand>
        m_fireCommand;
};

void ServerPlayerControls::Reflect(AZ::ReflectContext* reflection)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflection))
    {
        sc->Class<ServerPlayerControls, Component>()
            ->Version(1)
            ->Field("Movement Speed",
                &ServerPlayerControls::m_speed);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<ServerPlayerControls>(
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
                    &ServerPlayerControls::m_speed,
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

void ServerPlayerControls::Activate()
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

void ServerPlayerControls::Deactivate()
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

ReplicaChunkPtr ServerPlayerControls::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void ServerPlayerControls::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void ServerPlayerControls::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

AZ::u32 ServerPlayerControls::GetLocalTime() const
{
    AZ::u32 t = 0;
    EBUS_EVENT_RESULT(t, NetworkTimeRequestBus, GetLocalTime);
    return t;
}

void ServerPlayerControls::ForwardKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_stopForward(GetLocalTime());

        EBUS_EVENT_ID(GetEntityId(),
            LocalPredictionRequestBus,
            OnCharacterStop, GetLocalTime());
    }
}

void ServerPlayerControls::ForwardKeyDown()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_startForward(GetLocalTime());

        EBUS_EVENT_ID(GetEntityId(),
            LocalPredictionRequestBus,
            OnCharacterMoveForward, m_speed, GetLocalTime());
    }
}

void ServerPlayerControls::FireKeyUp()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_fireCommand();
}

void ServerPlayerControls::OnTick(float deltaTime,
    ScriptTimePoint)
{
    const auto localTime = GetLocalTime();
    for (auto iter = m_futureActions.begin();
        iter!= m_futureActions.end();)
    {
        auto& action = *iter;

        if (action.m_time.count() <= localTime)
        {
            PerformAction(action);
            iter = m_futureActions.erase(iter);
        }
        else
            iter++;
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

bool ServerPlayerControls::OnStartForward(
    AZ::u32 time, const GridMate::RpcContext&)
{
    const PlayerActionInTime action{
        PlayerActionInTime::ActionType::MoveForward, time };

    if (time > GetLocalTime())
        m_futureActions.push_back(action);
    else
        PerformAction(action);
    return false;
}

bool ServerPlayerControls::OnStopForward(
    AZ::u32 time, const GridMate::RpcContext&)
{
    const PlayerActionInTime action{
        PlayerActionInTime::ActionType::Stop, time };

    if (time > GetLocalTime())
        m_futureActions.push_back(action);
    else
        PerformAction(action);
    return false;
}

void ServerPlayerControls::PerformAction(
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

bool ServerPlayerControls::OnFireCommand(
    const GridMate::RpcContext&)
{
    Vector3 position;
    EBUS_EVENT_ID_RESULT(position, GetEntityId(),
        AZ::TransformBus, GetWorldTranslation);
    position += Vector3::CreateAxisY(1.f);
    EBUS_EVENT(PlayerActionsBus, PlayerFired, position);
    return false;
}
