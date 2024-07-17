#include <darmok/render_graph.hpp>
#include <darmok/string.hpp>
#include <stdexcept>
#include <sstream>

namespace darmok
{
    RenderGraphId RenderResourceDefinition::id() const noexcept
    {
        return idTypeCombine(name, type);
    }

    RenderResourceDefinition::operator RenderGraphId() const noexcept
    {
        return id();
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

    const RenderGraphId RenderGraphResources::defaultGroup = (RenderGraphId)-1;

    RenderGraphResources::RenderGraphResources(RenderGraphId group) noexcept
        : _group(group)
    {
    }

    RenderGraphResources& RenderGraphResources::setGroup(RenderGraphId group) noexcept
    {
        _group = group;
        return *this;
    }

    RenderPassDefinition::RenderPassDefinition(RenderGraphId group) noexcept
        : _group(group)
        , _id(randomIdType())
        , _viewId(-1)
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

    RenderGraphId RenderPassDefinition::getGroup() const noexcept
    {
        return _group;
    }

    bgfx::ViewId RenderPassDefinition::getViewId() const noexcept
    {
        return _viewId;
    }

    bool RenderPassDefinition::operator==(const RenderPassDefinition& other) const noexcept
    {
        return _name == other._name && _inputs == other._inputs && _outputs == other._outputs;
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

    void RenderPassDefinition::operator()(RenderGraphResources& res) const noexcept
    {
        if (!_delegate)
        {
            return;
        }
        _delegate->renderPassExecute(res);
    }

    void RenderPassDefinition::configureView(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        if (_delegate)
        {
            _delegate->renderPassConfigure(viewId);
        }
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

    RenderPassDefinition::operator RenderGraphId() const noexcept
    {
        return id();
    }

    RenderGraphId RenderPassDefinition::id() const noexcept
    {
        return _id;
    }

    bool RenderPassDefinition::getSync() const noexcept
    {
        // if all inputs of the pass are also outputs
        // it means the pass is not producing anything new
        // just modifying some resources
        for (auto& res : _inputs)
        {
            if (!_outputs.contains(res))
            {
                return false;
            }
        }
        return true;
    }

    RenderGraphDefinition::RenderGraphDefinition() noexcept
        : _id(randomIdType())
    {
    }

    bool RenderGraphDefinition::operator==(const RenderGraphDefinition& other) const noexcept
    {
        return _passes != other._passes && _children != other._children;
    }

    bool RenderGraphDefinition::operator!=(const RenderGraphDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    RenderGraphId RenderGraphDefinition::id() const noexcept
    {
        return _id;
    }

    RenderGraphDefinition::operator RenderGraphId() const noexcept
    {
        return id();
    }

    void RenderGraphDefinition::addChild(const RenderGraphDefinition& child) noexcept
    {
        _children.push_back(child);
    }

    RenderPassDefinition& RenderGraphDefinition::addPass() noexcept
    {
        return _passes.emplace_back(_id);
    }

    const RenderGraphDefinition::Pass& RenderGraphDefinition::addPass(IRenderPass& pass)
    {
        auto& def = addPass();
        def.setDelegate(pass);
        pass.renderPassDefine(def);
        return def;
    }

    bool RenderGraphDefinition::hasPass(const std::string& name) const noexcept
    {
        auto itr = std::find_if(_passes.begin(), _passes.end(), [&name](auto& pass) { return pass.getName() == name; });
        if (itr != _passes.end())
        {
            return true;
        }
        for (auto& child : _children)
        {
            if (child.hasPass(name))
            {
                return true;
            }
        }
        return false;
    }

    bool RenderGraphDefinition::removePass(IRenderPassDelegate& dlg)
    {
        auto itr = std::find_if(_passes.begin(), _passes.end(), [&dlg](auto& pass) { return pass.getDelegate() == dlg; });
        if (itr != _passes.end())
        {
            _passes.erase(itr);
            return true;
        }
        for (auto& child : _children)
        {
            if (child.removePass(dlg))
            {
                return true;
            }
        }
        return false;
    }

    size_t RenderGraphDefinition::size() const noexcept
    {
        size_t s = _passes.size();
        for (auto& child : _children)
        {
            s += child.size();
        }
        return s;
    }

    const RenderGraphDefinition::Pass& RenderGraphDefinition::operator[](size_t vertex) const
    {
        return getPass(vertex);
    }

    const RenderGraphDefinition::Pass& RenderGraphDefinition::getPass(size_t vertex) const
    {
        if (vertex < 0)
        {
            throw std::invalid_argument("pass does not exist");
        }
        auto s = _passes.size();
        if (vertex < s)
        {
            return _passes.at(vertex);
        }
        vertex -= s;
        for (auto& child : _children)
        {
            s = child.size();
            if (vertex < s)
            {
                return child.getPass(vertex);
            }
            vertex -= s;
        }
        throw std::invalid_argument("pass does not exist");
    }

    RenderGraphDefinition::Pass& RenderGraphDefinition::getPass(size_t vertex)
    {
        if (vertex < 0)
        {
            throw std::invalid_argument("pass does not exist");
        }
        auto s = _passes.size();
        if (vertex < s)
        {
            return _passes.at(vertex);
        }
        vertex -= s;
        for (auto& child : _children)
        {
            s = child.size();
            if (vertex < s)
            {
                return child.getPass(vertex);
            }
            vertex -= s;
        }
        throw std::invalid_argument("pass does not exist");
    }

    RenderGraphDefinition::ConstIterator RenderGraphDefinition::begin() const
    {
        return _passes.begin();
    }
    
    RenderGraphDefinition::ConstIterator RenderGraphDefinition::end() const
    {
        return _passes.end();
    }

    void RenderGraphDefinition::configureFlow(entt::flow& builder) const noexcept
    {
        auto baseId = id();

        auto getId = [&baseId](const auto& v)
        {
            return idTypeCombine(baseId, v.id());
        };

        for (auto& pass : _passes)
        {
            builder.bind(getId(pass));
            for (auto& input : pass.getInputs())
            {
                builder.ro(getId(input));
            }
            for (auto& output : pass.getOutputs())
            {
                builder.rw(getId(output));
            }
            if (pass.getSync())
            {
                builder.sync();
            }
        }
        for (auto& child : _children)
        {
            child.configureFlow(builder);
        }
    }

    RenderGraph RenderGraphDefinition::compile()
    {
        entt::flow builder;
        configureFlow(builder);
        return RenderGraph(builder.graph(), *this);
    }

    RenderGraph::RenderGraph(Matrix&& matrix, const Definition& def) noexcept
        : _matrix(std::move(matrix))
        , _def(def)
    {
    }

    void RenderGraph::configureViews(bgfx::ViewId initialViewId)
    {
        for (auto&& vertex : _matrix.vertices())
        {
            auto& pass = _def.getPass(vertex);
            auto viewId = initialViewId + vertex;
            bgfx::resetView(viewId);
            bgfx::setViewName(viewId, pass.getName().c_str());
            pass.configureView(viewId);
        }
    }

    size_t RenderGraph::size() const noexcept
    {
        return _matrix.size();
    }

    const RenderGraph::Definition& RenderGraph::getDefinition() const noexcept
    {
        return _def;
    }

    RenderGraph::Resources RenderGraph::operator()() const
    {
        Resources res;
        operator()(res);
        return res;
    }

    void RenderGraph::operator()(Resources& res) const
    {
        std::unordered_set<size_t> executed;
        execute(res, executed);
    }

    bool RenderGraph::execute(Resources& res, std::unordered_set<size_t>& executed) const
    {
        std::vector<size_t> readyVertices;
        auto pending = false;
        for (auto&& vertex : _matrix.vertices())
        {
            if (executed.contains(vertex))
            {
                continue;
            }
            pending = true;
            auto in_edges = _matrix.in_edges(vertex);
            auto ready = true;
            for (auto [lhs, rhs] : in_edges)
            {
                if (!executed.contains(lhs))
                {
                    ready = false;
                    break;
                }
            }
            if (!ready)
            {
                continue;
            }
            readyVertices.push_back(vertex);
        }
        if (!pending)
        {
            return true;
        }
        if (readyVertices.empty())
        {
            return false;
        }
        auto mainGroup = _def.id();

        for (auto& vertex : readyVertices)
        {
            auto& pass = _def.getPass(vertex);
            auto group = pass.getGroup();
            auto main = group == mainGroup;
            if (!main)
            {
                // we're in a child graph, set the group to the graph id
                res.setGroup(group);
            }
            pass(res);
            executed.insert(vertex);
            if (!main)
            {
                res.setGroup();
            }
        }

        return execute(res, executed);
    }

    std::string RenderGraph::getMatrixDebugInfo(const Matrix& mtx) noexcept
    {
        std::ostringstream ss;

        for (auto&& vertex : mtx.vertices())
        {
            ss << "vertex:" << vertex << std::endl;
            ss << "  in edges:";
            for (auto [lhs, rhs] : mtx.in_edges(vertex))
            {
                ss << "    " << lhs << " -> " << rhs << std::endl;
            }
            ss << std::endl;
            ss << "  out edges:";
            for (auto [lhs, rhs] : mtx.out_edges(vertex))
            {
                ss << "    " << lhs << " -> " << rhs << std::endl;
            }
            ss << std::endl;
        }
        return ss.str();
    }

    void RenderGraph::writeGraphviz(std::ostream& out) const
    {
        entt::dot(out, _matrix, [this, &out](auto& output, auto vertex)
        {
            auto& pass = _def.getPass(vertex);
            out << "label=\"" << pass.getName() << "\",shape=\"box\"";
        });
    }
}