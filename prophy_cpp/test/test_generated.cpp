#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "util.hpp"
#include "out/Composite.pp.hpp"
#include "out/CompositeDynamicArray.pp.hpp"
#include "out/CompositeFixedArray.pp.hpp"
#include "out/CompositeGreedyArray.pp.hpp"
#include "out/CompositeLimitedArray.pp.hpp"
#include "out/ConstantTypedefEnum.pp.hpp"
#include "out/DynamicComposite.pp.hpp"
#include "out/DynamicCompositeComposite.pp.hpp"
#include "out/DynamicCompositeDynamicArray.pp.hpp"
#include "out/DynamicCompositeGreedyArray.pp.hpp"
#include "out/ManyArrays.pp.hpp"
#include "out/ManyArraysMixed.pp.hpp"
#include "out/ManyArraysMixedHeavily.pp.hpp"
#include "out/ManyArraysPadding.pp.hpp"
#include "out/ManyArraysTailFixed.pp.hpp"
#include "out/ManyDynamic.pp.hpp"
#include "out/Optional.pp.hpp"
#include "out/Scalar.pp.hpp"
#include "out/ScalarDynamicArray.pp.hpp"
#include "out/ScalarFixedArray.pp.hpp"
#include "out/ScalarGreedyArray.pp.hpp"
#include "out/ScalarLimitedArray.pp.hpp"
#include "out/Union.pp.hpp"

using namespace testing;

TEST(generated, Composite)
{
    data x(
        "\x01\x00\x00\x02"
        "\x01\x00\x00\x02",

        "\x01\x00\x02\x00"
        "\x01\x00\x02\x00"
    );

    Composite* next = prophy::swap(reinterpret_cast<Composite*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 8);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, CompositeDynamicArray)
{
    data x(
        "\x00\x00\x00\x03"
        "\x01\x00\x00\x01"
        "\x02\x00\x00\x02"
        "\x03\x00\x00\x03",

        "\x03\x00\x00\x00"
        "\x01\x00\x01\x00"
        "\x02\x00\x02\x00"
        "\x03\x00\x03\x00"
    );

    CompositeDynamicArray* next = prophy::swap(reinterpret_cast<CompositeDynamicArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 16);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, CompositeFixedArray)
{
    data x(
        "\x01\x00\x00\x02"
        "\x01\x00\x00\x02"
        "\x01\x00\x00\x02",

        "\x01\x00\x02\x00"
        "\x01\x00\x02\x00"
        "\x01\x00\x02\x00"
    );

    CompositeFixedArray* next = prophy::swap(reinterpret_cast<CompositeFixedArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, CompositeGreedyArray)
{
    data x(
        "\x00\x01"
        "\x01\x00\x00\x01"
        "\x02\x00\x00\x02"
        "\x01\x00\x00\x01"
        "\x02\x00\x00\x02",

        "\x01\x00"
        "\x01\x00\x01\x00"
        "\x02\x00\x02\x00"
        "\x01\x00\x01\x00"
        "\x02\x00\x02\x00"
    );

    CompositeGreedyArray* next = prophy::swap(reinterpret_cast<CompositeGreedyArray*>(x.input.data()));
    Composite* past_end = prophy::swap_n_fixed(
        prophy::cast<Composite*>(next), 2);

    EXPECT_EQ(byte_distance(x.input.data(), next), 2);
    EXPECT_EQ(byte_distance(x.input.data(), past_end), 18);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, CompositeLimitedArray)
{
    data x(
        "\x00\x02"
        "\x01\x00\x00\x01"
        "\x02\x00\x00\x02"
        "\xab\xcd\xef\xba",

        "\x02\x00"
        "\x01\x00\x01\x00"
        "\x02\x00\x02\x00"
        "\xab\xcd\xef\xba"
    );

    CompositeLimitedArray* next = prophy::swap(reinterpret_cast<CompositeLimitedArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 14);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ConstantTypedefEnum)
{
    data x(
        "\x00\x01"
        "\x00\x02"
        "\x00\x03"
        "\x00\x04"
        "\x00\x00\x00\x01",

        "\x01\x00"
        "\x02\x00"
        "\x03\x00"
        "\x04\x00"
        "\x01\x00\x00\x00"
    );

    ConstantTypedefEnum* next = prophy::swap(reinterpret_cast<ConstantTypedefEnum*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, DynamicComposite)
{
    data x(
        "\x00\x00\x00\x03"
        "\x00\x01\x00\x02"
        "\x00\x03\xab\xcd",

        "\x03\x00\x00\x00"
        "\x01\x00\x02\x00"
        "\x03\x00\xab\xcd"
    );

    DynamicComposite* next = prophy::swap(reinterpret_cast<DynamicComposite*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, DynamicCompositeComposite)
{
    data x(
        "\x00\x00\x00\x03"
        "\x00\x01\x00\x02"
        "\x00\x03\xab\xcd"
        "\x00\x00\x00\x04"
        "\x00\x00\x00\x01"
        "\x00\x05\xab\xcd"
        "\x00\x00\x00\x02"
        "\x00\x06\x00\x07",

        "\x03\x00\x00\x00"
        "\x01\x00\x02\x00"
        "\x03\x00\xab\xcd"
        "\x04\x00\x00\x00"
        "\x01\x00\x00\x00"
        "\x05\x00\xab\xcd"
        "\x02\x00\x00\x00"
        "\x06\x00\x07\x00"
    );

    DynamicCompositeComposite* next = prophy::swap(reinterpret_cast<DynamicCompositeComposite*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 32);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, DynamicCompositeDynamicArray)
{
    data x(
        "\x00\x02\xab\xcd"
        "\x00\x00\x00\x01"
        "\x00\x01\xef\xab"
        "\x00\x00\x00\x03"
        "\x00\x01\x00\x02"
        "\x00\x03\xab\xcd",

        "\x02\x00\xab\xcd"
        "\x01\x00\x00\x00"
        "\x01\x00\xef\xab"
        "\x03\x00\x00\x00"
        "\x01\x00\x02\x00"
        "\x03\x00\xab\xcd"
    );

    DynamicCompositeDynamicArray* next = prophy::swap(reinterpret_cast<DynamicCompositeDynamicArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 24);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, DynamicCompositeGreedyArray)
{
    data x(
        "\x00\x01\xab\xcd"
        "\x00\x00\x00\x01"
        "\x00\x01\xef\xab"
        "\x00\x00\x00\x03"
        "\x00\x01\x00\x02"
        "\x00\x03\xab\xcd",

        "\x01\x00\xab\xcd"
        "\x01\x00\x00\x00"
        "\x01\x00\xef\xab"
        "\x03\x00\x00\x00"
        "\x01\x00\x02\x00"
        "\x03\x00\xab\xcd"
    );

    DynamicCompositeGreedyArray* next = prophy::swap(reinterpret_cast<DynamicCompositeGreedyArray*>(x.input.data()));
    DynamicComposite* past_end = prophy::swap_n_dynamic(
        prophy::cast<DynamicComposite*>(next), 2);

    EXPECT_EQ(byte_distance(x.input.data(), next), 4);
    EXPECT_EQ(byte_distance(x.input.data(), past_end), 24);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyArrays)
{
    data x(
        "\x00\x00\x00\x05"
        "\x01\x02\x03\x04"
        "\x05\xab"
        "\x00\x02\x00\x01"
        "\x00\x02"
        "\x03\xab\xab\xab\xab\xab\xab\xab"
        "\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x00\x00\x00\x00\x00\x00\x00\x02"
        "\x00\x00\x00\x00\x00\x00\x00\x03",

        "\x05\x00\x00\x00"
        "\x01\x02\x03\x04"
        "\x05\xab"
        "\x02\x00\x01\x00"
        "\x02\x00"
        "\x03\xab\xab\xab\xab\xab\xab\xab"
        "\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x02\x00\x00\x00\x00\x00\x00\x00"
        "\x03\x00\x00\x00\x00\x00\x00\x00"
    );

    ManyArrays* next = prophy::swap(reinterpret_cast<ManyArrays*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 48);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyArraysMixed)
{
    data x(
        "\x00\x00\x00\x05"
        "\x00\x02"
        "\x01\x02\x03\x04"
        "\x05\x00"
        "\x00\x01\x00\x02",

        "\x05\x00\x00\x00"
        "\x02\x00"
        "\x01\x02\x03\x04"
        "\x05\x00"
        "\x01\x00\x02\x00"
    );

    ManyArraysMixed* next = prophy::swap(reinterpret_cast<ManyArraysMixed*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 16);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyArraysMixedHeavily)
{
    data x(
        "\x00\x00\x00\x01"
        "\x00\x00\x00\x03"
        "\x00\x01\x00\x02"
        "\x00\x03\xab\xcd"
        "\x00\x00\x00\x05"
        "\x00\x04\x00\x05"
        "\x00\x06\x00\x07"
        "\x00\x08\xab\xcd"
        "\x00\x09\xab\xcd",

        "\x01\x00\x00\x00"
        "\x03\x00\x00\x00"
        "\x01\x00\x02\x00"
        "\x03\x00\xab\xcd"
        "\x05\x00\x00\x00"
        "\x04\x00\x05\x00"
        "\x06\x00\x07\x00"
        "\x08\x00\xab\xcd"
        "\x09\x00\xab\xcd"
    );

    ManyArraysMixedHeavily* next = prophy::swap(reinterpret_cast<ManyArraysMixedHeavily*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 36);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyArraysPadding)
{
    data x(
        "\x01\x00\x00\x00"
        "\x00\x00\x00\x00"
        "\x02\x02\x03\x00"
        "\x00\x00\x00\x02"
        "\x04\x05\x00\x00"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x06",

        "\x01\x00\x00\x00"
        "\x00\x00\x00\x00"
        "\x02\x02\x03\x00"
        "\x02\x00\x00\x00"
        "\x04\x05\x00\x00"
        "\x00\x00\x00\x00"
        "\x06\x00\x00\x00"
        "\x00\x00\x00\x00"
    );

    ManyArraysPadding* next = prophy::swap(reinterpret_cast<ManyArraysPadding*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 32);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyArraysTailFixed)
{
    data x(
        "\x02\x02\x03\x00"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x04"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x05",

        "\x02\x02\x03\x00"
        "\x00\x00\x00\x00"
        "\x04\x00\x00\x00"
        "\x00\x00\x00\x00"
        "\x05\x00\x00\x00"
        "\x00\x00\x00\x00"
    );

    ManyArraysTailFixed* next = prophy::swap(reinterpret_cast<ManyArraysTailFixed*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 24);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ManyDynamic)
{
    data x(
        "\x00\x00\x00\x01"
        "\x00\x01\xab\xcd"
        "\x00\x00\x00\x02"
        "\x00\x02\x00\x03"
        "\x00\x00\x00\x03"
        "\x00\x04\x00\x05"
        "\x00\x06\xab\xcd",

        "\x01\x00\x00\x00"
        "\x01\x00\xab\xcd"
        "\x02\x00\x00\x00"
        "\x02\x00\x03\x00"
        "\x03\x00\x00\x00"
        "\x04\x00\x05\x00"
        "\x06\x00\xab\xcd"
    );

    ManyDynamic* next = prophy::swap(reinterpret_cast<ManyDynamic*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 28);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Optional)
{
    data x(
        "\x00\x00\x00\x01"
        "\x00\x00\x00\x01"
        "\x00\x00\x00\x01"
        "\x02\x00\x00\x03"
        "\x04\x00\x00\x05",

        "\x01\x00\x00\x00"
        "\x01\x00\x00\x00"
        "\x01\x00\x00\x00"
        "\x02\x00\x03\x00"
        "\x04\x00\x05\x00"
    );

    Optional* next = prophy::swap(reinterpret_cast<Optional*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 20);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Optional_not_set)
{
    data x(
        "\x00\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\x00\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\xab\xcd\xef\xab",

        "\x00\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\x00\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\xab\xcd\xef\xab"
    );

    Optional* next = prophy::swap(reinterpret_cast<Optional*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 20);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Scalar)
{
    data x(
        "\x01\x00\x00\x02",

        "\x01\x00\x02\x00"
    );

    Scalar* next = prophy::swap(reinterpret_cast<Scalar*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 4);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ScalarDynamicArray)
{
    data x(
        "\x00\x00\x00\x03"
        "\x00\x05\x00\x06"
        "\x00\x07\xab\xcd",

        "\x03\x00\x00\x00"
        "\x05\x00\x06\x00"
        "\x07\x00\xab\xcd"
    );

    ScalarDynamicArray* next = prophy::swap(reinterpret_cast<ScalarDynamicArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ScalarFixedArray)
{
    data x(
        "\x00\x02"
        "\x00\x02"
        "\x00\x02",

        "\x02\x00"
        "\x02\x00"
        "\x02\x00"
    );

    ScalarFixedArray* next = prophy::swap(reinterpret_cast<ScalarFixedArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 6);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ScalarGreedyArray)
{
    data x(
        "\x00\x08\xab\xcd"
        "\x00\x00\x00\x01"
        "\x00\x00\x00\x02",

        "\x08\x00\xab\xcd"
        "\x01\x00\x00\x00"
        "\x02\x00\x00\x00"
    );

    ScalarGreedyArray* next = prophy::swap(reinterpret_cast<ScalarGreedyArray*>(x.input.data()));
    uint32_t* past_end = prophy::swap_n_fixed(
        prophy::cast<uint32_t*>(next), 2);

    EXPECT_EQ(byte_distance(x.input.data(), next), 4);
    EXPECT_EQ(byte_distance(x.input.data(), past_end), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, ScalarLimitedArray)
{
    data x(
        "\x00\x00\x00\x02"
        "\x00\x05\x00\x06"
        "\xab\xcd\xef\xba",

        "\x02\x00\x00\x00"
        "\x05\x00\x06\x00"
        "\xab\xcd\xef\xba"
    );

    ScalarLimitedArray* next = prophy::swap(reinterpret_cast<ScalarLimitedArray*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 12);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Union_a)
{
    data x(
        "\x00\x00\x00\x01"
        "\xab\xcd\xef\xab"
        "\x01\x00\x00\x00"
        "\x00\x00\x00\x00",

        "\x01\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\x01\x00\x00\x00"
        "\x00\x00\x00\x00"
    );

    Union* next = prophy::swap(reinterpret_cast<Union*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 16);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Union_b)
{
    data x(
        "\x00\x00\x00\x02"
        "\xab\xcd\xef\xab"
        "\x00\x00\x00\x00"
        "\x00\x00\x00\x01",

        "\x02\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\x01\x00\x00\x00"
        "\x00\x00\x00\x00"
    );

    Union* next = prophy::swap(reinterpret_cast<Union*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 16);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}

TEST(generated, Union_c)
{
    data x(
        "\x00\x00\x00\x03"
        "\xab\xcd\xef\xab"
        "\x01\x00\x00\x02"
        "\x03\x00\x00\x04",

        "\x03\x00\x00\x00"
        "\xab\xcd\xef\xab"
        "\x01\x00\x02\x00"
        "\x03\x00\x04\x00"
    );

    Union* next = prophy::swap(reinterpret_cast<Union*>(x.input.data()));

    EXPECT_EQ(byte_distance(x.input.data(), next), 16);
    EXPECT_THAT(x.input, ContainerEq(x.expected));
}