#include "StdAfx.h"
#include "CharacterTrackingComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include "LmbrCentral/Physics/CryCharacterPhysicsBus.h"
#include "AzCore/Component/TransformBus.h"
#include "GridMate/Replica/ReplicaMgr.h"

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class CharacterTrackingComponent::Chunk
    : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk()
        : m_serverCheckpoint("Server Check Point") {}

    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "CharacterTrackingComponent::Chunk";
    }

    DataSet<TransformWithTime, TransformWithTime::Marshaler>::
        BindInterface<CharacterTrackingComponent,
        &CharacterTrackingComponent::OnNewCheckpoint>
    m_serverCheckpoint;
};

void CharacterTrackingComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<CharacterTrackingComponent>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<CharacterTrackingComponent>(
                "CharacterTrackingComponent", "[Description]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMatePlayers");
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void CharacterTrackingComponent::Activate()
{
    const auto self = GetEntityId();
    CharacterMovementRequestBus::Handler::BusConnect(self);
    CharacterMovementNotificationBus::Handler::BusConnect(self);
    TickBus::Handler::BusConnect();
}

void CharacterTrackingComponent::Deactivate()
{
    CharacterMovementRequestBus::Handler::BusDisconnect();
    CharacterMovementNotificationBus::Handler::BusDisconnect();
    TickBus::Handler::BusDisconnect();
}

ReplicaChunkPtr CharacterTrackingComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void CharacterTrackingComponent::SetNetworkBinding(
    ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void CharacterTrackingComponent::UnbindFromNetwork()
{
    if (m_chunk)
    {
        m_chunk->SetHandler(nullptr);
        m_chunk = nullptr;
    }
}

void CharacterTrackingComponent::OnCharacterMoveUpdate(
    const Transform& serverTransform, u32 timestamp)
{
    if (m_chunk && m_chunk->IsProxy())
    {
        const auto local = m_movePoints.GetTransformAt(timestamp);
        if (!Compare(local, serverTransform))
        {
            // Correction
            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM, serverTransform);

            m_movePoints.AddDataPoint(serverTransform, timestamp);
        }
    }
}

void CharacterTrackingComponent::OnCharacterMoveForward(u32 time)
{
    if (m_chunk && m_chunk->IsProxy())
    {
        const auto moveDirection = Vector3::CreateAxisY(m_speed);

        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::CryCharacterPhysicsRequestBus,
            RequestVelocity,
            moveDirection, 0);

        m_movePoints.AddDataPoint(GetLocalTM(), time);
    }
}

void CharacterTrackingComponent::OnCharacterStop(u32 time)
{
    if (m_chunk && m_chunk->IsProxy())
    {
        EBUS_EVENT_ID(GetEntityId(),
            LmbrCentral::CryCharacterPhysicsRequestBus,
            RequestVelocity,
            Vector3::CreateZero(), 0);

        m_movePoints.AddDataPoint(GetLocalTM(), time);
    }
}

void CharacterTrackingComponent::OnTick(float, ScriptTimePoint)
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        {
            chunk->m_serverCheckpoint.Set(
                TransformWithTime(GetLocalTM(), GetTime()));
        }
    }
}

Transform CharacterTrackingComponent::GetLocalTM()
{
    auto t = AZ::Transform::CreateIdentity();
    EBUS_EVENT_ID_RESULT(t, GetEntityId(), AZ::TransformBus,
        GetWorldTM);
    return t;
}

AZ::u32 CharacterTrackingComponent::GetTime()
{
    if (!m_chunk) return 0;
    return m_chunk->GetReplicaManager()->GetTime().m_localTime;
}

bool CharacterTrackingComponent::Compare(
    const AZ::Transform& one,
    const AZ::Transform& two) const
{
    const auto diff = one.GetTranslation() - two.GetTranslation();
    return diff.GetLengthSq() < 0.1f;
}

bool CharacterTrackingComponent::TransformWithTime::operator==(
    const TransformWithTime& another) const
{
    return another.m_time == m_time &&
        another.m_transform == m_transform;
}

void CharacterTrackingComponent::OnNewCheckpoint(
    const TransformWithTime& value, const TimeContext &tc)
{
    OnCharacterMoveUpdate(value.m_transform, value.m_time);
}
