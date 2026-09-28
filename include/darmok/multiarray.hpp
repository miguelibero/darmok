#pragma once

#include <darmok/glm.hpp>
#include <darmok/data.hpp>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <vector>

    namespace darmok
{

    template <typename T, size_t N, typename L = glm::uint>
    class MultiArrayView
    {
        static_assert(N > 0);

      public:
        using value_type = T;
        using length_type = L;
        static constexpr size_t dimension_num = N;
        using size_type = glm::vec<dimension_num, length_type>;

        constexpr MultiArrayView() noexcept = default;

        constexpr MultiArrayView(
            value_type* data,
            const size_type& size,
            const size_type& strides) noexcept
            : _data(data),
              _size(size),
              _strides(strides)
        {
        }

        [[nodiscard]]
        constexpr length_type length() const noexcept
        {
            return _size[0] * _strides[0];
        }

        [[nodiscard]]
        constexpr length_type extent(size_t dimension) const noexcept
        {
            assert(dimension < dimension_num);
            return _size[dimension];
        }

        [[nodiscard]]
        constexpr const size_type& size() const noexcept
        {
            return _size;
        }

        [[nodiscard]]
        constexpr const size_type& strides() const noexcept
        {
            return _strides;
        }

        [[nodiscard]]
        constexpr bool empty() const noexcept
        {
            return _data == nullptr || size() == 0;
        }

        [[nodiscard]]
        constexpr value_type* data() noexcept
        {
            return _data;
        }

        [[nodiscard]]
        constexpr const value_type* data() const noexcept
        {
            return _data;
        }

        [[nodiscard]]
        DataView dataView() const noexcept
        {
            return DataView{
                reinterpret_cast<const void*>(_data),
                length() * sizeof(value_type)};
        }

        template <size_t M = dimension_num>
            requires(M > 1)
        [[nodiscard]]
        constexpr auto operator[](length_type index) noexcept
        {
            assert(index < _size[0]);

            constexpr size_t SubN = dimension_num - 1;

            glm::vec<SubN, length_type> dimensions;
            glm::vec<SubN, length_type> strides;

            for(size_t i = 0; i < SubN; ++i)
            {
                dimensions[i] = _size[i + 1];
                strides[i] = _strides[i + 1];
            }

            return MultiArrayView<value_type, SubN, length_type>{
                _data + index * _strides[0],
                dimensions,
                strides};
        }

        template <size_t M = dimension_num>
            requires(M > 1)
        [[nodiscard]]
        constexpr auto operator[](length_type index) const noexcept
        {
            assert(index < _size[0]);

            constexpr size_t SubN = dimension_num - 1;

            glm::vec<SubN, length_type> dimensions;
            glm::vec<SubN, length_type> strides;

            for(size_t i = 0; i < SubN; ++i)
            {
                dimensions[i] = _size[i + 1];
                strides[i] = _strides[i + 1];
            }

            return MultiArrayView<const value_type, SubN, length_type>{
                _data + index * _strides[0],
                dimensions,
                strides};
        }

        template <size_t M = dimension_num>
            requires(M == 1)
        [[nodiscard]]
        constexpr value_type& operator[](length_type index) noexcept
        {
            assert(index < _size[0]);
            return _data[index * _strides[0]];
        }

        template <size_t M = dimension_num>
            requires(M == 1)
        [[nodiscard]]
        constexpr const value_type& operator[](length_type index) const noexcept
        {
            assert(index < _size[0]);
            return _data[index * _strides[0]];
        }

      private:
        value_type* _data = nullptr;
        size_type _size{};
        size_type _strides{};
    };

    template <typename T, size_t N, typename L = glm::uint>
    class MultiArray
    {
        static_assert(N > 0);

      public:
        using value_type = T;
        using length_type = L;
        static constexpr size_t dimension_num = N;
        using size_type = glm::vec<dimension_num, length_type>;

        using storage_type = std::vector<value_type>;
        using iterator = typename storage_type::iterator;
        using const_iterator = typename storage_type::const_iterator;

        constexpr MultiArray() noexcept = default;

        explicit MultiArray(const size_type& size)
            : _size(size),
              _strides(makeStrides(size)),
              _data(totalSize(size))
        {
        }

        template <typename... Dims>
            requires(
                sizeof...(Dims) == dimension_num &&
                (std::convertible_to<Dims, length_type> && ...))
        explicit MultiArray(Dims... dimensions)
            : MultiArray(size_type{
                  static_cast<length_type>(dimensions)...})
        {
        }

        explicit MultiArray(length_type size) : MultiArray(size_type(size))
        {
        }

        explicit MultiArray(DataView dataView, const size_type& size)
            : MultiArray(size)
        {
            auto ms = std::min(memsize(), dataView.size());
            std::memcpy(data(), dataView.ptr(), ms);
        }

        static expected<MultiArray<value_type, dimension_num, length_type>, std::string> load(DataView data, const size_type& size)
        {
            auto elmsize = sizeof(T);
            if(data.size() != size.x * size.y * elmsize)
            {
                return unexpected{"data size does not match expected size"};
            }
            return MultiArray<value_type, dimension_num, length_type>{data, size};
        }

        [[nodiscard]]
        constexpr const size_type& size() const noexcept
        {
            return _size;
        }

        [[nodiscard]]
        constexpr const size_type& strides() const noexcept
        {
            return _strides;
        }

        [[nodiscard]]
        constexpr length_type extent(size_t dimension) const noexcept
        {
            assert(dimension < dimension_num);
            return _size[dimension];
        }

        [[nodiscard]]
        constexpr length_type length() const noexcept
        {
            return static_cast<length_type>(_data.size());
        }

        [[nodiscard]]
        constexpr size_t memsize() const noexcept
        {
            return _data.size() * sizeof(value_type);
        }

        [[nodiscard]]
        constexpr bool empty() const noexcept
        {
            return _data.empty();
        }

        [[nodiscard]]
        value_type* data() noexcept
        {
            return _data.data();
        }

        [[nodiscard]]
        const value_type* data() const noexcept
        {
            return _data.data();
        }

        [[nodiscard]]
        value_type& at(length_type index)
        {
            return _data.at(index);
        }

        [[nodiscard]]
        const value_type& at(length_type index) const
        {
            return _data.at(index);
        }

        template <size_t M = dimension_num>
            requires(M > 1)
        [[nodiscard]]
        constexpr auto operator[](length_type index) noexcept
        {
            assert(index < _size[0]);

            constexpr size_t SubN = dimension_num - 1;

            glm::vec<SubN, length_type> dimensions;
            glm::vec<SubN, length_type> strides;

            for(size_t i = 0; i < SubN; ++i)
            {
                dimensions[i] = _size[i + 1];
                strides[i] = _strides[i + 1];
            }

            return MultiArrayView<value_type, SubN, length_type>{
                _data.data() + index * _strides[0],
                dimensions,
                strides};
        }

        template <size_t M = dimension_num>
            requires(M > 1)
        [[nodiscard]]
        constexpr auto operator[](length_type index) const noexcept
        {
            assert(index < _size[0]);

            constexpr size_t SubN = dimension_num - 1;

            glm::vec<SubN, length_type> dimensions;
            glm::vec<SubN, length_type> strides;

            for(size_t i = 0; i < SubN; ++i)
            {
                dimensions[i] = _size[i + 1];
                strides[i] = _strides[i + 1];
            }

            return MultiArrayView<const value_type, SubN, length_type>{
                _data.data() + index * _strides[0],
                dimensions,
                strides};
        }

        template <size_t M = dimension_num>
            requires(M == 1)
        [[nodiscard]]
        constexpr value_type& operator[](length_type index) noexcept
        {
            assert(index < _size[0]);
            return _data[index];
        }

        template <size_t M = dimension_num>
            requires(M == 1)
        [[nodiscard]]
        constexpr const value_type& operator[](length_type index) const noexcept
        {
            assert(index < _size[0]);
            return _data[index];
        }

        template <typename... Indices>
            requires(
                sizeof...(Indices) == dimension_num &&
                (std::convertible_to<Indices, length_type> && ...))
        [[nodiscard]]
        value_type& operator()(Indices... indices)
        {
            return _data[index(
                static_cast<length_type>(indices)...)];
        }

        template <typename... Indices>
            requires(
                sizeof...(Indices) == dimension_num &&
                (std::convertible_to<Indices, length_type> && ...))
        [[nodiscard]]
        const value_type& operator()(Indices... indices) const
        {
            return _data[index(
                static_cast<length_type>(indices)...)];
        }

        [[nodiscard]]
        iterator begin() noexcept
        {
            return _data.begin();
        }

        [[nodiscard]]
        iterator end() noexcept
        {
            return _data.end();
        }

        [[nodiscard]]
        const_iterator begin() const noexcept
        {
            return _data.begin();
        }

        [[nodiscard]]
        const_iterator end() const noexcept
        {
            return _data.end();
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept
        {
            return _data.cbegin();
        }

        [[nodiscard]]
        const_iterator cend() const noexcept
        {
            return _data.cend();
        }

        void fill(const value_type& value)
        {
            std::fill(_data.begin(), _data.end(), value);
        }

        [[nodiscard]]
        auto view() noexcept
        {
            return MultiArrayView<value_type, dimension_num, length_type>{
                _data.data(),
                _size,
                _strides};
        }

        [[nodiscard]]
        auto view() const noexcept
        {
            return MultiArrayView<const value_type, dimension_num, length_type>{
                _data.data(),
                _size,
                _strides};
        }

        [[nodiscard]]
        DataView dataView() const noexcept
        {
            return DataView{
                reinterpret_cast<const void*>(_data.data()),
                _data.size() * sizeof(value_type)};
        }

      private:

        [[nodiscard]]
        static constexpr size_type
        makeStrides(const size_type& dimensions) noexcept
        {
            size_type strides{};

            strides[dimension_num - 1] = 1;

            for(size_t i = dimension_num - 1; i > 0; --i)
            {
                strides[i - 1] = strides[i] * dimensions[i];
            }

            return strides;
        }

        [[nodiscard]]
        static constexpr length_type
        totalSize(const size_type& dimensions) noexcept
        {
            length_type result = 1;

            for (size_t i = 0; i < dimension_num; ++i)
            {
                result *= dimensions[i];
            }

            return result;
        }

        template <typename... Indices>
            requires(sizeof...(Indices) == dimension_num)
        [[nodiscard]]
        constexpr length_type index(Indices... indices) const noexcept
        {
            const size_type values{
                static_cast<length_type>(indices)...};

            length_type result = 0;

            for (size_t i = 0; i < dimension_num; ++i)
            {
                result += values[i] * _strides[i];
            }

            return result;
        }

        size_type _size{};
        size_type _strides{};
        storage_type _data;
    };

    template <typename T, typename L = glm::uint>
    using Array2d = MultiArray<T, 2, L>;

    template <typename T, typename L = glm::uint>
    using Array3d = MultiArray<T, 3, L>;

    template <typename T, typename L = glm::uint>
    using Array4d = MultiArray<T, 4, L>;

} // namespace darmok
