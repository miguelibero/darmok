#include <darmok/render_chain.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	TextureConfig RenderTexture::createColorConfig(const Viewport& vp) noexcept
	{
		TextureConfig config;
		config.size = vp.origin + vp.size;
		config.format = bgfx::TextureFormat::RGBA16F;
		return config;
	}

	TextureConfig RenderTexture::createDepthConfig(const Viewport& vp) noexcept
	{
		auto config = createColorConfig(vp);
		config.format = bgfx::TextureFormat::D16F;
		return config;
	}

	RenderTexture::RenderTexture(const Viewport& vp) noexcept
		: _colorTex(createColorConfig(vp), BGFX_TEXTURE_RT)
		, _depthTex(createDepthConfig(vp), BGFX_TEXTURE_RT)
		, _viewport(vp)
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

	const Viewport& RenderTexture::getViewport() const noexcept
	{
		return _viewport;
	}

	void RenderTexture::configureView(bgfx::ViewId viewId) const noexcept
	{
		static const uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 1);
		bgfx::setViewFrameBuffer(viewId, _handle);
		_viewport.configureView(viewId);

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
			step->init(graph);
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

	void RenderChain::configureView(bgfx::ViewId viewId) const noexcept
	{
		if (_textures.empty())
		{
			return;
		}
		_textures.front()->configureView(viewId);
	}

	void RenderChain::addTexture() noexcept
	{
		_textures.emplace_back(std::make_unique<RenderTexture>(_viewport));
	}

	RenderChain& RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		addTexture();
		auto& ref = *step;
		_steps.push_back(std::move(step));
		if (_graph)
		{
			ref.init(_graph.value());
			updateStep(_steps.size() - 1);
		}

		return *this;
	}

	ScreenSpaceRenderPass::ScreenSpaceRenderPass() noexcept
		: _priority(0)
		, _texUniform{ bgfx::kInvalidHandle }
	{
	}

	void ScreenSpaceRenderPass::init(RenderGraphDefinition& graph) noexcept
	{
		_graph = graph;
		_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		renderReset();
	}

	void ScreenSpaceRenderPass::updateRenderChain(RenderTexture& read, OptionalRef<RenderTexture> write) noexcept
	{
		_readTex = read;
		_writeTex = write;
	}

	void ScreenSpaceRenderPass::renderReset() noexcept
	{
		if (_graph)
		{
			_graph->addPass(*this);
		}
	}

	void ScreenSpaceRenderPass::shutdown() noexcept
	{
		if (isValid(_texUniform))
		{
			bgfx::destroy(_texUniform);
			_texUniform.idx = bgfx::kInvalidHandle;
		}
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
		if (_writeTex)
		{
			_writeTex->configureView(viewId);
		}
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

		encoder.setTexture(0, _texUniform, _readTex->getTexture().getHandle());
		_mesh->render(encoder);

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			;
		encoder.submit(viewId, _program->getHandle(), state);
	}
}