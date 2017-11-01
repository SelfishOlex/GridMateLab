#include "StdAfx.h"
#include "PebbleSpawnerComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <LmbrCentral/Scripting/SpawnerComponentBus.h>
// for marshaling AZ::Vector3
#include <GridMate/Serialize/MathMarshal.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class PebbleSpawnerComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk() : m_playerFireRpc("PlayerFire") {}

    static const char* GetChunkName()
    {
        return "PebbleSpawnerComponent::Chunk";
    }

    bool IsReplicaMigratable() override { return true; }

    GridMate::Rpc<RpcArg<Vector3>>::BindInterface<
            PebbleSpawnerComponent,
            &PebbleSpawnerComponent::OnPlayerFire>
        m_playerFireRpc;
};

void PebbleSpawnerComponent::Reflect(
    ReflectContext* context)
{
    auto sc = azrtti_cast<SerializeContext*>(context);
    if (sc)
    {
        sc->Class<PebbleSpawnerComponent, Component>()
          ->Version(1);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<PebbleSpawnerComponent>(
                "Pebble Spawner",
                "Executes on server")
                ->ClassElement(Edit::ClassElements::EditorData,
                    "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }

    // We also need to register the chunk type,
    // and this would be a good time to do so.
    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void PebbleSpawnerComponent::Activate()
{
    PlayerActionsBus::Handler::BusConnect();
}

void PebbleSpawnerComponent::Deactivate()
{
    PlayerActionsBus::Handler::BusDisconnect();
}

void PebbleSpawnerComponent::PlayerFired(
    const Vector3& source)
{
    if (Chunk* chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_playerFireRpc(source);
    }
}

ReplicaChunkPtr PebbleSpawnerComponent::GetNetworkBinding()
{
    m_chunk = CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void PebbleSpawnerComponent::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void PebbleSpawnerComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

bool PebbleSpawnerComponent::OnPlayerFire(
    AZ::Vector3 location,
    const GridMate::RpcContext&)
{
    SpawnPebble(location);
    return false; // Don't send to all proxies
}

void PebbleSpawnerComponent::SpawnPebble(
    const AZ::Vector3& start)
{
    Transform t = AZ::Transform::CreateTranslation(start);

    EBUS_EVENT_ID(GetEntityId(),
        LmbrCentral::SpawnerComponentRequestBus,
        SpawnAbsolute, t);
}