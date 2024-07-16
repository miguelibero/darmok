#pragma once
#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/utils.hpp>

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
        entt::hashed_string name;
        entt::id_type type;

        template<typename T>
        static RenderResourceDefinition create(entt::hashed_string name) noexcept
        {
            return { name, entt::type_hash<T>::value() };
        }

        operator entt::id_type() const noexcept;
        entt::id_type hash() const noexcept;
        bool operator==(const RenderResourceDefinition& other) const noexcept;
        bool operator!=(const RenderResourceDefinition& other) const noexcept;
        bool operator<(const RenderResourceDefinition& other) const noexcept;
    };

    class DARMOK_EXPORT RenderResourcesDefinition
    {
    public:
        using Resource = RenderResourceDefinition;
        using ConstIterator = std::vector<Resource>::const_iterator;
        ConstIterator begin() const;
        ConstIterator end() const;
        bool contains(const Resource& res) const noexcept;

        template<typename T>
        bool contains(entt::hashed_string name = "") const noexcept
        {
            return contains(Resource::create<T>(name));
        }

        template<typename T>
        RenderResourcesDefinition& add(entt::hashed_string name = "")
        {
            _resources.push_back(Resource::create<T>(name));
            return *this;
        }

    private:
        std::vector<Resource> _resources;
    };

    class DARMOK_EXPORT RenderGraphResources final
    {
    public:
        template<typename T>
        struct Key final
        {
            entt::hashed_string name = "";
            entt::id_type group = 0;

            entt::id_type hash() const noexcept
            {
                auto v = idTypeCombine(name.value(), entt::type_hash<T>::value());
                if (group != 0)
                {
                    v = idTypeCombine(v, group);
                }
                return v;
            }
        };

        RenderGraphResources(entt::id_type group = 0) noexcept;
        RenderGraphResources& setGroup(entt::id_type group) noexcept;

        template<typename T>
        RenderGraphResources& set(T&& value, const Key<T>& key = {}) noexcept
        {
            getAny(key) = value;
            return *this;
        }

        template<typename T, typename... A>
        RenderGraphResources& emplace(const Key<T>& key, A&&... args) noexcept
        {
            getAny(key).emplace<T>(std::forward<A>(args)...);
            return *this;
        }

        template<typename T, typename... A>
        RenderGraphResources& emplaceDef(A&&... args) noexcept
        {
            return emplace<T>({}, std::forward<A>(args)...);
        }

        template<typename T>
        RenderGraphResources& setRef(const T& value, const Key<T>& key = {}) noexcept
        {
            getAny(key) = std::reference_wrapper<const T>(value);
            return *this;
        }

        template<typename T>
        bool get(T& val, const Key<T>& key = {}) const noexcept
        {
            if (auto ref = get(key))
            {
                val = ref.value();
                return true;
            }
            return false;
        }

        template<typename T>
        OptionalRef<const T> get(const Key<T>& key = {}) const noexcept
        {
            if (_group != 0 && key.group == 0)
            {
                auto groupKey = key;
                groupKey.group = _group;
                auto itr = _data.find(groupKey.hash());
                if (itr != _data.end())
                {
                    return castAny<T>(itr->second);
                }
            }
            auto itr = _data.find(key.hash());
            if (itr != _data.end())
            {
                return castAny<T>(itr->second);
            }
            return nullptr;
        }        

    private:
        std::unordered_map<entt::id_type, entt::any> _data;
        entt::id_type _group;

        template<typename T>
        entt::any& getAny(const Key<T>& key = {}) noexcept
        {
            if (key.group == 0 && _group != 0)
            {
                auto groupKey = key;
                groupKey.group = _group;
                return _data[groupKey.hash()];
            }
            return _data[key.hash()];
        }

        template<typename T>
        static OptionalRef<const T> castAny(const entt::any& value) noexcept
        {
            auto ptr = &value;
            if (auto val = entt::any_cast<T>(ptr))
            {
                return val;
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
        using Resources = RenderResourcesDefinition;

        RenderPassDefinition(const std::string& name, entt::id_type group = 0) noexcept;
        const std::string& getName() const noexcept;

        RenderPassDefinition& setDelegate(IRenderPassDelegate& dlg) noexcept;
        void operator()(RenderGraphResources& res) const noexcept;

        Resources& getInputs() noexcept;
        const Resources& getInputs() const noexcept;
        Resources& getOutputs() noexcept;
        const Resources& getOutputs() const noexcept;
        bool getSync() const noexcept;

        operator entt::id_type() const noexcept;
        entt::id_type hash() const noexcept;
    private:
        std::string _name;
        Resources _inputs;
        Resources _outputs;
        OptionalRef<IRenderPassDelegate> _delegate;
        entt::id_type _group;
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
        using Pass = RenderPassDefinition;
        RenderGraphDefinition() noexcept;
        void addChild(const RenderGraphDefinition& child) noexcept;
        Pass& addPass(const std::string& name);
        const Pass& addPass(IRenderPass& pass);

        const Pass& getPass(size_t vertex) const;
        size_t size() const noexcept;
        const Pass& operator[](size_t vertex) const;

        using ConstIterator = std::vector<Pass>::const_iterator;
        ConstIterator begin() const;
        ConstIterator end() const;

        RenderGraph compile() const;
        entt::id_type hash() const noexcept;
        operator entt::id_type() const noexcept;
    private:
        std::vector<Pass> _passes;
        std::vector<RenderGraphDefinition> _children;
        entt::id_type _hash;

        void configureFlow(entt::flow& builder) const noexcept;
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