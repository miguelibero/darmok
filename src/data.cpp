#include <darmok/data.hpp>
#include <darmok/string.hpp>
#include <bx/allocator.h>
#include <bx/file.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

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

    const void* DataView::end() const noexcept
    {
        return ((uint8_t*)_ptr) + _size;
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

    std::string DataView::toString() const noexcept
    {
        return toHex();
    }

    std::string DataView::toHex(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        std::stringstream ss;
        for (size_t i = 0; i < size; i++)
        {
            auto& v = ((uint8_t*)ptr)[i];
            ss << StringUtils::binToHex(v);
        }
        return ss.str();
    }

    std::string DataView::toHeader(std::string_view varName, size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);

        std::string fixVarName(varName);
        std::replace(fixVarName.begin(), fixVarName.end(), '.', '_');
        std::stringstream ss;

        ss << "static const uint8_t " << fixVarName << "[" << size << "] = " << std::endl;
        ss << "{" << std::endl;
        size_t i = 0;
        for (; i < size; i++)
        {
            if (i % 16 == 0)
            {
                ss << "  ";
            }
            auto& v = ((uint8_t*)ptr)[i];
            ss << "0x" << darmok::StringUtils::binToHex(v);
            if (i < size - 1)
            {
                ss << ", ";
            }
            if (i % 16 == 15)
            {
                ss << std::endl;
            }
        }
        if (i % 16 != 0)
        {
            ss << std::endl;
        }
        ss << "};" << std::endl;
        return ss.str();
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
        if (size == 0)
        {
            return nullptr;
        }
        return bgfx::makeRef(ptr, uint32_t(size));
    }

    const bgfx::Memory* DataView::copyMem(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        if (size == 0)
        {
            return nullptr;
        }
        return bgfx::copy(ptr, uint32_t(size));
    }

    size_t DataView::fixOffset(size_t offset, void*& ptr) const noexcept
    {
        if (offset >= _size)
        {
            offset = _size - 1;
        }
        ptr = ((uint8_t*)_ptr) + offset;
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

    void* Data::malloc(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept
    {
        if (size == 0)
        {
            return nullptr;
        }
        return alloc == nullptr ? std::malloc(size) : bx::alloc(alloc.ptr(), size);
    }

    Data::Data(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept
        : _ptr(malloc(size, alloc.ptr()))
        , _size(size)
        , _alloc(alloc)
    {
    }

    Data::Data(const OptionalRef<bx::AllocatorI>& alloc) noexcept
        : Data(0, alloc)
    {
    }

    Data::Data(const void* ptr, size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept
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

    void* Data::end() const noexcept
    {
        return ((uint8_t*)_ptr) + _size;
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
            bx::free(_alloc.ptr(), _ptr);
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
            ptr = bx::realloc(_alloc.ptr(), _ptr, size);
        }
        if (ptr != nullptr)
        {
            _size = size;
            _ptr = ptr;
        }
    }

    void Data::fill(const DataView& data) noexcept
    {
        if (data.empty())
        {
            return;
        }
        auto ptr = static_cast<uint8_t*>(_ptr);
        auto endPtr = static_cast<uint8_t*>(end());
        for (; ptr < endPtr; ptr += data.size())
        {
            auto ptrSize = data.size();
            if (ptr + ptrSize > endPtr)
            {
                ptrSize = endPtr - ptr;
            }
            std::memcpy(ptr, data.ptr(), ptrSize);
        }
    }

    std::string Data::toString() const noexcept
    {
        return view().toString();
    }

    Data Data::fromHex(std::string_view hex)
    {
        static const size_t elmLen = 2;
        Data data(hex.size() / elmLen);
        auto ptr = (uint8_t*)data.ptr();
        for (size_t i = 0; i < data.size(); i++)
        {
            auto elm = hex.substr(i*elmLen, elmLen);
            ptr[i] = StringUtils::hexToBin(elm);
        }
        return data;
    }

    Data Data::fromFile(const std::string& path)
    {
        FILE* fh;
        auto err = fopen_s(&fh, path.c_str(), "rb");
        if (err)
        {
            throw std::runtime_error("failed to open file");
        }
        fseek(fh, 0, SEEK_END);
        long size = ftell(fh);
        fseek(fh, 0, SEEK_SET);
        Data data(size);
        fread(data.ptr(), size, 1, fh);
        fclose(fh);
        return data;
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

    Data::Data(const DataView& other, const OptionalRef<bx::AllocatorI>& alloc) noexcept
        : Data(other.ptr(), other.size(), alloc)
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

    Data::Data(std::string_view str, const OptionalRef<bx::AllocatorI>& alloc) noexcept
        : Data(str.size() + 1, alloc)
    {
        *this = str;
    }

    Data& Data::operator=(std::string_view str) noexcept
    {
        auto strSize = str.size() + 1;
        if (_size < strSize)
        {
            resize(strSize);
        }
        bx::memCopy(_ptr, str.data(), str.size());
        static_cast<std::string_view::pointer>(_ptr)[str.size()] = 0;
        return *this;
    }

    Data& Data::operator=(const char* str) noexcept
    {
        return operator=(std::string_view(str));
    }

    FileDataLoader::FileDataLoader(bx::FileReaderI& fileReader, const OptionalRef<bx::AllocatorI>& alloc)
        : _fileReader(fileReader)
        , _allocator(alloc)
    {
    }

    Data FileDataLoader::operator()(std::string_view filePath)
    {
        bx::StringView bxFilePath(filePath.data(), int32_t(filePath.size()));
        if (!bx::open(&_fileReader, bxFilePath))
        {
            throw std::runtime_error("failed to load data from file.");
        }
        try
        {
            auto size = bx::getSize(&_fileReader);
            auto data = Data(size, _allocator);
            bx::read(&_fileReader, data.ptr(), (int32_t)size, bx::ErrorAssert{});
            bx::close(&_fileReader);
            return data;
        }
        catch(...)
        {
            bx::close(&_fileReader);
            throw;
        }
    }
}

std::ostream& operator<<(std::ostream& out, const darmok::DataView& data)
{
    out.write((const char*)data.ptr(), data.size());
    return out;
}