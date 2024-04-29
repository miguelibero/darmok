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

    std::string_view DataView::stringView(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        return std::string_view((char*)ptr, size);
    }

    DataView DataView::view(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        return DataView(ptr, size);
    }

    const bgfx::Memory* DataView::makeRef(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        return bgfx::makeRef(ptr, size);
    }

    const bgfx::Memory* DataView::copyMem(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        return bgfx::copy(ptr, size);
    }

    size_t DataView::fixOffset(size_t offset, void*& ptr) const noexcept
    {
        if (offset >= _size)
        {
            offset = _size - 1;
        }
        ptr = ((char*)_ptr) + offset;
        return offset;
    }

    size_t DataView::fixSize(size_t size, size_t offset) const noexcept
    {
        if (size == -1)
        {
            size = _size;
        }
        auto max = _size - offset;
        if (size > max)
        {
            size = max;
        }
        return size;
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

    std::string_view Data::stringView(size_t offset, size_t size) const noexcept
    {
        return DataView(_ptr, _size).stringView(offset, size);
    }

    DataView Data::view(size_t offset, size_t size) const noexcept
    {
        return DataView(_ptr, _size).view(offset, size);
    }

    const bgfx::Memory* Data::makeRef(size_t offset, size_t size) const noexcept
    {
        return view().makeRef(offset, size);
    }

    const bgfx::Memory* Data::copyMem(size_t offset, size_t size) const noexcept
    {
        return view().copyMem(offset, size);
    }

    Data::~Data() noexcept
    {
        clear();
    }

    void Data::release() noexcept
    {
        _size = 0;
        _ptr = nullptr;
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

    void Data::resize(size_t size) noexcept
    {
        if (size == _size)
        {
            return;
        }
        if (size == 0)
        {
            clear();
            return;
        }
        void* ptr = nullptr;
        if (empty())
        {
            ptr = malloc(size, _alloc);
        }
        else if (_alloc == nullptr)
        {
            ptr = std::realloc(_ptr, size);
        }
        else
        {
            ptr = bx::realloc(_alloc, _ptr, size);
        }
        if (ptr != nullptr)
        {
            _size = size;
            _ptr = ptr;
        }
    }

    Data::Data(const Data& other) noexcept
        : Data(other._ptr, other._size, other._alloc)
    {
    }

    Data& Data::operator=(const Data& other) noexcept
    {
        _alloc = other._alloc;
        return operator=(other.view());
    }

    Data::Data(const DataView& other, bx::AllocatorI* alloc) noexcept
        : Data(other.ptr(), other.size(), _alloc)
    {
    }

    Data& Data::operator=(const DataView& other) noexcept
    {
        resize(other.size());
        if (_size > 0)
        {
            bx::memCopy(_ptr, other.ptr(), _size);
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

    Data FileDataLoader::operator()(std::string_view filePath)
    {
        if (!bx::open(_fileReader, bx::StringView(filePath.data(), filePath.size())))
        {
            throw std::runtime_error("failed to load data from file.");
        }

        auto size = bx::getSize(_fileReader);
        auto data = Data(size, _allocator);
        bx::read(_fileReader, data.ptr(), (int32_t)size, bx::ErrorAssert{});
        bx::close(_fileReader);
        return data;
    }

    std::vector<std::string> FileDataLoader::find(std::string_view name)
    {
        return _finder(name);
    }
}