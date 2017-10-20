#include "StdAfx.h"
#include "CloseNetworkPeersComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <CrySystemBus.h>
#include <ISystem.h>
#include <IConsole.h>

using namespace AZ;
using namespace CloseAllNetworkPeers;
using namespace GridMate;

class CloseNetworkPeersComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);

    Chunk() : m_closeAllRpc("CloseAllRpc") {}

    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "CloseNetworkPeersComponent::Chunk";
    }

    GridMate::Rpc<>::BindInterface<
        CloseNetworkPeersComponent,
        &CloseNetworkPeersComponent::OnCloseAll> m_closeAllRpc;
};

void CloseNetworkPeersComponent::Reflect(
    AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<AZ::SerializeContext*>(context))
    {
        sc->Class<CloseNetworkPeersComponent, AZ::Component>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<CloseNetworkPeersComponent>(
                "Close All Network Peers",
                "[Closes the clients and the server]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "Testing Tools")
                ->Attribute(Edit::Attributes::
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

void CloseNetworkPeersComponent::GetProvidedServices(
    AZ::ComponentDescriptor::DependencyArrayType& list)
{
    list.push_back(AZ_CRC("CloseNetworkPeersService"));
}

void CloseNetworkPeersComponent::GetIncompatibleServices(
    AZ::ComponentDescriptor::DependencyArrayType& list)
{
}

void CloseNetworkPeersComponent::GetRequiredServices(
    AZ::ComponentDescriptor::DependencyArrayType& list)
{
}

void CloseNetworkPeersComponent::GetDependentServices(
    AZ::ComponentDescriptor::DependencyArrayType& list)
{
}

void CloseNetworkPeersComponent::Activate()
{
    m_isShuttinDown = false;

    CloseAllRequestBus::Handler::BusConnect();
    TickBus::Handler::BusConnect();
}

void CloseNetworkPeersComponent::Deactivate()
{
    CloseAllRequestBus::Handler::BusDisconnect();
    TickBus::Handler::BusDisconnect();

    m_isShuttinDown = false;
}

void CloseNetworkPeersComponent::CloseAll()
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        chunk->m_closeAllRpc();
    }
}

GridMate::ReplicaChunkPtr
CloseNetworkPeersComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void CloseNetworkPeersComponent::SetNetworkBinding(
    GridMate::ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void CloseNetworkPeersComponent::UnbindFromNetwork()
{
    m_chunk->SetHandler(nullptr);
    m_chunk = nullptr;
}

bool CloseNetworkPeersComponent::OnCloseAll(
    const GridMate::RpcContext& /*rc*/)
{
    m_shutdownCountdown = m_ticksBeforeShutdown;
    m_isShuttinDown = true;
    return true;
}

void CloseNetworkPeersComponent::OnTick(float, ScriptTimePoint)
{
    if (m_isShuttinDown && m_shutdownCountdown-- == 0)
    {
        ISystem* system = nullptr;
        EBUS_EVENT_RESULT(system,
            CrySystemRequestBus, GetCrySystem);
        if (system)
        {
            system->GetIConsole()->ExecuteString("quit");
        }
    }
}
