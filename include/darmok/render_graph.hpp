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
        using Resource = RenderResourceDefinition;
        using ConstIterator = std::vector<Resource>::const_iterator;
        ConstIterator begin() const;
        ConstIterator end() const;
        bool contains(const Resource& res) const noexcept;


        template<typename T>
        RenderResourceGroupDefinition& add(std::string_view name = "")
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
            return add<T>("");
        }

        template<typename T>
        RenderResourceGroupDefinition& add(const OptionalRef<const T>& value)
        {
            return add<T>("");
        }

        template<typename T>
        RenderResourceGroupDefinition& add(const OptionalRef<T>& value)
        {
            return add<T>("");
        }

    private:
        std::vector<Resource> _resources;
    };

    class DARMOK_EXPORT RenderGraphResources final
    {
    public:
        template<typename T>
        RenderGraphResources& set(std::string_view name, T&& value) noexcept
        {
            _data[getKey<T>(name)] = std::forward(value);
            return *this;
        }

        template<typename T>
        RenderGraphResources& setRef(std::string_view name, const T& value) noexcept
        {
            _data[getKey<T>(name)] = std::reference_wrapper<const T>(value);
            return *this;
        }

        template<typename T>
        RenderGraphResources& set(T&& value) noexcept
        {
            return set("", std::forward(value));
        }

        template<typename T>
        RenderGraphResources& setRef(const T& value) noexcept
        {
            return setRef("", value);
        }

        template<typename T>
        void get(std::string_view name, T& ref) const noexcept
        {
            ref = get<T>(name).value();
        }

        template<typename T>
        void get(T& ref) const noexcept
        {
            ref = get<T>().value();
        }

        template<typename T>
        void get(std::string_view name, OptionalRef<const T>& ref) const noexcept
        {
            ref = get<T>(name);
        }

        template<typename T>
        void get(OptionalRef<const T>& ref) const noexcept
        {
            ref = get<T>();
        }

        template<typename T>
        OptionalRef<const T> get(std::string_view name = "") const noexcept
        {
            auto itr = _data.find(getKey<T>(name));
            if (itr == _data.end())
            {
                return std::nullopt;
            }
            auto ptr = &itr->second;
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

    public:
        std::unordered_map<entt::id_type, entt::any> _data;

        template<typename T>
        static entt::id_type getKey(std::string_view name) noexcept
        {
            return RenderResourceDefinition::create<T>(name).hash();
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