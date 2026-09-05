#include <catch2/catch_test_macros.hpp>

#include <darmok/multiarray.hpp>

#include <cstdint>
#include <numeric>
#include <type_traits>

    namespace darmok
{

    TEST_CASE("MultiArray default construction", "[multi_array]")
    {
        MultiArray<int, 2> array;

        REQUIRE(array.empty());
        REQUIRE(array.length() == 0);
        REQUIRE(array.data() == nullptr);
        REQUIRE(array.extent(0) == 0);
        REQUIRE(array.extent(1) == 0);
    }

    TEST_CASE("MultiArray construction with dimensions", "[multi_array]")
    {
        MultiArray<int, 2> array(3, 4);

        REQUIRE_FALSE(array.empty());
        REQUIRE(array.length() == 12);
        REQUIRE(array.extent(0) == 3);
        REQUIRE(array.extent(1) == 4);
        REQUIRE(array.size() == glm::uvec2{3, 4});
    }

    TEST_CASE("MultiArray construction from dimensions array", "[multi_array]")
    {
        MultiArray<int, 3> array(
            glm::uvec3{2, 3, 4});

        REQUIRE(array.length() == 24);
        REQUIRE(array.extent(0) == 2);
        REQUIRE(array.extent(1) == 3);
        REQUIRE(array.extent(2) == 4);
    }

    TEST_CASE("MultiArray 1d indexing", "[multi_array]")
    {
        MultiArray<int, 1> array(5);

        for (unsigned i = 0; i < 5; ++i)
        {
            array[i] = static_cast<int>(i * 10);
        }

        REQUIRE(array[0] == 0);
        REQUIRE(array[1] == 10);
        REQUIRE(array[2] == 20);
        REQUIRE(array[3] == 30);
        REQUIRE(array[4] == 40);
    }

    TEST_CASE("MultiArray 2d indexing", "[multi_array]")
    {
        MultiArray<int, 2> array(3, 4);

        for(unsigned x = 0; x < 3; ++x)
        {
            for (unsigned y = 0; y < 4; ++y)
            {
                array[x][y] = static_cast<int>(x * 10 + y);
            }
        }

        REQUIRE(array[0][0] == 0);
        REQUIRE(array[0][3] == 3);
        REQUIRE(array[1][0] == 10);
        REQUIRE(array[1][2] == 12);
        REQUIRE(array[2][3] == 23);
    }

    TEST_CASE("MultiArray 3d indexing", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        for(unsigned x = 0; x < 2; ++x)
        {
            for(unsigned y = 0; y < 3; ++y)
            {
                for (unsigned z = 0; z < 4; ++z)
                {
                    array[x][y][z] =
                        static_cast<int>(x * 100 + y * 10 + z);
                }
            }
        }

        REQUIRE(array[0][0][0] == 0);
        REQUIRE(array[0][1][2] == 12);
        REQUIRE(array[1][0][0] == 100);
        REQUIRE(array[1][2][3] == 123);
    }

    TEST_CASE("MultiArray 4d indexing", "[multi_array]")
    {
        MultiArray<int, 4> array(2, 3, 4, 5);

        array[1][2][3][4] = 1234;

        REQUIRE(array[1][2][3][4] == 1234);
    }

    TEST_CASE("MultiArray operator() indexing", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        array(1, 2, 3) = 123;

        REQUIRE(array(1, 2, 3) == 123);
    }

    TEST_CASE("MultiArray operator[] and operator() access the same data",
              "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        array[1][2][3] = 42;

        REQUIRE(array(1, 2, 3) == 42);

        array(0, 1, 2) = 24;

        REQUIRE(array[0][1][2] == 24);
    }

    TEST_CASE("MultiArray storage is contiguous", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        for (unsigned i = 0; i < array.length(); ++i)
        {
            array.data()[i] = static_cast<int>(i);
        }

        for(unsigned i = 0; i < array.length(); ++i)
        {
            REQUIRE(array.data()[i] == static_cast<int>(i));
        }
    }

    TEST_CASE("MultiArray row-major layout", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        for (unsigned i = 0; i < array.length(); ++i)
        {
            array.data()[i] = static_cast<int>(i);
        }

        REQUIRE(array[0][0][0] == 0);
        REQUIRE(array[0][0][1] == 1);
        REQUIRE(array[0][0][3] == 3);
        REQUIRE(array[0][1][0] == 4);
        REQUIRE(array[0][1][1] == 5);
        REQUIRE(array[0][2][0] == 8);
        REQUIRE(array[1][0][0] == 12);
        REQUIRE(array[1][2][3] == 23);
    }

    TEST_CASE("MultiArray strides", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        const auto& strides = array.strides();

        REQUIRE(strides[0] == 12);
        REQUIRE(strides[1] == 4);
        REQUIRE(strides[2] == 1);
    }

    TEST_CASE("MultiArray dimensions", "[multi_array]")
    {
        MultiArray<int, 4> array(2, 3, 4, 5);

        const auto& size = array.size();

        REQUIRE(size[0] == 2);
        REQUIRE(size[1] == 3);
        REQUIRE(size[2] == 4);
        REQUIRE(size[3] == 5);
    }

    TEST_CASE("MultiArray fill", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        array.fill(42);

        for (const auto value : array)
        {
            REQUIRE(value == 42);
        }
    }

    TEST_CASE("MultiArray iteration", "[multi_array]")
    {
        MultiArray<int, 2> array(2, 3);

        for (unsigned i = 0; i < array.length(); ++i)
        {
            array.data()[i] = static_cast<int>(i);
        }

        int expected = 0;

        for(const auto value : array)
        {
            REQUIRE(value == expected);
            ++expected;
        }

        REQUIRE(expected == 6);
    }

    TEST_CASE("MultiArray mutable iteration", "[multi_array]")
    {
        MultiArray<int, 2> array(2, 3);

        std::iota(array.begin(), array.end(), 1);

        REQUIRE(array[0][0] == 1);
        REQUIRE(array[0][1] == 2);
        REQUIRE(array[0][2] == 3);
        REQUIRE(array[1][0] == 4);
        REQUIRE(array[1][1] == 5);
        REQUIRE(array[1][2] == 6);
    }

    TEST_CASE("MultiArray const access", "[multi_array]")
    {
        const MultiArray<int, 2> array(
            glm::uvec2{2, 3});

        static_assert(
            std::is_same_v<
                decltype(array[0][0]),
                const int&>);

        static_assert(
            std::is_same_v<
                decltype(array(0, 0)),
                const int&>);
    }

    TEST_CASE("MultiArray view", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        array[1][2][3] = 42;

        auto view = array.view();

        REQUIRE(view.extent(0) == 2);
        REQUIRE(view.extent(1) == 3);
        REQUIRE(view.extent(2) == 4);

        REQUIRE(view[1][2][3] == 42);

        view[0][1][2] = 24;

        REQUIRE(array[0][1][2] == 24);
    }

    TEST_CASE("MultiArray const view", "[multi_array]")
    {
        MultiArray<int, 2> array(2, 3);

        array[1][2] = 42;

        const auto& constArray = array;
        auto view = constArray.view();

        REQUIRE(view[1][2] == 42);

        static_assert(
            std::is_same_v<
                decltype(view[0][0]),
                const int&>);
    }

    TEST_CASE("MultiArray view preserves strides", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        array[1][2][3] = 42;

        auto view = array.view();

        REQUIRE(view.strides()[0] == 12);
        REQUIRE(view.strides()[1] == 4);
        REQUIRE(view.strides()[2] == 1);

        REQUIRE(view[1][2][3] == 42);
    }

    TEST_CASE("MultiArray 2d aliases", "[multi_array]")
    {
        Array2d<int> array(3, 4);

        array[1][2] = 42;

        REQUIRE(array.size().x == 3);
        REQUIRE(array.size().y == 4);
        REQUIRE(array[1][2] == 42);
    }

    TEST_CASE("MultiArray 3d aliases", "[multi_array]")
    {
        Array3d<int> array(2, 3, 4);

        array[1][2][3] = 42;

        REQUIRE(array.size().x == 2);
        REQUIRE(array.size().y == 3);
        REQUIRE(array.size().z == 4);
        REQUIRE(array[1][2][3] == 42);
    }

    TEST_CASE("MultiArray zero dimension", "[multi_array]")
    {
        MultiArray<int, 2> array(0, 10);

        REQUIRE(array.empty());
        REQUIRE(array.length() == 0);
    }

    TEST_CASE("MultiArray element count", "[multi_array]")
    {
        MultiArray<uint8_t, 4> array(2, 3, 4, 5);

        REQUIRE(array.length() == 2 * 3 * 4 * 5);
    }

    TEST_CASE("MultiArray views can be sliced", "[multi_array]")
    {
        MultiArray<int, 3> array(2, 3, 4);

        for(unsigned x = 0; x < 2; ++x)
        {
            for(unsigned y = 0; y < 3; ++y)
            {
                for (unsigned z = 0; z < 4; ++z)
                {
                    array[x][y][z] =
                        static_cast<int>(x * 100 + y * 10 + z);
                }
            }
        }

        auto slice = array[1];

        REQUIRE(slice.extent(0) == 3);
        REQUIRE(slice.extent(1) == 4);

        REQUIRE(slice[0][0] == 100);
        REQUIRE(slice[1][2] == 112);
        REQUIRE(slice[2][3] == 123);
    }

} // namespace darmok
