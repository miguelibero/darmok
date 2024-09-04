#include <darmok/render_graph.hpp>
#include <darmok/string.hpp>
#include <stdexcept>
#include <sstream>
#include <taskflow/taskflow.hpp>
#include <thread>

namespace darmok
{
    RenderGraphId RenderResourceDefinition::id() const noexcept
    {
        return idTypeCombine(name, type);
    }

    size_t RenderResourceDefinition::hash() const noexcept
    {
        size_t hash = 0;
        hashCombine(hash, name.value(), type);
        return hash;
    }

    bool RenderResourceDefinition::operator==(const RenderResourceDefinition& other) const noexcept
    {
        return id() == other.id();
    }

    bool RenderResourceDefinition::operator!=(const RenderResourceDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    bool RenderResourceDefinition::operator<(const RenderResourceDefinition& other) const noexcept
    {
        return id() < other.id();
    }

    RenderResourcesDefinition::ConstIterator RenderResourcesDefinition::begin() const
    {
        return _resources.begin();
    }

    RenderResourcesDefinition::ConstIterator RenderResourcesDefinition::end() const
    {
        return _resources.end();
    }

    bool RenderResourcesDefinition::operator==(const RenderResourcesDefinition& other) const noexcept
    {
        return _resources == other._resources;
    }

    bool RenderResourcesDefinition::operator!=(const RenderResourcesDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    bool RenderResourcesDefinition::contains(const Resource& res) const noexcept
    {
        return std::find(_resources.begin(), _resources.end(), res) != _resources.end();
    }

    size_t RenderResourcesDefinition::hash() const noexcept
    {
        size_t hash = 0;
        for (auto& res : _resources)
        {
            hashCombine(hash, res.hash());
        }
        return hash;
    }

    RenderGraphResources::RenderGraphResources(const RenderGraphResources& other) noexcept
        : _data(other._data)
    {
    }

    const int IRenderGraphNode::kMaxPriority = std::numeric_limits<int>::max();

    RenderPassDefinition::RenderPassDefinition(const std::string& name) noexcept
        : _id(randomIdType())
        , _name(name)
        , _delegate(nullptr)
        , _priority(0)
        , _viewId(-1)
        , _sync(false)
    {
    }

    RenderPassDefinition& RenderPassDefinition::setName(const std::string& name) noexcept
    {
        _name = name;
        return *this;
    }

    const std::string& RenderPassDefinition::getName() const noexcept
    {
        return _name;
    }

    RenderPassDefinition& RenderPassDefinition::setPriority(int prio) noexcept
    {
        _priority = prio;
        return *this;
    }

    int RenderPassDefinition::getPriority() const noexcept
    {
        return _priority;
    }

    bool RenderPassDefinition::operator==(const RenderPassDefinition& other) const noexcept
    {
        return hash() == other.hash();
    }

    bool RenderPassDefinition::operator!=(const RenderPassDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    RenderPassDefinition& RenderPassDefinition::setDelegate(IRenderPassDelegate& dlg) noexcept
    {
        _delegate = dlg;
        return *this;
    }

    OptionalRef<IRenderPassDelegate> RenderPassDefinition::getDelegate() const noexcept
    {
        return _delegate;
    }

    bool RenderPassDefinition::hasPassDelegate(IRenderPassDelegate& dlg) const noexcept
    {
        return _delegate == dlg;
    }

    bgfx::ViewId RenderPassDefinition::configureView(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        bgfx::resetView(viewId);
        bgfx::setViewName(viewId, _name.c_str());

        if (_delegate)
        {
            _delegate->renderPassConfigure(viewId);
        }

        return viewId + 1;
    }

    tf::Task RenderPassDefinition::createTask(tf::FlowBuilder& flowBuilder, RenderGraphContext& context) noexcept
    {
        auto task = flowBuilder.emplace([this, context]() mutable {
            if (!_delegate)
            {
                return;
            }
            context.setViewId(_viewId);
            _delegate->renderPassExecute(context);
            context.setViewId();
        });
        task.name(_name);
        return task;
    }

    std::unique_ptr<IRenderGraphNode> RenderPassDefinition::copyNode() const noexcept
    {
        return std::make_unique<RenderPassDefinition>(*this);
    }

    const RenderPassDefinition::Resources& RenderPassDefinition::getReadResources() const noexcept
    {
        return _read;
    }

    const RenderPassDefinition::Resources& RenderPassDefinition::getWriteResources() const noexcept
    {
        return _write;
    }

    RenderPassDefinition::Resources& RenderPassDefinition::getReadResources() noexcept
    {
        return _read;
    }

    RenderPassDefinition::Resources& RenderPassDefinition::getWriteResources() noexcept
    {
        return _write;
    }

    RenderPassDefinition& RenderPassDefinition::setSync(bool sync) noexcept
    {
        _sync = sync;
        return *this;
    }

    bool RenderPassDefinition::getSync() const noexcept
    {
        return _sync;
    }

    RenderGraphId RenderPassDefinition::id() const noexcept
    {
        return _id;
    }

    size_t RenderPassDefinition::hash() const noexcept
    {
        size_t hash = 0;
        hashCombine(hash, _read.hash(), _write.hash(), _name);
        return hash;
    }

    RenderGraphDefinition::RenderGraphDefinition(const std::string& name, int priority) noexcept
        : _id(randomIdType())
        , _name(name)
        , _priority(priority)
    {
    }

    RenderGraphDefinition::RenderGraphDefinition(const RenderGraphDefinition& other) noexcept
        : _id(other._id)
        , _name(other._name)
        , _read(other._read)
        , _write(other._write)
    {
        _nodes.reserve(other._nodes.size());
        for (auto& node : other._nodes)
        {
            _nodes.push_back(node->copyNode());
        }
    }

    RenderGraphDefinition& RenderGraphDefinition::setName(const std::string& name) noexcept
    {
        _name = name;
        return *this;
    }

    const std::string& RenderGraphDefinition::getName() const noexcept
    {
        return _name;
    }

    RenderGraphDefinition& RenderGraphDefinition::setPriority(int priority) noexcept
    {
        _priority = priority;
        return *this;
    }

    int RenderGraphDefinition::getPriority() const noexcept
    {
        return _priority;
    }

    bool RenderGraphDefinition::operator==(const RenderGraphDefinition& other) const noexcept
    {
        return hash() != other.hash();
    }

    bool RenderGraphDefinition::operator!=(const RenderGraphDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    RenderGraphId RenderGraphDefinition::id() const noexcept
    {
        return _id;
    }

    size_t RenderGraphDefinition::hash() const noexcept
    {
        size_t hash = 0;
        hashCombine(hash, _read.hash(), _write.hash(), _name);
        for (auto& node : _nodes)
        {
            hashCombine(hash, node->hash());
        }
        return hash;
    }

    RenderPassDefinition& RenderGraphDefinition::addPass() noexcept
    {
        auto ptr = std::make_unique<Pass>();
        auto& ref = *ptr;
        _nodes.push_back(std::move(ptr));
        return ref;
    }

    const RenderGraphDefinition::Pass& RenderGraphDefinition::addPass(IRenderPass& pass)
    {
        auto& def = addPass();
        def.setDelegate(pass);
        pass.renderPassDefine(def);
        return def;
    }

    bool RenderGraphDefinition::removePass(IRenderPassDelegate& dlg) noexcept
    {
        auto itr = std::remove_if(_nodes.begin(), _nodes.end(), [&dlg](auto& node) { return node->hasPassDelegate(dlg); });
        if (itr == _nodes.end())
        {
            return false;
        }
        _nodes.erase(itr, _nodes.end());
        return true;
    }

    bool RenderGraphDefinition::setChild(const RenderGraphDefinition& def)
    {
        if (this == &def)
        {
            throw std::invalid_argument("cannot add itself as a child");
        }
        auto itr = findNode(def.id());
        auto found = itr != _nodes.end();
        if (found && (*itr)->hash() == def.hash())
        {
            return false;
        }

        auto ptr = std::make_unique<RenderGraphNode>(def);
        auto& ref = *ptr;

        if (found)
        {
            *itr = std::move(ptr);
        }
        else
        {
            _nodes.push_back(std::move(ptr));
        }
        return true;
    }

    RenderGraphDefinition::Nodes::iterator RenderGraphDefinition::findNode(RenderGraphId id) noexcept
    {
        return std::find_if(_nodes.begin(), _nodes.end(), [id](auto& node) { return node->id() == id; });
    }

    RenderGraphDefinition::Nodes::const_iterator RenderGraphDefinition::findNode(RenderGraphId id) const noexcept
    {
        return std::find_if(_nodes.begin(), _nodes.end(), [id](auto& node) { return node->id() == id; });
    }

    bool RenderGraphDefinition::hasNode(RenderGraphId id) const noexcept
    {
        auto itr = findNode(id);
        return itr != _nodes.end();
    }

    OptionalRef<RenderGraphDefinition::INode> RenderGraphDefinition::getNode(RenderGraphId id) noexcept
    {
        auto itr = findNode(id);
        if (itr == _nodes.end())
        {
            return nullptr;
        }
        return itr->get();
    }

    OptionalRef<const RenderGraphDefinition::INode> RenderGraphDefinition::getNode(RenderGraphId id) const noexcept
    {
        auto itr = findNode(id);
        if (itr == _nodes.end())
        {
            return nullptr;
        }
        return itr->get();
    }

    bool RenderGraphDefinition::removeNode(RenderGraphId id) noexcept
    {
        auto itr = findNode(id);
        if (itr != _nodes.end())
        {
            _nodes.erase(itr);
            return true;
        }
        return false;
    }

    void RenderGraphDefinition::clear()
    {
        _nodes.clear();
    }

    size_t RenderGraphDefinition::size() const noexcept
    {
        return _nodes.size();
    }

    RenderGraphDefinition::Resources& RenderGraphDefinition::getReadResources() noexcept
    {
        return _read;
    }

    const RenderGraphDefinition::Resources& RenderGraphDefinition::getReadResources() const noexcept
    {
        return _read;
    }

    RenderGraphDefinition::Resources& RenderGraphDefinition::getWriteResources() noexcept
    {
        return _write;
    }

    const RenderGraphDefinition::Resources& RenderGraphDefinition::getWriteResources() const noexcept
    {
        return _write;
    }

    const RenderGraphDefinition::INode& RenderGraphDefinition::operator[](size_t vertex) const
    {
        return *_nodes.at(vertex);
    }

    RenderGraphDefinition::INode& RenderGraphDefinition::operator[](size_t vertex)
    {
        return *_nodes.at(vertex);
    }

    RenderGraphNode::RenderGraphNode(const Definition& def) noexcept
        : _def(def)
        , _taskflow(def.getName())
        , _priority(def.getPriority())
    {

        // sort nodes based on priority
        std::vector<std::pair<size_t, int>> prios;
        prios.reserve(_def.size());
        for (size_t i = 0; i < def.size(); i++)
        {
            auto& def = _def[i];
            auto prio = std::make_pair(i, def.getPriority());
            prios.insert(std::upper_bound(prios.begin(), prios.end(), prio,
                [](auto& a, auto& b) { return a.second > b.second; }), prio);
        }
        _indices.reserve(prios.size());
        for (auto& prio : prios)
        {
            _indices.push_back(prio.first);
        }

        entt::flow builder;
        for(auto& i : _indices)
        {
            auto& node = _def[i];
            builder.bind(node.id());
            for (auto& input : node.getReadResources())
            {
                builder.ro(input.id());
            }
            for (auto& output : node.getWriteResources())
            {
                builder.rw(output.id());
            }
            if (node.getSync())
            {
                builder.sync();
            }
        }
        _matrix = builder.graph();
    }

    RenderGraphNode::RenderGraphNode(const RenderGraphNode& other) noexcept
        : _def(other._def)
        , _matrix(other._matrix)
        , _taskflow(_def.getName())
        , _priority(other._priority)
        , _indices(other._indices)
    {
    }

    const RenderGraphNode::ResourcesDefinition& RenderGraphNode::getReadResources() const noexcept
    {
        return _def.getReadResources();
    }

    const RenderGraphNode::ResourcesDefinition& RenderGraphNode::getWriteResources() const noexcept
    {
        return _def.getWriteResources();
    }

    int RenderGraphNode::getPriority() const noexcept
    {
        return _priority;
    }

    IRenderGraphNode& RenderGraphNode::getVertexNode(int vertex)
    {
        return _def[_indices[vertex]];
    }

    bgfx::ViewId RenderGraphNode::configureView(bgfx::ViewId viewId)
    {
        for (auto&& vertex : _matrix.vertices())
        {
            auto& node = getVertexNode(vertex);
            viewId = node.configureView(viewId);
        }
        return viewId;
    }

    tf::Task RenderGraphNode::createTask(tf::FlowBuilder& builder, RenderGraphContext& context) noexcept
    {
        // TODO: add delegate to copy data from parent context
        auto childContext = context.createChild(id());
        _taskflow.clear();
        configureTasks(_taskflow, childContext);
        auto task = builder.composed_of(_taskflow);
        task.name(_def.getName());
        return task;
    }

    void RenderGraphNode::configureTasks(tf::FlowBuilder& builder, RenderGraphContext& context)
    {
        std::unordered_map<size_t, tf::Task> tasks;
        for (auto&& vertex : _matrix.vertices())
        {
            auto& node = getVertexNode(vertex);
            tasks[vertex] = node.createTask(builder, context);
        }
        for (auto& [vertex, task] : tasks)
        {
            auto in_edges = _matrix.in_edges(vertex);
            for (auto [lhs, rhs] : in_edges)
            {
                task.succeed(tasks[lhs]);
            }
        }
    }

    std::unique_ptr<IRenderGraphNode> RenderGraphNode::copyNode() const noexcept
    {
        return std::make_unique<RenderGraphNode>(*this);
    }

    RenderGraphId RenderGraphNode::id() const noexcept
    {
        return _def.id();
    }

    size_t RenderGraphNode::hash() const noexcept
    {
        return _def.hash();
    }

    bool RenderGraphNode::empty() const noexcept
    {
        return size() == 0;
    }

    size_t RenderGraphNode::size() const noexcept
    {
        return _matrix.size();
    }

    const RenderGraphNode::Definition& RenderGraphNode::getDefinition() const noexcept
    {
        return _def;
    }

    RenderGraphNode::Definition& RenderGraphNode::getDefinition() noexcept
    {
        return _def;
    }

    RenderGraph::RenderGraph(const Definition& def) noexcept
        : _root(def)
    {
        _taskflow.name("root");
        RenderGraphContext context(*this, _root.id());
        _root.configureTasks(_taskflow, context);
    }

    size_t RenderGraph::hash() const noexcept
    {
        return _root.hash();
    }

    void RenderGraph::dump(std::ostream& out) const
    {
        _taskflow.dump(out);
    }

    bgfx::ViewId RenderGraph::configureView(bgfx::ViewId viewId)
    {
        return _root.configureView(viewId);
    }

    bgfx::Encoder& RenderGraph::getEncoder()
    {
        std::scoped_lock lock(_encoderMutex);
        auto id = std::this_thread::get_id();
        auto itr = _encoders.find(id);
        bgfx::Encoder* encoder = nullptr;
        if (itr == _encoders.end())
        {
            encoder = bgfx::begin(true);
            _encoders.emplace(id, encoder);
        }
        else
        {
            encoder = itr->second;
        }
        if (!encoder)
        {
            throw std::runtime_error("bgfx did not return an encoder");
        }
        return *encoder;
    }

    RenderGraphResources& RenderGraph::getResources() noexcept
    {
        return getResources(_root.id());
    }

    RenderGraphResources& RenderGraph::getResources(RenderGraphId id) noexcept
    {
        auto itr = _resources.find(id);
        if (itr == _resources.end())
        {
            itr = _resources.emplace(id, Resources()).first;
        }
        return itr->second;
    }

    void RenderGraph::operator()(tf::Executor& executor) const
    {
        _encoders.clear();
        executor.run(_taskflow).wait();
        for (auto& [thread_id, encoder] : _encoders)
        {
            bgfx::end(encoder);
        }
        _encoders.clear();
    }

    RenderGraphContext::RenderGraphContext(RenderGraph& graph, RenderGraphId id)
        : _graph(graph)
        , _id(id)
        , _viewId(-1)
    {
    }

    bgfx::ViewId RenderGraphContext::getViewId() const
    {
        if (_viewId == -1)
        {
            throw std::runtime_error("no view id set");
        }
        return _viewId;
    }

    void RenderGraphContext::setViewId(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
    }

    bgfx::Encoder& RenderGraphContext::getEncoder() noexcept
    {
        return _graph.get().getEncoder();
    }

    RenderGraphResources& RenderGraphContext::getResources() noexcept
    {
        return _graph.get().getResources(_id);
    }

    RenderGraphContext RenderGraphContext::createChild(RenderGraphId id) const noexcept
    {
        return RenderGraphContext(_graph, id);
    }

}