#pragma once
#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/utils.hpp>

#include <bx/bx.h>
#include <bgfx/bgfx.h>

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

    using RenderGraphId = entt::id_type;

    struct RenderResourceDefinition final
    {
        entt::hashed_string name;
        RenderGraphId type;

        template<typename T>
        static RenderResourceDefinition create(entt::hashed_string name) noexcept
        {
            return { name, entt::type_hash<T>::value() };
        }

        bool operator==(const RenderResourceDefinition& other) const noexcept;
        bool operator!=(const RenderResourceDefinition& other) const noexcept;
        bool operator<(const RenderResourceDefinition& other) const noexcept;

        operator RenderGraphId() const noexcept;
        RenderGraphId id() const noexcept;

    };

    class DARMOK_EXPORT RenderResourcesDefinition
    {
    public:
        using Resource = RenderResourceDefinition;
        using ConstIterator = std::vector<Resource>::const_iterator;

        bool operator==(const RenderResourcesDefinition& other) const noexcept;
        bool operator!=(const RenderResourcesDefinition& other) const noexcept;

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
        static const RenderGraphId defaultGroup;

        template<typename T>
        struct Key final
        {
            entt::hashed_string name = "";
            RenderGraphId group = defaultGroup;

            operator RenderGraphId() const noexcept
            {
                auto v = idTypeCombine(name.value(), entt::type_hash<T>::value());
                if (group != defaultGroup && group != 0)
                {
                    v = idTypeCombine(v, group);
                }
                return v;
            }
        };

        RenderGraphResources(RenderGraphId group = defaultGroup) noexcept;
        RenderGraphResources& setGroup(RenderGraphId group = defaultGroup) noexcept;

        template<typename T>
        RenderGraphResources& set(const T& value, const Key<T>& key = {}) noexcept
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
        RenderGraphResources& setRef(T& value, const Key<T>& key = {}) noexcept
        {
            getAny(key) = std::reference_wrapper<T>(value);
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
            if (key.group == defaultGroup)
            {
                auto groupKey = key;
                groupKey.group = _group;
                auto itr = _data.find(groupKey);
                if (itr != _data.end())
                {
                    return castAny<T>(itr->second);
                }
            }
            auto itr = _data.find(key);
            if (itr != _data.end())
            {
                return castAny<T>(itr->second);
            }
            return nullptr;
        }

        template<typename T>
        OptionalRef<T> get(const Key<T>& key = {}) noexcept
        {
            if (key.group == defaultGroup)
            {
                auto groupKey = key;
                groupKey.group = _group;
                auto itr = _data.find(groupKey);
                if (itr != _data.end())
                {
                    return castAny<T>(itr->second);
                }
            }
            auto itr = _data.find(key);
            if (itr != _data.end())
            {
                return castAny<T>(itr->second);
            }
            return nullptr;
        }

    private:
        std::unordered_map<RenderGraphId, entt::any> _data;
        RenderGraphId _group;

        template<typename T>
        entt::any& getAny(const Key<T>& key = {}) noexcept
        {
            if (key.group == defaultGroup)
            {
                auto groupKey = key;
                groupKey.group = _group;
                return _data[groupKey];
            }
            return _data[key];
        }

        template<typename T>
        static OptionalRef<const T> castAny(const entt::any& value) noexcept
        {
            auto ptr = &value;
            if (auto val = entt::any_cast<T>(ptr))
            {
                return val;
            }
            if (auto constRef = entt::any_cast<std::reference_wrapper<T>>(ptr))
            {
                return constRef->get();
            }
            return nullptr;
        }

        template<typename T>
        static OptionalRef<T> castAny(entt::any& value) noexcept
        {
            auto ptr = &value;
            if (auto val = entt::any_cast<T>(ptr))
            {
                return val;
            }
            if (auto constRef = entt::any_cast<std::reference_wrapper<T>>(ptr))
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
        virtual void renderPassConfigure(bgfx::ViewId viewId) { };
        virtual void renderPassExecute(RenderGraphResources& res) = 0;
    };

    class DARMOK_EXPORT RenderPassDefinition final
    {
    public:
        using Resources = RenderResourcesDefinition;

        RenderPassDefinition(RenderGraphId group = 0) noexcept;

        bool operator==(const RenderPassDefinition& other) const noexcept;
        bool operator!=(const RenderPassDefinition& other) const noexcept;

        RenderGraphId getGroup() const noexcept;
        bgfx::ViewId getViewId() const noexcept;

        RenderPassDefinition& setName(const std::string& name) noexcept;
        const std::string& getName() const noexcept;

        RenderPassDefinition& setDelegate(IRenderPassDelegate& dlg) noexcept;
        OptionalRef<IRenderPassDelegate> getDelegate() const noexcept;
        void operator()(RenderGraphResources& res) const noexcept;
        void configureView(bgfx::ViewId viewId) noexcept;

        Resources& getInputs() noexcept;
        const Resources& getInputs() const noexcept;
        Resources& getOutputs() noexcept;
        const Resources& getOutputs() const noexcept;
        bool getSync() const noexcept;

        operator RenderGraphId() const noexcept;
        RenderGraphId id() const noexcept;
    private:
        std::string _name;
        RenderGraphId _id;
        Resources _inputs;
        Resources _outputs;
        OptionalRef<IRenderPassDelegate> _delegate;
        RenderGraphId _group;
        bgfx::ViewId _viewId;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderPass : public IRenderPassDelegate
    {
    public:
        virtual ~IRenderPass() = default;
        virtual void renderPassDefine(RenderPassDefinition& def) {};
    };

    class RenderGraph;

    class DARMOK_EXPORT RenderGraphDefinition final
    {
    public:
        using Pass = RenderPassDefinition;
        RenderGraphDefinition() noexcept;
        bool operator==(const RenderGraphDefinition& other) const noexcept;
        bool operator!=(const RenderGraphDefinition& other) const noexcept;

        bool setChild(const RenderGraphDefinition& child) noexcept;

        Pass& addFrontPass() noexcept;
        const Pass& addFrontPass(IRenderPass& pass);
        
        Pass& addPass() noexcept;
        const Pass& addPass(IRenderPass& pass);

        bool hasPass(const std::string& name) const noexcept;
        bool removePass(IRenderPassDelegate& dlg);

        void clear();
        size_t size() const noexcept;

        const Pass& operator[](size_t vertex) const;
        Pass& operator[](size_t vertex);

        RenderGraph compile();

        RenderGraphId id() const noexcept;
        operator RenderGraphId() const noexcept;
    private:
        std::vector<Pass> _frontPasses;
        std::vector<Pass> _backPasses;
        std::vector<RenderGraphDefinition> _children;
        RenderGraphId _id;

        void configureFlow(entt::flow& builder) const noexcept;
    };

    class DARMOK_EXPORT RenderGraph final
    {
    public:
        using Matrix = entt::adjacency_matrix<entt::directed_tag>;
        using Resources = RenderGraphResources;
        using Definition = RenderGraphDefinition;

        RenderGraph(Matrix&& matrix, const Definition& def) noexcept;

        void configureViews(bgfx::ViewId viewId = 0);

        size_t size() const noexcept;
        const Definition& getDefinition() const noexcept;

        Resources operator()() const;
        void operator()(Resources& res) const;
        void writeGraphviz(std::ostream& out) const;

        static std::string getMatrixDebugInfo(const Matrix& mtx) noexcept;

    private:
        Matrix _matrix;
        Definition _def;

        bool execute(Resources& res, std::unordered_set<size_t>& executed) const;
    };

}