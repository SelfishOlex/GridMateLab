#include "StdAfx.h"
#include "NetSyncComponent.h"
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <AzCore/Serialization/EditContext.h>
#include <GridMate/Serialize/CompressionMarshal.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace SimplestCPlusPlus;

class NetSyncComponent::Chunk
    : public GridMate::ReplicaChunk
{
public:
    AZ_CLASS_ALLOCATOR(Chunk, AZ::SystemAllocator, 0);

    Chunk() : m_position("position") {}

    bool IsReplicaMigratable() override
    {
        return true;
    }

    static const char* GetChunkName()
    {
        return "NetSyncComponent::Chunk";
    }

    DataSet<AZ::Vector3>::BindInterface<
        NetSyncComponent,
        &NetSyncComponent::OnNewTransform> m_position;
};

void NetSyncComponent::Reflect(ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<NetSyncComponent, Component>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<NetSyncComponent>("Net Sync for Transform",
                "[Sync TransformComponent manually]")
                ->ClassElement(Edit::ClassElements::EditorData,
                    "")
                ->Attribute(Edit::Attributes::Category,
                    "Simplest C++")
                ->Attribute(
                    Edit::Attributes::AppearsInAddComponentMenu,
                    AZ_CRC("Game"))
                ;
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

void NetSyncComponent::Activate()
{
    m_isAuthoritative = NetQuery::IsEntityAuthoritative(
        GetEntityId());
    if (m_isAuthoritative)
        TransformNotificationBus::Handler::BusConnect(
            GetEntityId());
}

void NetSyncComponent::Deactivate()
{
    if (m_isAuthoritative)
        TransformNotificationBus::Handler::BusDisconnect();
}

void NetSyncComponent::OnTransformChanged(
    const Transform& local,
    const Transform& world)
{
    auto chunk = static_cast<Chunk*>(m_chunk.get());
    if (chunk)
    {
        chunk->m_position.Set(world.GetTranslation());
    }
}

void NetSyncComponent::OnNewTransform(
    const AZ::Vector3& v,
    const GridMate::TimeContext& tc)
{
    auto t = AZ::Transform::CreateIdentity();
    t.SetTranslation(v);
    EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
        SetWorldTM, t);
}

ReplicaChunkPtr NetSyncComponent::GetNetworkBinding()
{
    Chunk* replicaChunk =
        GridMate::CreateReplicaChunk<Chunk>();

    replicaChunk->SetHandler(this);
    m_chunk = replicaChunk;

    return m_chunk;
}

void NetSyncComponent::SetNetworkBinding(
    ReplicaChunkPtr replicaChunk)
{
    if (replicaChunk != nullptr)
    {
        replicaChunk->SetHandler(this);
        m_chunk = replicaChunk;
    }
}

void NetSyncComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}