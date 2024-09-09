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

	FrameBuffer::FrameBuffer(const glm::uvec2& size, bool depth) noexcept
		: _colorTex(std::make_shared<Texture>(createColorConfig(size), BGFX_TEXTURE_RT))
		, _depthTex(depth ? std::make_shared<Texture>(createDepthConfig(size), BGFX_TEXTURE_RT) : nullptr)
		, _handle{ bgfx::kInvalidHandle }
	{
		std::vector<bgfx::TextureHandle> handles = { _colorTex->getHandle() };
		if (_depthTex)
		{
			handles.push_back(_depthTex->getHandle());
		}
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

	RenderChain::RenderChain(IRenderChainDelegate& dlg) noexcept
		: _delegate(dlg)
		, _running(false)
	{
	}

	void RenderChain::init(const std::string& name, int priority)
	{
		_renderGraph.setName(name);
		_renderGraph.setPriority(priority);
		_running = true;
		for (size_t i = 0; i < _steps.size(); ++i)
		{
			auto& step = _steps[i];
			step->init(*this);
			step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
		}
		auto& parentGraph = _delegate.getRenderChainParentGraph();
		parentGraph.addPass(*this);
		parentGraph.setChild(_renderGraph);
	}

	void RenderChain::shutdown()
	{
		for (auto itr = _steps.rbegin(); itr != _steps.rend(); ++itr)
		{
			(*itr)->shutdown();
		}
		getRenderGraph().removePass(*this);
		_running = false;
	}

	RenderChain& RenderChain::setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept
	{
		if (_output == fb)
		{
			return *this;
		}
		_output = fb;
		auto size = _steps.size();
		if (size > 0)
		{
			auto i = size - 1;
			auto& lastStep = _steps.at(i);
			lastStep->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
		}
		return *this;
	}

	std::shared_ptr<FrameBuffer> RenderChain::getOutput() const noexcept
	{
		return _output;
	}

	void RenderChain::renderReset()
	{
		auto& parentGraph = _delegate.getRenderChainParentGraph();
		parentGraph.addPass(*this);
		_renderGraph.clear();

		auto amount = _buffers.size();
		_buffers.clear();
		for (size_t i = 0; i < amount; ++i)
		{
			addBuffer();
		}
		for (size_t i = 0; i < _steps.size(); ++i)
		{
			auto& step = _steps[i];
			step->updateRenderChain(getReadBuffer(i).value(), getWriteBuffer(i));
			step->renderReset();
		}

		parentGraph.setChild(_renderGraph);
	}

	OptionalRef<FrameBuffer> RenderChain::getReadBuffer(size_t i) const noexcept
	{
		if (i < 0 || i >= _buffers.size())
		{
			return std::nullopt;
		}
		return *_buffers[i];
	}

	OptionalRef<FrameBuffer> RenderChain::getWriteBuffer(size_t i) const noexcept
	{
		if (i < 0 || i >= _buffers.size())
		{
			return std::nullopt;
		}
		if (i == _buffers.size() - 1)
		{
			return getOutput().get();
		}
		return *_buffers[i + 1];
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
		if (auto parent = _delegate.getRenderChainParent())
		{
			return parent->getInput();
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
		if (auto parent = _delegate.getRenderChainParent())
		{
			return parent->getInput();
		}
		return nullptr;
	}

	void RenderChain::configureView(bgfx::ViewId viewId, OptionalRef<const FrameBuffer> writeBuffer) const
	{
		uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
		if (writeBuffer)
		{
			writeBuffer->configureView(viewId);
		}
		auto vp = _delegate.getRenderChainViewport();
		vp.configureView(viewId);
	}

	RenderGraphDefinition& RenderChain::getRenderGraph()
	{
		return _renderGraph;
	}

	const RenderGraphDefinition& RenderChain::getRenderGraph() const
	{
		return _renderGraph;
	}

	FrameBuffer& RenderChain::addBuffer() noexcept
	{
		auto vp = _delegate.getRenderChainViewport();
		auto size = vp.origin + vp.size;
		return *_buffers.emplace_back(std::make_unique<FrameBuffer>(size));
	}

	RenderChain& RenderChain::addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept
	{
		auto& readBuffer = addBuffer();
		auto& ref = *step;
		auto i = _steps.size();
		_steps.push_back(std::move(step));
		if (!_running)
		{
			return *this;
		}

		ref.init(*this);
		ref.updateRenderChain(readBuffer, getWriteBuffer(i));

		auto& parentGraph = _delegate.getRenderChainParentGraph();
		parentGraph.setChild(_renderGraph);

		if (i > 0)
		{
			auto j = i - 1;
			auto& prevStep = _steps[j];
			prevStep->updateRenderChain(getReadBuffer(j).value(), getWriteBuffer(j));
		}
		else
		{
			_delegate.onRenderChainInputChanged();
		}
		return *this;
	}

	void RenderChain::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		auto name = getRenderGraph().getName();
		if (!name.empty())
		{
			name += " ";
		}
		def.setName(name + "clear");
		def.setPriority(RenderPassDefinition::kMaxPriority);
	}

	void RenderChain::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL | BGFX_CLEAR_COLOR;
		uint8_t clearColor = 1;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U,
			clearColor, clearColor, clearColor, clearColor,
			clearColor, clearColor, clearColor, clearColor);
		
		if (auto input = getInput())
		{
			input->configureView(viewId);
		}
		auto vp = _delegate.getRenderChainViewport();
		vp.configureView(viewId);
	}

	void RenderChain::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		if (!_buffers.empty() || _output)
		{
			context.getEncoder().touch(context.getViewId());
		}
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

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept
	{
		_textureUniforms.set(name, stage, texture);
		return *this;
	}

	ScreenSpaceRenderPass& ScreenSpaceRenderPass::setUniform(const std::string& name, std::optional<UniformValue> value) noexcept
	{
		_uniforms.set(name, value);
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
		_uniforms.configure(encoder);
		_textureUniforms.configure(encoder);

		uint64_t state = 0
			| BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_BLEND_ALPHA
			;
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}
}