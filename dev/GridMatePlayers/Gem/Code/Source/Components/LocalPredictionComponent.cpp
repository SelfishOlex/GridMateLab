#include "StdAfx.h"
#include "LocalPredictionComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include "AzCore/Component/TransformBus.h"
#include "GridMate/Replica/ReplicaMgr.h"
#include "LmbrCentral/Physics/CryCharacterPhysicsBus.h"
#include <GridMatePlayers/GridMatePlayersBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class LocalPredictionComponent::Chunk : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk() : m_serverCheckpoint("Server Checkpoint") {}
    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "LocalPredictionComponent::Chunk";
    }

    DataSetVectorInTime::BindInterface<
            LocalPredictionComponent,
            &LocalPredictionComponent::OnNewServerCheckpoint>
        m_serverCheckpoint;
};

void LocalPredictionComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<LocalPredictionComponent, Component>()
            ->Version(1)
            ->Field("Allowed Deviation",
                &LocalPredictionComponent::m_allowedDeviation);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<LocalPredictionComponent>(
                "Local Prediction",
                "[Guesses how server will move the character]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                    ->Attribute(Edit::Attributes::Category,
                        "GridMate Players")
                    ->Attribute(Edit::Attributes::
                        AppearsInAddComponentMenu,
                        AZ_CRC("Game"))
                ->DataElement(nullptr,
                    &LocalPredictionComponent::m_allowedDeviation,
                    "Allowed Deviation", "")
                    ->Attribute(Edit::Attributes::Suffix, " m");
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void LocalPredictionComponent::Activate()
{
    const auto self = GetEntityId();
    LocalPredictionRequestBus::Handler::BusConnect(self);

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusConnect(self);
    else
        TickBus::Handler::BusConnect();

    m_isActive = true;
}

void LocalPredictionComponent::Deactivate()
{
    m_isActive = false;
    LocalPredictionRequestBus::Handler::BusDisconnect();

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusDisconnect();
    else
        TickBus::Handler::BusDisconnect();
}

ReplicaChunkPtr LocalPredictionComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void LocalPredictionComponent::SetNetworkBinding(
    ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void LocalPredictionComponent::UnbindFromNetwork()
{
    m_chunk->SetHandler(nullptr);
    m_chunk = nullptr;
}

void LocalPredictionComponent::OnCharacterMoveForward(
    float speed, u32 time)
{
    m_movingForward = true;
    m_speed = speed;
    AZ_Printf("Book", "forward, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnCharacterStop(u32 time)
{
    m_movingForward = false;
    m_speed = 0;
    AZ_Printf("Book", "stopped, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform& world)
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        if (world.GetTranslation() !=
            chunk->m_serverCheckpoint.Get().m_vector)
        {
            AZ_Printf("Book", "server (-- %f --) @ %d",
                static_cast<float>(world.GetTranslation().GetY()),
                GetTime());
        }

        chunk->m_serverCheckpoint.Set(
            { world.GetTranslation(), GetTime() });
    }
}

void LocalPredictionComponent::OnTick(float deltaTime,
    ScriptTimePoint)
{
    EBUS_EVENT_ID(GetEntityId(),
        LmbrCentral::CryCharacterPhysicsRequestBus,
        RequestVelocity,
        Vector3::CreateAxisY(m_speed), 0);
}

Vector3 LocalPredictionComponent::GetPosition() const
{
    auto v = AZ::Vector3::CreateZero();
    EBUS_EVENT_ID_RESULT(v, GetEntityId(), TransformBus,
        GetWorldTranslation);
    return v;
}

AZ::u32 LocalPredictionComponent::GetTime() const
{
    AZ::u32 t = 0;
    EBUS_EVENT_RESULT(t, GridMatePlayersRequestBus, GetLocalTime);
    return t;
}

void LocalPredictionComponent::OnNewServerCheckpoint(
    const VectorInTime& value, const GridMate::TimeContext&)
{
    if (m_isActive &&
        !NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        const auto& serverPos = value.m_vector;
        const auto serverTime = value.m_time;

        if (m_history.HasHistory())
        {
            const auto backThen =
                m_history.GetPositionAt(serverTime);
            const auto diff = backThen - serverPos;
            const auto diffLength = diff.GetLength();

            if (diffLength > m_allowedDeviation)
            {
                Vector3 adjusted;
                if (diffLength > 2 * m_allowedDeviation)
                    adjusted = serverPos;
                else
                    adjusted = GetPosition() - diff;

                m_history.DeleteAfter(serverTime);
                m_history.AddDataPoint(serverPos, serverTime);

                EBUS_EVENT_ID(GetEntityId(), TransformBus,
                    SetWorldTranslation, adjusted);

                AZ_Printf("Book", "new (-- %f --) @%d; dv %f",
                    static_cast<float>(adjusted.GetY()),
                    GetTime(),
                    static_cast<float>(diff.GetY()));
            }
        }
        else
        {
            m_history.AddDataPoint(serverPos, serverTime);
            EBUS_EVENT_ID(GetEntityId(), TransformBus,
                SetWorldTranslation, serverPos);
        }
    }
}
