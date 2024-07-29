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

    RenderPassDefinition::RenderPassDefinition(const std::string& name) noexcept
        : _id(randomIdType())
        , _viewId(-1)
        , _name(name)
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

    bgfx::ViewId RenderPassDefinition::getViewId() const noexcept
    {
        return _viewId;
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
            _delegate->renderPassExecute(context);
        });
        task.name(_name);
        return task;
    }

    std::unique_ptr<IRenderGraphNode> RenderPassDefinition::copyNode() const noexcept
    {
        return std::make_unique<RenderPassDefinition>(*this);
    }

    const RenderPassDefinition::Resources& RenderPassDefinition::getInputs() const noexcept
    {
        return _inputs;
    }

    const RenderPassDefinition::Resources& RenderPassDefinition::getOutputs() const noexcept
    {
        return _outputs;
    }

    RenderPassDefinition::Resources& RenderPassDefinition::getInputs() noexcept
    {
        return _inputs;
    }

    RenderPassDefinition::Resources& RenderPassDefinition::getOutputs() noexcept
    {
        return _outputs;
    }

    RenderGraphId RenderPassDefinition::id() const noexcept
    {
        return _id;
    }

    size_t RenderPassDefinition::hash() const noexcept
    {
        size_t hash = 0;
        hashCombine(hash, _inputs.hash(), _outputs.hash(), _name, _viewId);
        return hash;
    }

    RenderGraphDefinition::RenderGraphDefinition(const std::string& name) noexcept
        : _id(randomIdType())
        , _name(name)
    {
    }

    RenderGraphDefinition::RenderGraphDefinition(const RenderGraphDefinition& other) noexcept
        : _id(other._id)
        , _name(other._name)
        , _inputs(other._inputs)
        , _outputs(other._outputs)
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
        hashCombine(hash, _inputs.hash(), _outputs.hash(), _name);
        for (auto& node : _nodes)
        {
            hashCombine(hash, node->hash());
        }
        return hash;
    }

    RenderGraphDefinition::Pass& RenderGraphDefinition::doAddPass(bool front) noexcept
    {
        auto ptr = std::make_unique<Pass>();
        auto& ref = *ptr;
        if (front)
        {
            _nodes.insert(_nodes.begin(), std::move(ptr));
        }
        else
        {
            _nodes.push_back(std::move(ptr));
        }
        return ref;
    }

    RenderGraphDefinition::Pass& RenderGraphDefinition::addPassFront() noexcept
    {
        return doAddPass(true);
    }

    RenderPassDefinition& RenderGraphDefinition::addPass() noexcept
    {
        return doAddPass(false);
    }

    const RenderGraphDefinition::Pass& RenderGraphDefinition::addPassFront(IRenderPass& pass)
    {
        auto& def = addPassFront();
        def.setDelegate(pass);
        pass.renderPassDefine(def);
        return def;
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

    bool RenderGraphDefinition::setChild(const RenderGraphDefinition& def) noexcept
    {
        return doSetChild(def, false);
    }

    bool RenderGraphDefinition::setChildFront(const RenderGraphDefinition& def) noexcept
    {
        return doSetChild(def, true);
    }

    bool RenderGraphDefinition::doSetChild(const RenderGraphDefinition& def, bool front) noexcept
    {
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
        else if (front)
        {
            _nodes.insert(_nodes.begin(), std::move(ptr));
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
            _nodes.erase(itr, _nodes.end());
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

    RenderGraphDefinition::Resources& RenderGraphDefinition::getInputs() noexcept
    {
        return _inputs;
    }

    const RenderGraphDefinition::Resources& RenderGraphDefinition::getInputs() const noexcept
    {
        return _inputs;
    }

    RenderGraphDefinition::Resources& RenderGraphDefinition::getOutputs() noexcept
    {
        return _outputs;
    }

    const RenderGraphDefinition::Resources& RenderGraphDefinition::getOutputs() const noexcept
    {
        return _outputs;
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
    {
        entt::flow builder;
        for(size_t i =0; i<def.size(); i++)
        {
            auto& node = def[i];
            builder.bind(node.id());
            auto& inputs = node.getInputs();
            auto& outputs = node.getOutputs();
            auto sync = true;
            for (auto& input : inputs)
            {
                builder.ro(input.id());
                if (!outputs.contains(input))
                {
                    // if all inputs of the node are also outputs
                    // it means the node is not producing anything new
                    // just modifying some resources
                    sync = false;
                }
            }
            for (auto& output : node.getOutputs())
            {
                builder.rw(output.id());
            }
            if (sync)
            {
                builder.sync();
            }
        }
        _matrix = builder.graph();
    }

    const RenderGraphNode::ResourcesDefinition& RenderGraphNode::getInputs() const noexcept
    {
        return _def.getInputs();
    }

    const RenderGraphNode::ResourcesDefinition& RenderGraphNode::getOutputs() const noexcept
    {
        return _def.getOutputs();
    }

    bgfx::ViewId RenderGraphNode::configureView(bgfx::ViewId viewId)
    {
        for (auto&& vertex : _matrix.vertices())
        {
            auto& node = _def[vertex];
            viewId = node.configureView(viewId);
        }
        return viewId;
    }

    tf::Task RenderGraphNode::createTask(tf::FlowBuilder& flowBuilder, RenderGraphContext& context) noexcept
    {
        // TODO: add delegate to copy data from parent context
        auto childContext = context.createChild(id());
        auto task = flowBuilder.emplace([this, childContext](tf::Subflow& subflow) mutable {
            configureTasks(subflow, childContext);
        });
        task.name(_def.getName());
        return task;
    }

    void RenderGraphNode::configureTasks(tf::FlowBuilder& builder, RenderGraphContext& context)
    {
        std::unordered_map<size_t, tf::Task> tasks;
        for (auto&& vertex : _matrix.vertices())
        {
            auto& node = _def[vertex];
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

    bgfx::Encoder& RenderGraph::getEncoder() noexcept
    {
        std::scoped_lock lock(_encoderMutex);
        auto id = std::this_thread::get_id();
        auto itr = _encoders.find(id);
        if (itr == _encoders.end())
        {
            itr = _encoders.emplace(id, bgfx::begin()).first;
        }
        return *itr->second;
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
    {
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