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
        : _ptr{ ptr }
        , _size{ size }
    {
    }

    DataView::DataView(std::string_view str) noexcept
        : _ptr{ str.data() }
        , _size{ str.size() }
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
        return static_cast<const uint8_t*>(_ptr) + _size;
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

    expected<void, std::string> DataView::write(const std::filesystem::path& path) const noexcept
    {
        FILE* fh;
#ifdef _MSC_VER        
        auto err = fopen_s(&fh, path.string().c_str(), "wb");
#else
        fh = fopen(path.string().c_str(), "wb");
        int err = 0;
        if (fh == nullptr)
        {
            err = errno;
        }
#endif
        if (err)
        {
            std::string error = strerror(err);
            return unexpected<std::string>{ "failed to open file: " + error };
        }
        fwrite(_ptr, sizeof(uint8_t), _size, fh);
        fclose(fh);
        return {};
    }

    std::string DataView::toHex(size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        std::stringstream ss;
        for (size_t i = 0; i < size; i++)
        {
            ss << std::hex << ((uint8_t*)ptr)[i];
        }
        return ss.str();
    }

    std::string DataView::toHeader(std::string_view varName, size_t offset, size_t size) const noexcept
    {
        void* ptr;
        offset = fixOffset(offset, ptr);
        size = fixSize(size, offset);
        auto uptr = static_cast<uint8_t*>(ptr);

        std::string fixVarName(varName);
        std::replace(fixVarName.begin(), fixVarName.end(), '.', '_');
        std::stringstream ss;

        ss << "static const uint8_t " << fixVarName << "[" << size << "] = " << std::endl;
        ss << "{" << std::endl;
        ss << std::hex << std::setfill('0');
        size_t i = 0;
        for (; i < size; ++i)
        {
            if (i % 16 == 0)
            {
                ss << "  ";
            }
            ss << "0x" << std::setw(2) << static_cast<int>(uptr[i]);
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

    void DataView::fill(const DataView& data) noexcept
    {
        if (data.empty())
        {
            return;
        }
        auto ptr = const_cast<uint8_t*>(static_cast<const uint8_t*>(_ptr));
        auto endPtr = static_cast<const uint8_t*>(end());
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

    void* Data::malloc(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept
    {
        if (size == 0)
        {
            return nullptr;
        }
        return alloc == nullptr ? std::malloc(size) : bx::alloc(alloc.ptr(), size);
    }

    Data::Data(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept
        : _ptr{ malloc(size, alloc.ptr()) }
        , _size{ size }
        , _alloc{ alloc }
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
        return DataView{ _ptr, _size }.stringView(offset, size);
    }

    DataView Data::view(size_t offset, size_t size) const noexcept
    {
        return DataView{ _ptr, _size }.view(offset, size);
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
        view().fill(data);
    }

    std::string Data::toString() const noexcept
    {
        return view().toString();
    }

    Data Data::fromHex(std::string_view hex, const OptionalRef<bx::AllocatorI>& alloc)
    {
        Data data{ hex.size() / 2, alloc };
        auto ptr = (uint8_t*)data.ptr();
		size_t i = 0;
        std::istringstream input{ std::string{hex} };
        while (!input.eof())
        {
            char high, low;
            input >> high >> low;
            if (input.fail())
            {
                break;
            }
            std::string byteStr{ high, low };
            ptr[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
            ++i;
        }

        return data;
    }

    expected<Data, std::string> Data::fromFile(const std::filesystem::path& path, const OptionalRef<bx::AllocatorI>& alloc)
    {
        FILE* fh;
#ifdef _MSC_VER        
        auto err = fopen_s(&fh, path.string().c_str(), "rb");
#else
        fh = fopen(path.string().c_str(), "rb");
        int err = 0;
        if (fh == nullptr)
        {
            err = errno;
        }
#endif
        if (err)
        {
            return unexpected{ strerror(err) };
        }
        fseek(fh, 0, SEEK_END);
        long size = ftell(fh);
        fseek(fh, 0, SEEK_SET);
        Data data(size, alloc);
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
        : _ptr{ other._ptr }
        , _size{ other._size }
        , _alloc{ other._alloc }
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

    FileDataLoader::FileDataLoader(const OptionalRef<bx::AllocatorI>& alloc)
        : _alloc{ alloc }
        , _absolutePathsAllowed{ false }
    {
    }

    bool FileDataLoader::setBasePath(const std::filesystem::path& path) noexcept
    {
        if(_basePath == path)
        {
            return false;
		}
		_basePath = path;
        return true;
    }

    bool FileDataLoader::addRootPath(const std::filesystem::path& path) noexcept
    {
        if (_rootPaths.contains(path))
        {
            return false;
        }
        _rootPaths.insert(path);
        return true;
    }
    bool FileDataLoader::removeRootPath(const std::filesystem::path& path) noexcept
    {
        return _rootPaths.erase(path);
    }

    bool FileDataLoader::getAbsolutePathsAllowed() const noexcept
    {
        return _absolutePathsAllowed;
    }

    void FileDataLoader::setAbsolutePathsAllowed(bool allowed) noexcept
    {
        _absolutePathsAllowed = allowed;
    }

    expected<Data, std::string> FileDataLoader::operator()(const std::filesystem::path& path) noexcept
    {
        if (path.is_absolute())
        {
            if (!_absolutePathsAllowed)
            {
                return unexpected{ "absolute paths not allowed"};
            }
            return Data::fromFile(path, _alloc);
        }
        auto fpath = (_basePath / path).relative_path();
        for (auto& rootPath : _rootPaths)
        {
            auto combPath = rootPath / fpath;
            if (std::filesystem::exists(combPath))
            {
                return Data::fromFile(combPath, _alloc);
            }
        }
        auto err = std::string{ "path " } + path.string() + " does not exist";
        return unexpected{ std::move(err) };
    }
}

std::ostream& operator<<(std::ostream& out, const darmok::DataView& data)
{
    out.write(static_cast<const char*>(data.ptr()), data.size());
    return out;
}

std::istream& operator>>(std::istream& in, darmok::Data& data)
{
    in.read(static_cast<char*>(data.ptr()), data.size());
    return in;
}