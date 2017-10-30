#include "StdAfx.h"
#include <AzCore/Serialization/EditContext.h>
#include "ServerPlayerControlsComponent.h"
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include <LmbrCentral/Physics/CryCharacterPhysicsBus.h>
#include <GridMatePlayers/PlayerActionsBus.h>
#include "AzCore/Component/TransformBus.h"

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

    GridMate::Rpc<>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnStartForward>
        m_startForward;

    GridMate::Rpc<>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnStopForward>
        m_stopForward;

    GridMate::Rpc<>::BindInterface<
        ServerPlayerControlsComponent,
        &ServerPlayerControlsComponent::OnFireCommand>
        m_fireCommand;
};

void ServerPlayerControlsComponent::Reflect(AZ::ReflectContext* reflection)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflection))
    {
        sc->Class<ServerPlayerControlsComponent, Component>()
            ->Version(1)
            ->Field("Movement Speed",
                &ServerPlayerControlsComponent::m_speed);

        if (EditContext* ec = sc->GetEditContext())
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

void ServerPlayerControlsComponent::ForwardKeyReleased()
{
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_stopForward();
}

void ServerPlayerControlsComponent::ForwardKeyPressed()
{
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_startForward();
}

void ServerPlayerControlsComponent::FireKeyReleased()
{
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
        chunk->m_fireCommand();
}

void ServerPlayerControlsComponent::OnTick(float deltaTime,
    ScriptTimePoint)
{
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
    const GridMate::RpcContext& rc)
{
    m_movingForward = true;
    return false;
}

bool ServerPlayerControlsComponent::OnStopForward(
    const GridMate::RpcContext& rc)
{
    m_movingForward = false;
    return false;
}

bool ServerPlayerControlsComponent::OnFireCommand(
    const GridMate::RpcContext& rc)
{
    Vector3 position;
    EBUS_EVENT_ID_RESULT(position, GetEntityId(),
        AZ::TransformBus, GetWorldTranslation);
    position += Vector3::CreateAxisY(1.f);
    EBUS_EVENT(PlayerActionsBus, PlayerFired, position);
    return false;
}
