#pragma once
#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/utils.hpp>

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <entt/entt.hpp>
#include <taskflow/taskflow.hpp>

#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <map>


namespace darmok
{
    class App;

    using RenderGraphId = entt::id_type;

    struct RenderResourceDefinition final
    {
        entt::hashed_string name;
        RenderGraphId type;

        template<typename T>
        [[nodiscard]] static RenderResourceDefinition create(entt::hashed_string name) noexcept
        {
            return { name, entt::type_hash<T>::value() };
        }

        [[nodiscard]] bool operator==(const RenderResourceDefinition& other) const noexcept;
        [[nodiscard]] bool operator!=(const RenderResourceDefinition& other) const noexcept;
        [[nodiscard]] bool operator<(const RenderResourceDefinition& other) const noexcept;

        [[nodiscard]] RenderGraphId id() const noexcept;
        [[nodiscard]] size_t hash() const noexcept;
    };

    class DARMOK_EXPORT RenderResourcesDefinition
    {
    public:
        using Resource = RenderResourceDefinition;
        using ConstIterator = std::vector<Resource>::const_iterator;

        [[nodiscard]] bool operator==(const RenderResourcesDefinition& other) const noexcept;
        [[nodiscard]] bool operator!=(const RenderResourcesDefinition& other) const noexcept;

        [[nodiscard]] ConstIterator begin() const;
        [[nodiscard]] ConstIterator end() const;
        [[nodiscard]] bool contains(const Resource& res) const noexcept;
        RenderResourcesDefinition& add(const Resource& res) noexcept;

        template<typename T>
        [[nodiscard]] bool contains(entt::hashed_string name = "") const noexcept
        {
            return contains(Resource::create<T>(name));
        }

        template<typename T>
        RenderResourcesDefinition& add(entt::hashed_string name = "")
        {
            _resources.push_back(Resource::create<T>(name));
            return *this;
        }

        [[nodiscard]] size_t hash() const noexcept;

    private:
        std::vector<Resource> _resources;
    };

    class DARMOK_EXPORT RenderGraphResources final
    {
    public:
        RenderGraphResources() noexcept = default;
        RenderGraphResources(const RenderGraphResources& other) noexcept;

        template<typename T>
        RenderGraphResources& set(const T& value, entt::hashed_string name = "") noexcept
        {
            std::scoped_lock lock(_mutex);
            getAny<T>(name) = value;
            return *this;
        }

        template<typename T, typename... A>
        RenderGraphResources& emplace(entt::hashed_string name, A&&... args) noexcept
        {
            std::scoped_lock lock(_mutex);
            getAny<T>(name).emplace<T>(std::forward<A>(args)...);
            return *this;
        }

        template<typename T, typename... A>
        RenderGraphResources& emplaceDef(A&&... args) noexcept
        {
            return emplace<T>("", std::forward<A>(args)...);
        }

        template<typename T>
        RenderGraphResources& setRef(T& value, entt::hashed_string name = "") noexcept
        {
            std::scoped_lock lock(_mutex);
            getAny<T>(name) = std::reference_wrapper<T>(value);
            return *this;
        }

        template<typename T>
        bool get(T& val, entt::hashed_string name = "") const noexcept
        {
            if (auto ref = get<T>(name))
            {
                val = ref.value();
                return true;
            }
            return false;
        }

        template<typename T>
        [[nodiscard]] OptionalRef<const T> get(entt::hashed_string name = "") const noexcept
        {
            std::scoped_lock lock(_mutex);
            auto itr = _data.find(getKey<T>(name));
            if (itr != _data.end())
            {
                return castAny<T>(itr->second);
            }
            return nullptr;
        }

        template<typename T>
        [[nodiscard]] OptionalRef<T> get(entt::hashed_string name = "") noexcept
        {
            std::scoped_lock lock(_mutex);
            auto itr = _data.find(getKey<T>(name));
            if (itr != _data.end())
            {
                return castAny<T>(itr->second);
            }
            return nullptr;
        }

        template<typename T>
        bool remove(entt::hashed_string name = "") noexcept
        {
            std::scoped_lock lock(_mutex);
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return false;
            }
            _data.erase(itr);
            return true;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            std::scoped_lock lock(_mutex);
            return _data.empty();
        }

        [[nodiscard]] size_t size() const noexcept
        {
            std::scoped_lock lock(_mutex);
            return _data.size();
        }

    private:
        mutable std::mutex _mutex;
        std::unordered_map<RenderGraphId, entt::any> _data;

        template<typename T>
        static RenderGraphId getKey(entt::hashed_string name = "")
        {
            return idTypeCombine(name.value(), entt::type_hash<T>::value());
        };

        template<typename T>
        entt::any& getAny(entt::hashed_string name = "") noexcept
        {
            return _data[getKey<T>(name)];
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
            if (auto ref = entt::any_cast<std::reference_wrapper<T>>(ptr))
            {
                return ref->get();
            }
            return nullptr;
        }
    };

    class RenderGraph;

    class DARMOK_EXPORT BX_NO_VTABLE IRenderGraphContext
    {
    public:
        virtual ~IRenderGraphContext() = default;
        [[nodiscard]] virtual bgfx::Encoder& getEncoder() noexcept = 0;
        [[nodiscard]] virtual bgfx::ViewId getViewId() const = 0;
        [[nodiscard]] virtual RenderGraphResources& getResources() noexcept = 0;
    };

    class DARMOK_EXPORT RenderGraphContext : public IRenderGraphContext
    {
    public:
        RenderGraphContext(RenderGraph& graph, RenderGraphId id);
        [[nodiscard]] bgfx::Encoder& getEncoder() noexcept override;
        [[nodiscard]] bgfx::ViewId getViewId() const override;
        void setViewId(bgfx::ViewId viewId = -1) noexcept;
        [[nodiscard]] RenderGraphResources& getResources() noexcept override;
        [[nodiscard]] RenderGraphContext createChild(RenderGraphId id) const noexcept;
    private:
        std::reference_wrapper<RenderGraph> _graph;
        RenderGraphId _id;
        bgfx::ViewId _viewId;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderPassDelegate
    {
    public:
        virtual ~IRenderPassDelegate() = default;
        virtual void renderPassConfigure(bgfx::ViewId viewId) { };
        virtual void renderPassExecute(IRenderGraphContext& context) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderGraphNode
    {
    public:
        static const int kMaxPriority;
        using Resources = RenderResourcesDefinition;
        virtual ~IRenderGraphNode() = default;
        [[nodiscard]] virtual RenderGraphId id() const = 0;
        [[nodiscard]] virtual size_t hash() const = 0;
        [[nodiscard]] virtual const Resources& getReadResources() const = 0;
        [[nodiscard]] virtual const Resources& getWriteResources() const = 0;
        [[nodiscard]] virtual bool getSync() const { return false; }
        [[nodiscard]] virtual int getPriority() const = 0;
        [[nodiscard]] virtual bgfx::ViewId configureView(bgfx::ViewId viewId) = 0;
        [[nodiscard]] virtual tf::Task createTask(tf::FlowBuilder& flowBuilder, RenderGraphContext& context) = 0;
        [[nodiscard]] virtual std::unique_ptr<IRenderGraphNode> copyNode() const = 0;
        
        [[nodiscard]] virtual bool hasPassDelegate(const IRenderPassDelegate& dlg) const { return false; }
    };

    class DARMOK_EXPORT RenderPassDefinition final : public IRenderGraphNode
    {
    public:
        using Resources = RenderResourcesDefinition;

        RenderPassDefinition(const std::string& name = "") noexcept;

        [[nodiscard]] bool operator==(const RenderPassDefinition& other) const noexcept;
        [[nodiscard]] bool operator!=(const RenderPassDefinition& other) const noexcept;

        RenderPassDefinition& setName(const std::string& name) noexcept;
        [[nodiscard]] const std::string& getName() const noexcept;

        RenderPassDefinition& setPriority(int prio) noexcept;
        [[nodiscard]] int getPriority() const noexcept override;

        RenderPassDefinition& setDelegate(IRenderPassDelegate& dlg) noexcept;
        [[nodiscard]] bool hasPassDelegate(const IRenderPassDelegate& dlg) const noexcept override;

        bgfx::ViewId configureView(bgfx::ViewId viewId) noexcept override;
        [[nodiscard]] tf::Task createTask(tf::FlowBuilder& flowBuilder, RenderGraphContext& context) noexcept override;
        [[nodiscard]] std::unique_ptr<IRenderGraphNode> copyNode() const noexcept override;

        [[nodiscard]] Resources& getReadResources() noexcept;
        [[nodiscard]] const Resources& getReadResources() const noexcept override;
        [[nodiscard]] Resources& getWriteResources() noexcept;
        [[nodiscard]] const Resources& getWriteResources() const noexcept override;
        
        RenderPassDefinition& setSync(bool sync) noexcept;
        [[nodiscard]] bool getSync() const noexcept override;

        [[nodiscard]] RenderGraphId id() const noexcept override;
        [[nodiscard]] size_t hash() const noexcept override;
    private:
        std::string _name;
        int _priority;
        bool _sync;
        RenderGraphId _id;
        bgfx::ViewId _viewId;
        Resources _read;
        Resources _write;
        OptionalRef<IRenderPassDelegate> _delegate;
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
        using INode = IRenderGraphNode;
        using Pass = RenderPassDefinition;
        using Resources = RenderResourcesDefinition;

        RenderGraphDefinition(const std::string& name = "", int priority = 0) noexcept;
        RenderGraphDefinition(const RenderGraphDefinition& other) noexcept;

        [[nodiscard]] bool operator==(const RenderGraphDefinition& other) const noexcept;
        [[nodiscard]] bool operator!=(const RenderGraphDefinition& other) const noexcept;

        RenderGraphDefinition& setName(const std::string& name) noexcept;
        [[nodiscard]] const std::string& getName() const noexcept;

        RenderGraphDefinition& setPriority(int priority) noexcept;
        [[nodiscard]] int getPriority() const noexcept;

        const Pass& addPass(IRenderPass& pass);
        [[nodiscard]] Pass& addPass() noexcept;
        bool removePass(const IRenderPassDelegate& dlg) noexcept;
        bool removePass(size_t hash) noexcept;

        bool setChild(const RenderGraphDefinition& def);

        [[nodiscard]] bool hasNode(RenderGraphId id) const noexcept;
        bool removeNode(RenderGraphId id) noexcept;
        [[nodiscard]] OptionalRef<INode> getNode(RenderGraphId id) noexcept;
        [[nodiscard]] OptionalRef<const INode> getNode(RenderGraphId id) const noexcept;

        void clear();
        [[nodiscard]] size_t size() const noexcept;

        [[nodiscard]] const INode& operator[](size_t vertex) const;
        [[nodiscard]] INode& operator[](size_t vertex);

        [[nodiscard]] size_t hash() const noexcept;
        [[nodiscard]] RenderGraphId id() const noexcept;
    private:
        std::string _name;
        int _priority;
        using Nodes = std::vector<std::unique_ptr<INode>>;
        Nodes _nodes;
        RenderGraphId _id;

        Pass& doAddPass(bool front) noexcept;
        bool doSetChild(const RenderGraphDefinition& def, bool front) noexcept;

        Nodes::iterator findNode(RenderGraphId id) noexcept;
        Nodes::const_iterator findNode(RenderGraphId id) const noexcept;
    };

    class DARMOK_EXPORT RenderGraphNode final : public IRenderGraphNode
    {
    public:
        using Matrix = entt::adjacency_matrix<entt::directed_tag>;
        using Definition = RenderGraphDefinition;
        using ResourcesDefinition = RenderResourcesDefinition;
        using Resources = RenderGraphResources;

        RenderGraphNode(const Definition& def) noexcept;
        RenderGraphNode(const RenderGraphNode& other) noexcept;

        [[nodiscard]] const ResourcesDefinition& getReadResources() const noexcept override;
        [[nodiscard]] const ResourcesDefinition& getWriteResources() const noexcept override;
        [[nodiscard]] int getPriority() const noexcept override;
        bgfx::ViewId configureView(bgfx::ViewId viewId = 0) override;
        [[nodiscard]] tf::Task createTask(tf::FlowBuilder& builder, RenderGraphContext& context) noexcept override;
        [[nodiscard]] std::unique_ptr<IRenderGraphNode> copyNode() const noexcept override;
        [[nodiscard]] RenderGraphId id() const noexcept override;
        [[nodiscard]] size_t hash() const noexcept override;

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] const Definition& getDefinition() const noexcept;
        [[nodiscard]] Definition& getDefinition() noexcept;
        void configureTasks(tf::FlowBuilder& builder, RenderGraphContext& context);

    private:
        Matrix _matrix;
        Definition _def;
        std::vector<size_t> _indices;
        int _priority;
        tf::Taskflow _taskflow;
        ResourcesDefinition _read;
        ResourcesDefinition _write;

        IRenderGraphNode& getVertexNode(int vertex);
    };

    class DARMOK_EXPORT RenderGraph final
    {
    public:
        using Resources = RenderGraphResources;
        using Definition = RenderGraphDefinition;
        RenderGraph(const Definition& def) noexcept;
        [[nodiscard]] size_t hash() const noexcept;
        void dump(std::ostream& out) const;

        bgfx::ViewId configureView(bgfx::ViewId viewId = 0);

        void operator()(tf::Executor& executor) const;

        [[nodiscard]] bgfx::Encoder& getEncoder();
        [[nodiscard]] Resources& getResources() noexcept;
        [[nodiscard]] Resources& getResources(RenderGraphId id) noexcept;

    private:
        RenderGraphNode _root;
        mutable tf::Taskflow _taskflow;
        std::mutex _encoderMutex;
        mutable std::unordered_map<std::thread::id, bgfx::Encoder*> _encoders;
        std::unordered_map<RenderGraphId, Resources> _resources;
    };
}