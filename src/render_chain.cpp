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
		: _colorTex(createColorConfig(size), BGFX_TEXTURE_RT)
		, _depthTex(createDepthConfig(size), BGFX_TEXTURE_RT)
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles =
			{ _colorTex.getHandle(), _depthTex.getHandle() };
		_handle = bgfx::createFrameBuffer(handles.size(), &handles.front());
	}

	RenderTexture::~RenderTexture() noexcept
	{
		bgfx::destroy(_handle);
	}

	const Texture& RenderTexture::getTexture() const noexcept
	{
		return _colorTex;
	}

	const Texture& RenderTexture::getDepthTexture() const noexcept
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

	void RenderChain::init(RenderGraphDefinition& graph)
	{
		_graph = graph;
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
		_steps[i]->updateRenderChain(read, write);
		return true;
	}

	void RenderChain::shutdown()
	{
		_graph.reset();
		for (auto itr = _steps.rbegin(); itr != _steps.rend(); ++itr)
		{
			(*itr)->shutdown();
		}
	}


	OptionalRef<RenderTexture> RenderChain::getFirstTexture() noexcept
	{
		if (_textures.empty())
		{
			return nullptr;
		}
		return *_textures.front();
	}

	OptionalRef<const RenderTexture> RenderChain::getFirstTexture() const noexcept
	{
		if (_textures.empty())
		{
			return nullptr;
		}
		return *_textures.front();
	}

	void RenderChain::configureView(bgfx::ViewId viewId) const noexcept
	{
		static const uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 1);
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
		ref.init(*this);
		updateStep(_steps.size() - 1);

		return *this;
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass() noexcept
		: _priority(0)
		, _texUniform{ bgfx::kInvalidHandle }
		, _viewId(-1)
	{
	}

	void ScreenSpaceRenderPass::init(RenderChain& chain) noexcept
	{
		_chain = chain;
		_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		renderReset();
	}

	void ScreenSpaceRenderPass::updateRenderChain(RenderTexture& read, OptionalRef<RenderTexture> write) noexcept
	{
		_readTex = read;
		_writeTex = write;
		if (write && _viewId != -1)
		{
			write->configureView(_viewId);
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
		_viewId = -1;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setName(const std::string& name) noexcept
	{
		_name = name;
		return *this;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setPriority(int priority)
	{
		_priority = priority;
		return *this;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setProgram(const std::shared_ptr<Program>& prog) noexcept
	{
		_program = prog;
		Rectangle screen(glm::uvec2(2));
		_mesh = MeshData(screen).createMesh(prog->getVertexLayout());
		return *this;
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
			_chain->configureView(viewId);
		}
		if (_writeTex)
		{
			_writeTex->configureView(viewId);
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

		encoder.setTexture(0, _texUniform, _readTex->getTexture().getHandle());

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			;
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}
}