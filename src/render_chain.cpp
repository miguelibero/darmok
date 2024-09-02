#include <darmok/render_chain.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	TextureConfig FrameBuffer::createColorConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::RGBA16F;
		return config;
	}

	TextureConfig FrameBuffer::createDepthConfig(const glm::uvec2& size) noexcept
	{
		TextureConfig config;
		config.size = size;
		config.format = bgfx::TextureFormat::D16F;
		return config;
	}

	FrameBuffer::FrameBuffer(const glm::uvec2& size) noexcept
		: _colorTex(std::make_shared<Texture>(createColorConfig(size), BGFX_TEXTURE_RT))
		, _depthTex(std::make_shared<Texture>(createDepthConfig(size), BGFX_TEXTURE_RT))
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles =
			{ _colorTex->getHandle(), _depthTex->getHandle() };
		_handle = bgfx::createFrameBuffer(handles.size(), &handles.front());
	}

	FrameBuffer::~FrameBuffer() noexcept
	{
		bgfx::destroy(_handle);
	}

	const std::shared_ptr<Texture>& FrameBuffer::getTexture() const noexcept
	{
		return _colorTex;
	}

	const std::shared_ptr<Texture>& FrameBuffer::getDepthTexture() const noexcept
	{
		return _depthTex;
	}

	const bgfx::FrameBufferHandle& FrameBuffer::getHandle() const noexcept
	{
		return _handle;
	}

	const glm::uvec2& FrameBuffer::getSize() const noexcept
	{
		return _colorTex->getSize();
	}

	void FrameBuffer::configureView(bgfx::ViewId viewId) const noexcept
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

	RenderChain& RenderChain::setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept
	{
		_output = fb;
		return *this;
	}

	std::shared_ptr<FrameBuffer> RenderChain::getOutput() const noexcept
	{
		return _output;
	}

	RenderChain& RenderChain::setViewport(const Viewport& vp) noexcept
	{
		if (_viewport != vp)
		{
			_viewport = vp;
			updateBuffers();
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

	void RenderChain::updateBuffers()
	{
		auto amount = _buffers.size();
		_buffers.clear();
		for (size_t i = 0; i < amount; ++i)
		{
			addBuffer();
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
		if (i >= _buffers.size() || i >= _steps.size())
		{
			return false;
		}
		auto& read = *_buffers[i];
		OptionalRef<FrameBuffer> write;
		auto j = i + 1;
		if (j < _buffers.size())
		{
			write = *_buffers[j];
		}
		else if (_output)
		{
			write = *_output;
		}
		else if (_parent)
		{
			write = _parent->getInput();
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

	OptionalRef<FrameBuffer> RenderChain::getInput() noexcept
	{
		if (!_buffers.empty())
		{
			return *_buffers.front();
		}
		if (_output)
		{
			return *_output;
		}
		if (_parent)
		{
			return _parent->getInput();
		}
		return nullptr;
	}

	OptionalRef<const FrameBuffer> RenderChain::getInput() const noexcept
	{
		if (!_buffers.empty())
		{
			return *_buffers.front();
		}
		if (_output)
		{
			return *_output;
		}
		if (_parent)
		{
			return _parent->getInput();
		}
		return nullptr;
	}

	void RenderChain::configureView(bgfx::ViewId viewId, OptionalRef<const FrameBuffer> writeBuffer) const
	{
		uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		if (writeBuffer)
		{
			clearFlags |= BGFX_CLEAR_COLOR;
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 0);
			writeBuffer->configureView(viewId);
		}
		else
		{
			bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
		}
		_viewport.configureView(viewId);
	}

	FrameBuffer& RenderChain::addBuffer() noexcept
	{
		auto size = _viewport.origin + _viewport.size;
		return *_buffers.emplace_back(std::make_unique<FrameBuffer>(size));
	}

	RenderChain& RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		addBuffer();
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

	void ScreenSpaceRenderPass::updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept
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