#pragma once

#include <bgfx/bgfx.h>
#include <fmt/format.h>

namespace darmok
{
    template<typename T>
    struct DARMOK_EXPORT BaseBgfxHandle
    {
    public:
        BaseBgfxHandle() noexcept
            : _bgfx{ bgfx::kInvalidHandle }
        {
        }

        BaseBgfxHandle(T handle) noexcept
            : _bgfx{ handle }
        {
        }

        virtual ~BaseBgfxHandle() noexcept = default;

        BaseBgfxHandle& operator=(T handle) noexcept
        {
            _bgfx = handle;
            return* this;
        }

        operator T() const noexcept
        {
            return get();
        }

        const T& get() const noexcept
        {
            return _bgfx;
        }

        virtual void reset() noexcept
        {
            _bgfx.idx = bgfx::kInvalidHandle;
        }

        operator bool() const noexcept
        {
            return valid();
        }

        bool valid() const noexcept
        {
            return isValid(_bgfx);
        }

        uint16_t idx() const noexcept
        {
            return _bgfx.idx;
        }

    protected:
        T _bgfx;
    };

    template<typename T, typename BaseHandle = BaseBgfxHandle<T>>
    struct DARMOK_EXPORT BaseBgfxOwnedHandle : public BaseBgfxHandle<T>
    {
    public:
        using BaseBgfxHandle<T>::BaseBgfxHandle;
        using BaseBgfxHandle<T>::_bgfx;

        virtual ~BaseBgfxOwnedHandle() noexcept
        {
            reset();
        }

        BaseBgfxOwnedHandle(const BaseBgfxOwnedHandle& other) = delete;
        BaseBgfxOwnedHandle& operator=(const BaseBgfxOwnedHandle& other) = delete;
        
        BaseBgfxOwnedHandle(BaseBgfxOwnedHandle&& other) noexcept
            : BaseBgfxOwnedHandle{ other._bgfx }
        {
            other._bgfx.idx = bgfx::kInvalidHandle;
        }

        BaseBgfxOwnedHandle& operator=(BaseBgfxOwnedHandle&& other) noexcept
        {
            reset();
            _bgfx = other._bgfx;
            other._bgfx.idx = bgfx::kInvalidHandle;
            return *this;
        }

        void reset() noexcept override
        {
            if (isValid(_bgfx))
            {
                bgfx::destroy(_bgfx);
                _bgfx.idx = bgfx::kInvalidHandle;
            }
        }

        operator BaseHandle() const noexcept
        {
            return { _bgfx };
        }
    };

    template<class T>
    std::string to_string(const BaseBgfxHandle<T>& handle)
    {
        using std::to_string;
        return to_string(handle.idx());
    }

    template<class T>
    std::string to_string(const BaseBgfxOwnedHandle<T>& handle)
    {
        using std::to_string;
        return to_string(handle.idx());
    }
}

namespace std
{
    template<typename T>
    inline ostream& operator<<(ostream& out, const darmok::BaseBgfxHandle<T>& handle)
    {
        return out << darmok::to_string(handle);
    }

    template<typename T>
    inline ostream& operator<<(ostream& out, const darmok::BaseBgfxOwnedHandle<T>& handle)
    {
        return out << darmok::to_string(handle);
    }
}

namespace fmt
{
    template<typename T>
    struct formatter<darmok::BaseBgfxHandle<T>> : public formatter<uint16_t>
    {
        template <typename FormatContext>
        auto format(const darmok::BaseBgfxHandle<T>& handle, FormatContext& ctx) const
        {
            return formatter<uint16_t>::format(handle.idx(), ctx);
        }
    };

    template<typename T>
    struct formatter<darmok::BaseBgfxOwnedHandle<T>> : public formatter<uint16_t>
    {
        template <typename FormatContext>
        auto format(const darmok::BaseBgfxOwnedHandle<T>& handle, FormatContext& ctx) const
        {
            return formatter<uint16_t>::format(handle.idx(), ctx);
        }
    };
}