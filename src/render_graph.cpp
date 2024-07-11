#include <darmok/render_graph.hpp>
#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <stdexcept>

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

    OptionalRef<IRenderPassDelegate> RenderPassDefinition::getDelegate() const noexcept
    {
        return _delegate;
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

    RenderPassDefinition& RenderGraph::addPass(const std::string& name)
    {
        if (_matrix)
        {
            throw std::runtime_error("render graph already compiled");
        }
        auto itr = std::find_if(_passes.begin(), _passes.end(), [&name](auto& pass) { return pass.getName() == name; });
        if (itr != _passes.end())
        {
            throw std::invalid_argument("pass name already exists");
        }
        return _passes.emplace_back(name);
    }

    const RenderPassDefinition& RenderGraph::addPass(IRenderPass& pass)
    {
        auto& def = addPass(pass.getRenderPassName());
        def.setDelegate(pass);
        pass.onRenderPassDefine(def);
        return def;
    }

    const RenderGraph::Matrix& RenderGraph::compile()
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
        _matrix = builder.graph();

        _passMap.clear();
        for (size_t i = 0; i < builder.size(); i++)
        {
            auto hash = builder[i];
            auto itr = std::find_if(_passes.begin(), _passes.end(), [&hash](auto& pass) { return pass.hash() == hash; });
            _passMap.push_back(std::distance(_passes.begin(), itr));
        }
        return _matrix.value();
    }

    RenderGraph::Resources RenderGraph::execute() const
    {
        Resources res;
        execute(res);
        return res;
    }

    void RenderGraph::execute(Resources& res) const
    {
        if (!_matrix)
        {
            throw std::runtime_error("render graph needs to be compiled first");
        }
        std::cout << StringUtils::join(", ", _passMap) << std::endl;
        auto& mtx = _matrix.value();
        for (auto&& vertex : mtx.vertices())
        {
            std::cout << "vertex:" << vertex << std::endl;
            std::cout << "    in edges:";
            for (auto [lhs, rhs] : mtx.in_edges(vertex))
            {
                std::cout << lhs << " -> " << rhs << ", ";
            }
            std::cout << std::endl;
            std::cout << "    out edges:";
            for (auto [lhs, rhs] : mtx.out_edges(vertex))
            {
                std::cout << lhs << " -> " << rhs << ", ";
            }
            std::cout << std::endl;
        }
    }

    void RenderGraph::writeGraphviz(std::ostream& out) const
    {
        if (!_matrix)
        {
            throw std::runtime_error("render graph needs to be compiled first");
        }
        entt::dot(out, _matrix.value(), [this, &out](auto& output, auto vertex)
        {
            // out << "label=\"v\"" << vertex << ",shape=\"box\"";
        });
    }
}