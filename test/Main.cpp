#include "StaticResourcePool.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <iostream>

TEST(StaticResourcePoolTests, EmptyPool)
{
    using ItemType = int;
    StaticResourcePool<ItemType, 0> p;

    ASSERT_TRUE(p.FreePoolEmpty());
    ASSERT_TRUE(p.UsedPoolEmpty());

    ASSERT_EQ(p.FreePoolRemaining(), 0);
    ASSERT_EQ(p.UsedPoolRemaining(), 0);

    {
        auto node = p.TakeUsed();
        ASSERT_EQ(node, nullptr);
    }

    {
        auto node = p.TakeFree();
        ASSERT_EQ(node, nullptr);
    }

    {
        bool funcRan = false;
        auto func = [&funcRan](ItemType) { funcRan = true; };
        ASSERT_FALSE(p.ProcessFree(func));
        ASSERT_FALSE(funcRan);
    }

    {
        bool funcRan = false;
        auto func = [&funcRan](ItemType) { funcRan = true; };
        ASSERT_FALSE(p.ProcessUsed(func));
        ASSERT_FALSE(funcRan);
    }
}

// TEST(StaticResourcePoolTests, Single)
// {
//     StaticResourcePool<int, 1> p;

//     auto nodeOpt0 = p.GetUnused();
//     ASSERT_TRUE(nodeOpt0.has_value());

//     auto nodeOpt1 = p.GetUnused();
//     ASSERT_FALSE(nodeOpt1.has_value());

//     auto nodeOpt2 = p.GetUsed();
//     ASSERT_FALSE(nodeOpt2.has_value());

//     auto node0 = nodeOpt0.value();
//     node0->Item() = 1;
//     p.ReturnUsed(node0);

//     auto nodeOpt3 = p.GetUnused();
//     ASSERT_FALSE(nodeOpt3.has_value());

//     auto nodeOpt4 = p.GetUsed();
//     ASSERT_TRUE(nodeOpt4.has_value());

//     auto nodeOpt5 = p.GetUsed();
//     ASSERT_FALSE(nodeOpt5.has_value());

//     auto const& node4 = nodeOpt4.value();
//     ASSERT_EQ(node4->Item(), 1);

//     p.ReturnUnused(node4);
// }

// struct OperationCountingClass
// {
//     static constexpr size_t bigThingSize_ = 1 << 14;
//     std::array<std::uint64_t, bigThingSize_> bigThing_;
//     static size_t ctorDefaultCnt_;
//     static size_t ctorCopyCnt_;
//     static size_t ctorMoveCnt_;
//     static size_t assignCopyCnt_;
//     static size_t assignMoveCnt_;

//     OperationCountingClass()
//         : bigThing_{}
//     {
//         ++ctorDefaultCnt_;
//     }

//     OperationCountingClass(OperationCountingClass const& other)
//     {
//         ++ctorCopyCnt_;
//         bigThing_ = other.bigThing_;
//     }

//     OperationCountingClass(OperationCountingClass&& other)
//     {
//         ++ctorMoveCnt_;
//         bigThing_ = std::move(other.bigThing_);
//     }

//     OperationCountingClass& operator=(OperationCountingClass const& other)
//     {
//         ++assignCopyCnt_;
//         if (this != &other)
//         {
//             bigThing_ = other.bigThing_;
//         }
//         return *this;
//     }

//     OperationCountingClass& operator=(OperationCountingClass&& other)
//     {
//         ++assignMoveCnt_;
//         if (this != &other)
//         {
//             bigThing_ = std::move(other.bigThing_);
//         }
//         return *this;
//     }

//     static void Reset(void)
//     {
//         ctorDefaultCnt_ = 0;
//         ctorCopyCnt_ = 0;
//         ctorMoveCnt_ = 0;
//         assignCopyCnt_ = 0;
//         assignMoveCnt_ = 0;
//     }

//     static void Print(void)
//     {
//         std::cout << "ctorDefaultCnt_: " << ctorDefaultCnt_ << std::endl;
//         std::cout << "ctorCopyCnt_:    " << ctorCopyCnt_ << std::endl;
//         std::cout << "ctorMoveCnt_:    " << ctorMoveCnt_ << std::endl;
//         std::cout << "assignCopyCnt_:   " << assignCopyCnt_ << std::endl;
//         std::cout << "assignMoveCnt_:   " << assignMoveCnt_ << std::endl;
//     }
// };

// size_t OperationCountingClass::ctorDefaultCnt_ = 0;
// size_t OperationCountingClass::ctorCopyCnt_ = 0;
// size_t OperationCountingClass::ctorMoveCnt_ = 0;
// size_t OperationCountingClass::assignCopyCnt_ = 0;
// size_t OperationCountingClass::assignMoveCnt_ = 0;

// TEST(StaticResourcePoolTests, Profiling)
// {
//     OperationCountingClass::Reset();
//     StaticResourcePool<OperationCountingClass, 8> p;

//     std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//     for (size_t i = 0; i < 1000; ++i)
//     {
//         {
//             auto nodeOpt = p.GetUnused();
//             if (nodeOpt.has_value())
//             {
//                 auto& node = nodeOpt.value();
//                 node.Item().bigThing_.at(0) = 1;
//                 p.ReturnUsed(node);
//             }
//         }

//         {
//             auto nodeOpt = p.GetUsed();
//             if (nodeOpt.has_value())
//             {
//                 auto node = nodeOpt.value();
//                 // ASSERT_NE(node.Item().bigThing_.at(0), 0);
//                 p.ReturnUnused(node);
//             }
//         }
//     }
//     std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

//     OperationCountingClass::Print();
//     std::cout << "Time difference = "
//               << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
//               << "[µs]" << std::endl;
//     std::cout << "Time difference = "
//               << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() <<
//               "[ns]"
//               << std::endl;

//     // OperationCountingClass::Reset();

//     // {
//     //     auto nodeOpt = p.GetUnused();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         node->Item().bigThing_.at(0) = 1;
//     //         p.ReturnUsed(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUnused();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         node->Item().bigThing_.at(0) = 2;
//     //         p.ReturnUsed(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUnused();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         node->Item().bigThing_.at(0) = 3;
//     //         p.ReturnUsed(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUnused();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         node->Item().bigThing_.at(0) = 4;
//     //         p.ReturnUsed(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUsed();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         ASSERT_NE(node->Item().bigThing_.at(0), 0);
//     //         p.ReturnUnused(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUsed();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         ASSERT_NE(node->Item().bigThing_.at(0), 0);
//     //         p.ReturnUnused(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUsed();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         ASSERT_NE(node->Item().bigThing_.at(0), 0);
//     //         p.ReturnUnused(node);
//     //     }
//     // }

//     // {
//     //     auto nodeOpt = p.GetUsed();
//     //     if (nodeOpt.has_value())
//     //     {
//     //         auto node = nodeOpt.value();
//     //         ASSERT_NE(node->Item().bigThing_.at(0), 0);
//     //         p.ReturnUnused(node);
//     //     }
//     // }

//     ASSERT_TRUE(false);
// }
