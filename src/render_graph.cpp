#include <darmok/render_graph.hpp>
#include <darmok/utils.hpp>
#include <stdexcept>

namespace darmok
{
    size_t RenderResourceDefinition::hash() const noexcept
    {
        std::size_t hash;
        hash_combine(hash, name, type);
        return hash;
    }

    RenderResourceGroupDefinition::ConstIterator RenderResourceGroupDefinition::begin() const
    {
        return _resources.begin();
    }

    RenderResourceGroupDefinition::ConstIterator RenderResourceGroupDefinition::end() const
    {
        return _resources.end();
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

    RenderPassDefinition& RenderGraph::addPass(const std::string& name)
    {
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
            builder.bind(entt::hashed_string(pass.getName().c_str()));
            for (auto& input : pass.getInputs())
            {
                builder.ro(input.hash());
            }
            for (auto& output : pass.getOutputs())
            {
                builder.rw(output.hash());
            }
        }
        _matrix = builder.graph();
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
        auto& mtx = _matrix.value();
        for (auto&& vertex : mtx.vertices())
        {
            if (auto in_edges = mtx.in_edges(vertex); in_edges.begin() == in_edges.end())
            {
                std::cout << "vertex:" << vertex << std::endl;
            }
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
            out << "label=\"v\"" << vertex << ",shape=\"box\"";
        });
    }
}