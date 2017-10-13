
#include "StdAfx.h"

#include <AzTest/AzTest.h>
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/std/smart_ptr/unique_ptr.h"
#include "Utils/MovementTrack.h"

using namespace AZ;
using namespace GridMatePlayers;

class GridMatePlayersTest
    : public ::testing::Test
{
protected:
    void SetUp() override
    {
        AllocatorInstance<SystemAllocator>::Create();
        track = AZStd::make_unique<MovementTrack>();
    }

    void TearDown() override
    {
        track.reset();
        AllocatorInstance<SystemAllocator>::Destroy();
    }

    AZStd::unique_ptr<MovementTrack> track;
};

TEST_F(GridMatePlayersTest, MovementTrack_Empty)
{
    auto t = track->GetTransformAt(10);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);

    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(5)), 10);
}

TEST_F(GridMatePlayersTest, MovementTrack_One)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(5)), 10);

    auto t = track->GetTransformAt(10);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 5);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementTrack_OnSecond)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(0)), 0);
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(5)), 10);

    auto t = track->GetTransformAt(10);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 5);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementTrack_OnFirst)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(0)), 0);
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(5)), 10);

    auto t = track->GetTransformAt(0);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementTrack_InTheMiddle)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(0)), 0);
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(10)), 10);

    auto t = track->GetTransformAt(5);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 5);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementTrack_InFuture)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(0)), 0);
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(10)), 10);

    auto t = track->GetTransformAt(100);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 10);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementTrack_InThePast)
{
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(100)), 100);
    track->AddDataPoint(Transform::CreateTranslation(Vector3::CreateAxisX(200)), 200);

    auto t = track->GetTransformAt(0);
    ASSERT_EQ((float)t.GetTranslation().GetX(), 100);
    ASSERT_EQ((float)t.GetTranslation().GetY(), 0);
    ASSERT_EQ((float)t.GetTranslation().GetZ(), 0);
}

AZ_UNIT_TEST_HOOK();
