#include "StaticResourcePool.hpp"
#include "gtest/gtest.h"
#include <list>
#include <random>

// #include <chrono>
// #include <iostream>

TEST(StaticResourcePoolTest, SingleItemTake)
{
    StaticResourcePool<int, 1> p;

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    {
        auto n = p.TakeFree();
        ASSERT_NE(n, nullptr);

        n->Item() = 1;

        p.ReturnUsed(n);
    }

    ASSERT_TRUE(p.FreePoolEmpty());
    ASSERT_FALSE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 0);
    ASSERT_EQ(p.UsedPoolRemaining(), 1);

    {
        auto n = p.TakeUsed();
        ASSERT_NE(n, nullptr);

        ASSERT_EQ(n->Item(), 1);

        p.ReturnFree(n);
    }

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    // ASSERT_TRUE(p.CheckInvariants());
}

TEST(StaticResourcePoolTest, SingleItemProcess)
{
    StaticResourcePool<int, 1> p;

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    ASSERT_TRUE(p.ProcessFree([](int& i) { i = 1; }));

    ASSERT_TRUE(p.FreePoolEmpty());
    ASSERT_FALSE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 0);
    ASSERT_EQ(p.UsedPoolRemaining(), 1);

    ASSERT_TRUE(p.ProcessUsed([](int& i) { ASSERT_EQ(i, 1); }));

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    // ASSERT_TRUE(p.CheckInvariants());
}

TEST(StaticResourcePoolTest, NoCopyOrMove)
{
    struct NoCopyMove
    {
        int a_;

        NoCopyMove()
            : a_{0}
        {
        }

        NoCopyMove(NoCopyMove const&) = delete;
        NoCopyMove(NoCopyMove&&) = delete;
        NoCopyMove& operator=(NoCopyMove const&) = delete;
        NoCopyMove&& operator=(NoCopyMove&&) = delete;
    };

    StaticResourcePool<NoCopyMove, 1> p;

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    {
        auto n = p.TakeFree();
        ASSERT_NE(n, nullptr);

        n->Item().a_ = 1;

        p.ReturnUsed(n);
    }

    ASSERT_TRUE(p.FreePoolEmpty());
    ASSERT_FALSE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 0);
    ASSERT_EQ(p.UsedPoolRemaining(), 1);

    {
        auto n = p.TakeUsed();
        ASSERT_NE(n, nullptr);

        ASSERT_EQ(n->Item().a_, 1);

        p.ReturnFree(n);
    }

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    ASSERT_TRUE(p.ProcessFree([](NoCopyMove& i) { i.a_ = 1; }));

    ASSERT_TRUE(p.FreePoolEmpty());
    ASSERT_FALSE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 0);
    ASSERT_EQ(p.UsedPoolRemaining(), 1);

    ASSERT_TRUE(p.ProcessUsed([](NoCopyMove& i) { ASSERT_EQ(i.a_, 1); }));

    ASSERT_FALSE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 1);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    // ASSERT_TRUE(p.CheckInvariants());
}

TEST(StaticResourcePoolTest, ALotOfOperations)
{
    constexpr size_t aLotOfOperations = 1 << 12;
    constexpr size_t aLotOfThings = 1 << 10;
    using ResourcePoolType = StaticResourcePool<int, aLotOfThings>;
    ResourcePoolType p;

    std::ranlux24_base rng;
    std::list<ResourcePoolType::Node*> takenNodes;
    for (size_t i = 0; i < aLotOfOperations; ++i)
    {
        switch (rng() % 6)
        {
        case 0: {
            auto n = p.TakeFree();
            if (n != nullptr)
            {
                takenNodes.emplace_back(n);
            }
            break;
        }
        case 1: {
            auto n = p.TakeUsed();
            if (n != nullptr)
            {
                takenNodes.emplace_back(n);
            }
            break;
        }
        case 2: {
            if (!takenNodes.empty())
            {
                p.ReturnFree(takenNodes.front());
                takenNodes.pop_front();
            }
            break;
        }
        case 3: {
            if (!takenNodes.empty())
            {
                p.ReturnUsed(takenNodes.front());
                takenNodes.pop_front();
            }
            break;
        }
        case 4: {
            p.ProcessFree([](int& i) { i = 1; });
            break;
        }
        case 5: {
            p.ProcessUsed([](int& i) { i = 0; });
            break;
        }
        }

        // ASSERT_TRUE(p.CheckInvariants());
    }

    ASSERT_EQ(p.FreePoolRemaining() + p.UsedPoolRemaining() + takenNodes.size(), aLotOfThings);

    while (!takenNodes.empty())
    {
        p.ReturnFree(takenNodes.front());
        takenNodes.pop_front();
    }

    ASSERT_EQ(p.FreePoolRemaining() + p.UsedPoolRemaining(), aLotOfThings);

    while (!p.UsedPoolEmpty())
    {
        auto n = p.TakeUsed();
        p.ReturnFree(n);
    }

    ASSERT_EQ(p.FreePoolRemaining(), aLotOfThings);
    // ASSERT_TRUE(p.CheckInvariants());
}

TEST(StaticResourcePoolDeathTest, ReturnNullptr)
{
    {
        StaticResourcePool<int, 1> p;
        EXPECT_DEATH(p.ReturnFree(nullptr), R"--(Assertion `NodeBelongs\(node\)' failed.)--");
    }
    {
        StaticResourcePool<int, 1> p;
        EXPECT_DEATH(p.ReturnUsed(nullptr), R"--(Assertion `NodeBelongs\(node\)' failed.)--");
    }
}

TEST(StaticResourcePoolDeathTest, DuplicateReturn)
{
    {
        StaticResourcePool<int, 1> p;
        auto n = p.TakeFree();
        p.ReturnFree(n);
        EXPECT_DEATH(p.ReturnFree(n), R"--(Assertion `NodeIsTaken\(node\)' failed.)--");
    }
    {
        StaticResourcePool<int, 1> p;
        auto n = p.TakeFree();
        p.ReturnUsed(n);
        EXPECT_DEATH(p.ReturnUsed(n), R"--(Assertion `NodeIsTaken\(node\)' failed.)--");
    }
}

TEST(StaticResourcePoolDeathTest, ReturnToWrongPool)
{
    StaticResourcePool<int, 1> p0;
    StaticResourcePool<int, 1> p1;
    auto n0 = p0.TakeFree();
    EXPECT_DEATH(p1.ReturnFree(n0), R"--(Assertion `NodeBelongs\(node\)' failed.)--");
    EXPECT_DEATH(p1.ReturnUsed(n0), R"--(Assertion `NodeBelongs\(node\)' failed.)--");
}

TEST(StaticResourcePoolDeathTest, ReturnedMoreThanOnce)
{

    {
        StaticResourcePool<int, 1> p;
        auto n = p.TakeFree();
        p.ReturnFree(n);
        EXPECT_DEATH(p.ReturnUsed(n), R"--(Assertion `NodeIsTaken\(node\)' failed.)--");
    }
    {
        StaticResourcePool<int, 1> p;
        auto n = p.TakeFree();
        p.ReturnUsed(n);
        EXPECT_DEATH(p.ReturnFree(n), R"--(Assertion `NodeIsTaken\(node\)' failed.)--");
    }
    {
        StaticResourcePool<int, 2> p;
        auto n0 = p.TakeFree();
        auto n1 = p.TakeFree();
        p.ReturnUsed(n0);
        p.ReturnUsed(n1);
        EXPECT_DEATH(p.ReturnUsed(n0), R"--(Assertion `NodeIsTaken\(node\)' failed.)--");
    }
}

// TEST(StaticResourcePoolTests, Profiling)
// {
//     using BigType = std::array<int, 1 << 14>;
//     StaticResourcePool<BigType, 1> p;

//     std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
//     for (size_t i = 0; i < 1000; ++i)
//     {
//         {
//             auto n = p.TakeFree();
//             if (n != nullptr)
//             {
//                 n->Item().at(0) = 1;
//                 p.ReturnFree(n);
//             }
//         }

//         {
//             auto n = p.TakeUsed();
//             if (n != nullptr)
//             {
//                 n->Item().at(0) = 2;
//                 p.ReturnFree(n);
//             }
//         }
//     }
//     std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
//     auto f = [](BigType& a) { a[0] = 1; };
//     for (size_t i = 0; i < 1000; ++i)
//     {
//         p.ProcessFree(f);
//         p.ProcessUsed(f);
//     }
//     std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

//     std::cout << "Take = " << std::chrono::duration_cast<std::chrono::nanoseconds>(t1 -
//     t0).count()
//               << "[ns]" << std::endl;
//     std::cout << "Process = "
//               << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "[ns]"
//               << std::endl;

//     ASSERT_TRUE(false);
// }