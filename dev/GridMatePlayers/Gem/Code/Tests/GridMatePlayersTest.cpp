
#include "StdAfx.h"

#include <AzTest/AzTest.h>
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/std/smart_ptr/unique_ptr.h"
#include "Utils/MovementHistory.h"

using namespace AZ;
using namespace GridMatePlayers;

class GridMatePlayersTest
    : public ::testing::Test
{
protected:
    void SetUp() override
    {
        AllocatorInstance<SystemAllocator>::Create();
        track = AZStd::make_unique<MovementHistory>();
    }

    void TearDown() override
    {
        track.reset();
        AllocatorInstance<SystemAllocator>::Destroy();
    }

    AZStd::unique_ptr<MovementHistory> track;
};

TEST_F(GridMatePlayersTest, MovementHistory_Empty)
{
    auto t = track->GetPositionAt(10);
    ASSERT_EQ((float)t.GetX(), 0);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);

    track->AddDataPoint(Vector3::CreateAxisX(5), 10);
}

TEST_F(GridMatePlayersTest, MovementHistory_One)
{
    track->AddDataPoint(Vector3::CreateAxisX(5), 10);

    auto t = track->GetPositionAt(10);
    ASSERT_EQ((float)t.GetX(), 5);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_OnSecond)
{
    track->AddDataPoint(Vector3::CreateAxisX(0), 0);
    track->AddDataPoint(Vector3::CreateAxisX(5), 10);

    auto t = track->GetPositionAt(10);
    ASSERT_EQ((float)t.GetX(), 5);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_OnFirst)
{
    track->AddDataPoint(Vector3::CreateAxisX(0), 0);
    track->AddDataPoint(Vector3::CreateAxisX(5), 10);

    auto t = track->GetPositionAt(0);
    ASSERT_EQ((float)t.GetX(), 0);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_InTheMiddle)
{
    track->AddDataPoint(Vector3::CreateAxisX(0), 0);
    track->AddDataPoint(Vector3::CreateAxisX(10), 10);

    auto t = track->GetPositionAt(5);
    ASSERT_EQ((float)t.GetX(), 5);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_InFuture)
{
    track->AddDataPoint(Vector3::CreateAxisX(0), 0);
    track->AddDataPoint(Vector3::CreateAxisX(10), 10);

    auto t = track->GetPositionAt(100);
    ASSERT_EQ((float)t.GetX(), 10);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_InThePast)
{
    track->AddDataPoint(Vector3::CreateAxisX(100), 100);
    track->AddDataPoint(Vector3::CreateAxisX(200), 200);

    auto t = track->GetPositionAt(0);
    ASSERT_EQ((float)t.GetX(), 100);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

TEST_F(GridMatePlayersTest, MovementHistory_DeleteAll)
{
    track->AddDataPoint(Vector3::CreateAxisX(100), 100);
    track->DeleteAll();

    auto t = track->GetPositionAt(0);
    ASSERT_EQ((float)t.GetX(), 0);
    ASSERT_EQ((float)t.GetY(), 0);
    ASSERT_EQ((float)t.GetZ(), 0);
}

AZ_UNIT_TEST_HOOK();
