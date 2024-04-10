#include "data.hpp"
#include <bx/allocator.h>

namespace darmok
{
    DataView::DataView(const void* ptr, size_t size) noexcept
        : _ptr(ptr)
        , _size(size)
    {
    }

    DataView::DataView(const char* str) noexcept
        : _ptr(str)
        , _size(std::strlen(str)+1)
    {
    }

    DataView::DataView(const Data& data) noexcept
        : DataView(data.ptr(), data.size())
    {

    }

    DataView::DataView(const bgfx::Memory& mem) noexcept
        : DataView(mem.data, mem.size)
    {
    }

    const void* DataView::ptr() const noexcept
    {
        return _ptr;
    }

    size_t DataView::size() const noexcept
    {
        return _size;
    }

    bool DataView::empty() const noexcept
    {
        return _ptr == nullptr || _size == 0;
    }

    bool DataView::operator==(const DataView& other) const noexcept
    {
        if (_size != other._size)
        {
            return false;
        }
        return std::memcmp(_ptr, other._ptr, _size) == 0;
    }

    bool DataView::operator!=(const DataView& other) const noexcept
    {
        return !(*this == other);
    }

    void* Data::malloc(size_t size, bx::AllocatorI* alloc) noexcept
    {
        if (size == 0)
        {
            return nullptr;
        }
        return alloc == nullptr ? std::malloc(size) : bx::alloc(alloc, size);
    }

    Data::Data(size_t size, bx::AllocatorI* alloc) noexcept
        : _ptr(malloc(size, alloc))
        , _size(size)
        , _alloc(alloc)
    {
    }

    Data::Data(const void* ptr, size_t size, bx::AllocatorI* alloc) noexcept
        : Data(ptr == nullptr ? 0 : size, alloc)
    {
        if (_ptr != nullptr)
        {
            bx::memCopy(_ptr, ptr, size);
        }
    }

    void* Data::ptr() const noexcept
    {
        return _ptr;
    }

    size_t Data::size() const noexcept
    {
        return _size;
    }

    bool Data::empty() const noexcept
    {
        return _ptr == nullptr || _size == 0;
    }

    bool Data::operator==(const Data& other) const noexcept
    {
        if (_size != other._size)
        {
            return false;
        }
        return std::memcmp(_ptr, other._ptr, _size) == 0;
    }

    bool Data::operator!=(const Data& other) const noexcept
    {
        return !(*this == other);
    }

    Data::~Data() noexcept
    {
        clear();
    }

    void Data::clear() noexcept
    {
        _size = 0;
        if (_ptr == nullptr)
        {
            return;
        }
        if (_alloc == nullptr)
        {
            std::free(_ptr);
        }
        else
        {
            bx::free(_alloc, _ptr);
        }
        _ptr = nullptr;
    }

    const bgfx::Memory* Data::makeRef() const noexcept
    {
        if (empty())
        {
            return nullptr;
        }
        return bgfx::makeRef(_ptr, _size);
    }

    Data::Data(const Data& other) noexcept
        : Data(other._ptr, other._size, other._alloc)
    {
    }

    Data& Data::operator=(const Data& other) noexcept
    {
        clear();
        _alloc = other._alloc;
        if (other._ptr && _size > 0)
        {
            _size = other._size;
            _ptr = malloc(_size, _alloc);
            bx::memCopy(_ptr, other._ptr, _size);
        }
        else
        {
            _ptr = nullptr;
            _size = 0;
        }
        return *this;
    }

    Data::Data(Data&& other) noexcept
        : _ptr(other._ptr)
        , _size(other._size)
        , _alloc(other._alloc)
    {
        other._ptr = nullptr;
        other._size = 0;
    }

    Data& Data::operator=(Data&& other) noexcept
    {
        clear();
        _ptr = other._ptr;
        _size = other._size;
        _alloc = other._alloc;
        other._ptr = nullptr;
        other._size = 0;
        return *this;
    }

    Data::operator DataView() const
    {
        return DataView(*this);
    }

    FileDataLoader::FileDataLoader(bx::FileReaderI* fileReader, bx::AllocatorI* alloc)
        : _fileReader(fileReader)
        , _allocator(alloc)
    {
    }

    std::shared_ptr<Data> FileDataLoader::operator()(std::string_view filePath)
    {
        if (!bx::open(_fileReader, bx::StringView(filePath.data(), filePath.size())))
        {
            throw std::runtime_error("failed to load data from file.");
        }

        auto size = bx::getSize(_fileReader);
        auto data = std::make_shared<Data>(size, _allocator);
        bx::read(_fileReader, data->ptr(), (int32_t)size, bx::ErrorAssert{});
        bx::close(_fileReader);
        return data;
    }
}