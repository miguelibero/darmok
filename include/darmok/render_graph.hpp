#pragma once
#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <bx/bx.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <entt/entt.hpp>

namespace darmok
{
    class App;

    struct RenderResourceDefinition final
    {
        std::string name;
        entt::id_type type;

        template<typename T>
        static RenderResourceDefinition create(std::string_view name) noexcept
        {
            return { std::string(name), entt::type_hash<T>::value() };
        }

        entt::id_type hash() const noexcept;
        bool operator==(const RenderResourceDefinition& other) const noexcept;
        bool operator!=(const RenderResourceDefinition& other) const noexcept;
        bool operator<(const RenderResourceDefinition& other) const noexcept;
    };

    class DARMOK_EXPORT RenderResourceGroupDefinition
    {
    public:
        static const std::string defaultName;
        using Resource = RenderResourceDefinition;
        using ConstIterator = std::vector<Resource>::const_iterator;
        ConstIterator begin() const;
        ConstIterator end() const;
        bool contains(const Resource& res) const noexcept;


        template<typename T>
        RenderResourceGroupDefinition& add(std::string_view name = defaultName)
        {
            _resources.push_back(Resource::create<T>(name));
            return *this;
        }

        template<typename T>
        RenderResourceGroupDefinition& add(std::string_view name, const T& value)
        {
            return add<T>(name);
        }

        template<typename T>
        RenderResourceGroupDefinition& add(std::string_view name, const OptionalRef<const T>& value)
        {
            return add<T>(name);
        }

        template<typename T>
        RenderResourceGroupDefinition& add(std::string_view name, const OptionalRef<T>& value)
        {
            return add<T>(name);
        }

        template<typename T>
        RenderResourceGroupDefinition& add(const T& value)
        {
            return add<T>(defaultName);
        }

        template<typename T>
        RenderResourceGroupDefinition& add(const OptionalRef<const T>& value)
        {
            return add<T>(defaultName);
        }

        template<typename T>
        RenderResourceGroupDefinition& add(const OptionalRef<T>& value)
        {
            return add<T>(defaultName);
        }

    private:
        std::vector<Resource> _resources;
    };

    class DARMOK_EXPORT RenderGraphResources final
    {
    public:
        static const std::string defaultName;

        template<typename T>
        RenderGraphResources& set(std::string_view name, T&& value) noexcept
        {
            createKey<T>(name).push_back(std::forward(value));
            return *this;
        }

        template<typename T>
        RenderGraphResources& set(T&& value) noexcept
        {
            return set(defaultName, std::forward(value));
        }

        template<typename T, typename... A>
        RenderGraphResources& emplace(std::string_view name, A&&... args) noexcept
        {
            createKey<T>(name).push_back(T(std::forward<A>(args)...));
            return *this;
        }

        template<typename T, typename... A>
        RenderGraphResources& emplaceDef(A&&... args) noexcept
        {
            return emplace<T>(defaultName, std::forward<A>(args)...);
        }

        template<typename T>
        RenderGraphResources& setRef(std::string_view name, const T& value) noexcept
        {
            createKey<T>(name).push_back(std::reference_wrapper<const T>(value));
            return *this;
        }

        template<typename T>
        RenderGraphResources& setRef(const T& value) noexcept
        {
            return setRef(defaultName, value);
        }

        template<typename Iter>
        RenderGraphResources& setCollection(Iter begin, Iter end) noexcept
        {
            return setCollection(defaultName, begin, end);
        }

        template<typename Iter>
        RenderGraphResources& setCollection(std::string_view name, Iter begin, Iter end) noexcept
        {
            using T = Iter::value_type;
            auto& values = createKey<T>(name);
            for (auto itr = begin; itr != end; ++itr)
            {
                values.push_back(*itr);
            }
            return *this;
        }

        template<typename Iter>
        RenderGraphResources& setRefCollection(Iter begin, Iter end) noexcept
        {
            return setRefCollection(defaultName, begin, end);
        }

        template<typename Iter>
        RenderGraphResources& setRefCollection(std::string_view name, Iter begin, Iter end) noexcept
        {
            using T = Iter::value_type;
            auto& values = createKey<T>(name);
            for (auto itr = begin; itr != end; ++itr)
            {
                values.push_back(std::reference_wrapper<const T>(*itr));
            }
            return *this;
        }

        template<typename T>
        bool get(std::string_view name, T& val, size_t pos = -1) const noexcept
        {
            if (auto ref = get<T>(name, pos))
            {
                val = ref.value();
                return true;
            }
            return false;
        }

        template<typename T>
        bool get(T& val, size_t pos = 0) const noexcept
        {
            return get(defaultName, val, pos);
        }

        template<typename T>
        void get(std::string_view name, OptionalRef<const T>& ref, size_t pos = std::string::npos) const noexcept
        {
            ref = get<T>(name, pos);
        }

        template<typename T>
        void get(OptionalRef<const T>& ref, size_t pos = 0) const noexcept
        {
            ref = get<T>(defaultName, pos);
        }

        template<typename T>
        size_t get(std::vector<T>& results) const noexcept
        {
            return get(defaultName, results);
        }
        
        template<typename T>
        size_t get(std::string_view name, std::vector<T>& results) const noexcept
        {
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return 0;
            }
            size_t count = 0;
            for (auto& val : itr->second)
            {
                if (auto ref = castAny(val))
                {
                    results.emplace_back(ref.value());
                    count++;
                }
            }
            return count;
        }

        template<typename T>
        size_t get(std::vector<OptionalRef<const T>>& results) const noexcept
        {
            return get(defaultName, results);
        }

        template<typename T>
        size_t get(std::string_view name, std::vector<OptionalRef<const T>>& results) const noexcept
        {
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return 0;
            }
            for (auto& val : itr->second)
            {
                results.push_back(castAny(val));
            }
            return itr->second.size();
        }

        template<typename T>
        OptionalRef<const T> get(std::string_view name = defaultName, size_t pos = std::string::npos) const noexcept
        {
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return nullptr;
            }
            auto& values = itr->second;
            if (pos == std::string::npos)
            {
                pos = values.size() - 1;
            }
            if (pos < 0 || pos >= values.size())
            {
                return nullptr;
            }
            return castAny<T>(values[pos]);
        }

        template<typename T>
        size_t size(std::string_view name = defaultName) const noexcept
        {
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return 0;
            }
            return itr->second.size();
        }

    private:
        std::unordered_map<entt::id_type, std::vector<entt::any>> _data;

        template<typename T>
        std::vector<entt::any>& createKey(std::string_view name) noexcept
        {
            auto key = getKey<T>(name);
            auto itr = _data.find(key);
            if (itr != _data.end())
            {
                itr->second.clear();
            }
            else
            {
                itr = _data.emplace(key, std::vector<entt::any>{}).first;
            }
            return itr->second;
        }

        template<typename T>
        static entt::id_type getKey(std::string_view name = defaultName) noexcept
        {
            return RenderResourceDefinition::create<T>(name).hash();
        }

        template<typename T>
        static OptionalRef<const T> castAny(const entt::any& value) noexcept
        {
            auto ptr = &value;
            if (auto val = entt::any_cast<T>(ptr))
            {
                return val;
            }
            if (auto ref = entt::any_cast<std::reference_wrapper<T>>(ptr))
            {
                return ref->get();
            }
            if (auto constRef = entt::any_cast<std::reference_wrapper<const T>>(ptr))
            {
                return constRef->get();
            }
            return nullptr;
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderPassDelegate
    {
    public:
        virtual ~IRenderPassDelegate() = default;
        virtual void renderPassExecute(RenderGraphResources& res) = 0;
    };


    class DARMOK_EXPORT RenderPassDefinition final
    {
    public:
        using Resources = RenderResourceGroupDefinition;

        RenderPassDefinition(const std::string& name) noexcept;
        const std::string& getName() const noexcept;

        RenderPassDefinition& setDelegate(IRenderPassDelegate& dlg) noexcept;
        bool operator()(RenderGraphResources& res) const;

        Resources& getInputs() noexcept;
        const Resources& getInputs() const noexcept;
        Resources& getOutputs() noexcept;
        const Resources& getOutputs() const noexcept;
        bool getSync() const noexcept;

        entt::id_type hash() const noexcept;
    private:
        std::string _name;
        Resources _inputs;
        Resources _outputs;
        OptionalRef<IRenderPassDelegate> _delegate;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderPass : public IRenderPassDelegate
    {
    public:
        virtual ~IRenderPass() = default;
        virtual const std::string& getRenderPassName() const noexcept = 0;
        virtual void renderPassDefine(RenderPassDefinition& def) = 0;
    };

    class RenderGraph;

    class DARMOK_EXPORT RenderGraphDefinition final
    {
    public:
        RenderPassDefinition& addPass(const std::string& name);
        const RenderPassDefinition& addPass(IRenderPass& pass);
        const RenderPassDefinition& getPass(size_t vertex) const;
        RenderGraph compile() const;
    private:
        std::vector<RenderPassDefinition> _passes;
    };

    class DARMOK_EXPORT RenderGraph final
    {
    public:
        using Matrix = entt::adjacency_matrix<entt::directed_tag>;
        using Resources = RenderGraphResources;
        using Definition = RenderGraphDefinition;

        RenderGraph(const Matrix& matrix, const Definition& def) noexcept;

        Resources execute() const;
        void execute(Resources& res) const;
        void writeGraphviz(std::ostream& out) const;

        static std::string getMatrixDebugInfo(const Matrix& mtx) noexcept;

    private:
        Matrix _matrix;
        Definition _def;

        bool execute(Resources& res, std::unordered_set<size_t>& executed) const;
    };

}