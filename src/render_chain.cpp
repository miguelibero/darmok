#include <darmok/render_chain.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	TextureConfig RenderTexture::createColorConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::RGBA16F;
		return config;
	}

	TextureConfig RenderTexture::createDepthConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::D16F;
		return config;
	}

	RenderTexture::RenderTexture(const glm::uvec2& size) noexcept
		: _colorTex(std::make_shared<Texture>(createColorConfig(size), BGFX_TEXTURE_RT))
		, _depthTex(std::make_shared<Texture>(createDepthConfig(size), BGFX_TEXTURE_RT))
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles =
			{ _colorTex->getHandle(), _depthTex->getHandle() };
		_handle = bgfx::createFrameBuffer(handles.size(), &handles.front());
	}

	RenderTexture::~RenderTexture() noexcept
	{
		bgfx::destroy(_handle);
	}

	const std::shared_ptr<Texture>& RenderTexture::getTexture() const noexcept
	{
		return _colorTex;
	}

	const std::shared_ptr<Texture>& RenderTexture::getDepthTexture() const noexcept
	{
		return _depthTex;
	}

	const bgfx::FrameBufferHandle& RenderTexture::getHandle() const noexcept
	{
		return _handle;
	}

	void RenderTexture::configureView(bgfx::ViewId viewId) const noexcept
	{
		bgfx::setViewFrameBuffer(viewId, _handle);
	}

	RenderGraphDefinition& RenderChain::getRenderGraph()
	{
		return _graph.value();
	}

	const RenderGraphDefinition& RenderChain::getRenderGraph() const
	{
		return _graph.value();
	}

	RenderChain& RenderChain::setRenderTexture(const std::shared_ptr<RenderTexture>& renderTex) noexcept
	{
		_endTexture = renderTex;
		return *this;
	}

	std::shared_ptr<RenderTexture> RenderChain::getRenderTexture() const noexcept
	{
		return _endTexture;
	}

	RenderChain& RenderChain::setViewport(const Viewport& vp) noexcept
	{
		if (_viewport != vp)
		{
			_viewport = vp;
			updateTextures();
		}
		return *this;
	}

	const Viewport& RenderChain::getViewport() const noexcept
	{
		return _viewport;
	}

	void RenderChain::init(RenderGraphDefinition& graph, OptionalRef<RenderChain> parent)
	{
		_graph = graph;
		_parent = parent;
		for (auto& step : _steps)
		{
			step->init(*this);
		}
		updateSteps();
	}

	void RenderChain::renderReset()
	{
		for (auto& step : _steps)
		{
			step->renderReset();
		}
	}

	void RenderChain::updateTextures()
	{
		auto amount = _textures.size();
		_textures.clear();
		for (size_t i = 0; i < amount; ++i)
		{
			addTexture();
		}
		updateSteps();
	}

	void RenderChain::updateSteps()
	{
		if (!_graph)
		{
			return;
		}
		for (size_t i = 0; i < _steps.size(); ++i)
		{
			updateStep(i);
		}
	}

	bool RenderChain::updateStep(size_t i)
	{
		if (i >= _textures.size() || i >= _steps.size())
		{
			return false;
		}
		auto& read = *_textures[i];
		OptionalRef<RenderTexture> write;
		auto j = i + 1;
		if (j < _textures.size())
		{
			write = *_textures[j];
		}
		else if (_endTexture)
		{
			write = *_endTexture;
		}
		else if (_parent)
		{
			write = _parent->getFirstTexture();
		}
		_steps[i]->updateRenderChain(read, write);
		return true;
	}

	void RenderChain::shutdown()
	{
		for (auto itr = _steps.rbegin(); itr != _steps.rend(); ++itr)
		{
			(*itr)->shutdown();
		}
		_graph.reset();
	}

	OptionalRef<RenderTexture> RenderChain::getFirstTexture() noexcept
	{
		if (!_textures.empty())
		{
			return *_textures.front();
		}
		if (_parent)
		{
			return _parent->getFirstTexture();
		}
		return nullptr;
	}

	OptionalRef<const RenderTexture> RenderChain::getFirstTexture() const noexcept
	{
		if (!_textures.empty())
		{
			return *_textures.front();
		}
		if (_parent)
		{
			return _parent->getFirstTexture();
		}
		return nullptr;
	}

	void RenderChain::configureView(bgfx::ViewId viewId, OptionalRef<const RenderTexture> write) const
	{
		uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		if (write)
		{
			clearFlags |= BGFX_CLEAR_COLOR;
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 0);
			write->configureView(viewId);
		}
		else
		{
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
		}
		_viewport.configureView(viewId);
	}

	void RenderChain::addTexture() noexcept
	{
		auto size = _viewport.origin + _viewport.size;
		_textures.emplace_back(std::make_unique<RenderTexture>(size));
	}

	RenderChain& RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		addTexture();
		auto& ref = *step;
		_steps.push_back(std::move(step));
		if (_graph)
		{
			ref.init(*this);
			updateStep(_steps.size() - 1);
		}

		return *this;
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name, int priority)
		: _program(prog)
		, _name(name)
		, _priority(priority)
		, _texUniform{ bgfx::kInvalidHandle }
	{
		if (!prog)
		{
			throw std::runtime_error("empty program");
		}
		if (_name.empty())
		{
			_name = "ScreenSpaceRenderPass " + prog->getName();
		}
	}

	ScreenSpaceRenderPass::~ScreenSpaceRenderPass() noexcept
	{
		// empty on purpose
	}

	void ScreenSpaceRenderPass::init(RenderChain& chain) noexcept
	{
		_chain = chain;
		_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		Rectangle screen(glm::uvec2(2));
		_mesh = MeshData(screen).createMesh(_program->getVertexLayout());

		renderReset();
	}

	void ScreenSpaceRenderPass::updateRenderChain(RenderTexture& read, OptionalRef<RenderTexture> write) noexcept
	{
		_readTex = read;
		_writeTex = write;
		if (write && _viewId)
		{
			write->configureView(_viewId.value());
		}
	}

	void ScreenSpaceRenderPass::renderReset() noexcept
	{
		if (_chain)
		{
			_chain->getRenderGraph().addPass(*this);
		}
	}

	void ScreenSpaceRenderPass::shutdown() noexcept
	{
		if (isValid(_texUniform))
		{
			bgfx::destroy(_texUniform);
			_texUniform.idx = bgfx::kInvalidHandle;
		}
		_viewId.reset();
		_mesh.reset();
		if (_chain)
		{
			_chain->getRenderGraph().removePass(*this);
		}
		_chain.reset();
	}

	void ScreenSpaceRenderPass::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName(_name);
		def.setPriority(_priority);
		def.setSync(true);
	}

	void ScreenSpaceRenderPass::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		if (_chain)
		{
			_chain->configureView(viewId, _writeTex);
		}
		_viewId = viewId;
	}

	void ScreenSpaceRenderPass::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		auto& encoder = context.getEncoder();
		auto viewId = context.getViewId();

		if (!_mesh || !_program || !_readTex)
		{
			encoder.touch(viewId);
			return;
		}

		_mesh->render(encoder);

		encoder.setTexture(0, _texUniform, _readTex->getTexture()->getHandle());

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_BLEND_ALPHA
			;
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}
}