#include <darmok/render_graph.hpp>
#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <stdexcept>
#include <sstream>

namespace darmok
{
    entt::id_type RenderResourceDefinition::hash() const noexcept
    {
        std::size_t hash;
        hash_combine(hash, name, type);
        return hash;
    }

    bool RenderResourceDefinition::operator==(const RenderResourceDefinition& other) const noexcept
    {
        return hash() == other.hash();
    }

    bool RenderResourceDefinition::operator!=(const RenderResourceDefinition& other) const noexcept
    {
        return !operator==(other);
    }

    bool RenderResourceDefinition::operator<(const RenderResourceDefinition& other) const noexcept
    {
        return hash() < other.hash();
    }

    RenderResourceGroupDefinition::ConstIterator RenderResourceGroupDefinition::begin() const
    {
        return _resources.begin();
    }

    RenderResourceGroupDefinition::ConstIterator RenderResourceGroupDefinition::end() const
    {
        return _resources.end();
    }

    bool RenderResourceGroupDefinition::contains(const Resource& res) const noexcept
    {
        return std::find(_resources.begin(), _resources.end(), res) != _resources.end();
    }

    RenderPassDefinition::RenderPassDefinition(const std::string& name) noexcept
        : _name(name)
    {
    }

    const std::string& RenderPassDefinition::getName() const noexcept
    {
        return _name;
    }

    RenderPassDefinition& RenderPassDefinition::setDelegate(IRenderPassDelegate& dlg) noexcept
    {
        _delegate = dlg;
        return *this;
    }

    bool RenderPassDefinition::operator()(RenderGraphResources& res) const
    {
        if (!_delegate)
        {
            return false;
        }
        _delegate->renderPassExecute(res);
        return true;
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

    entt::id_type RenderPassDefinition::hash() const noexcept
    {
        return entt::hashed_string(_name.c_str());
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

    RenderPassDefinition& RenderGraphDefinition::addPass(const std::string& name)
    {
        auto itr = std::find_if(_passes.begin(), _passes.end(), [&name](auto& pass) { return pass.getName() == name; });
        if (itr != _passes.end())
        {
            throw std::invalid_argument("pass name already exists");
        }
        return _passes.emplace_back(name);
    }

    const RenderPassDefinition& RenderGraphDefinition::addPass(IRenderPass& pass)
    {
        auto& def = addPass(pass.getRenderPassName());
        def.setDelegate(pass);
        pass.renderPassDefine(def);
        return def;
    }

    const RenderPassDefinition& RenderGraphDefinition::getPass(size_t vertex) const
    {
        if (vertex < 0 || vertex >= _passes.size())
        {
            throw std::invalid_argument("pass does not exist");
        }
        return _passes[vertex];
    }

    RenderGraph RenderGraphDefinition::compile() const
    {
        entt::flow builder;
        for (auto& pass : _passes)
        {
            builder.bind(pass.hash());
            for (auto& input : pass.getInputs())
            {
                builder.ro(input.hash());
            }
            for (auto& output : pass.getOutputs())
            {
                builder.rw(output.hash());
            }
            if (pass.getSync())
            {
                builder.sync();
            }
        }
        return RenderGraph(builder.graph(), *this);
    }

    RenderGraph::RenderGraph(const Matrix& matrix, const Definition& def) noexcept
        : _matrix(matrix)
        , _def(def)
    {
    }

    RenderGraph::Resources RenderGraph::execute() const
    {
        Resources res;
        execute(res);
        return res;
    }

    void RenderGraph::execute(Resources& res) const
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
        for (auto& vertex : readyVertices)
        {
            // TODO: run passes in parallel
            auto& pass = _def.getPass(vertex);
            pass(res);
            executed.insert(vertex);
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