#pragma once

#include <bgfx/embedded_shader.h>

// fix for bgfx saying that linux supports DirectX shaders

#undef BGFX_PLATFORM_SUPPORTS_DXBC
#define BGFX_PLATFORM_SUPPORTS_DXBC (0  \
	|| BX_PLATFORM_WINDOWS              \
	|| BX_PLATFORM_WINRT                \
	|| BX_PLATFORM_XBOXONE              \
	)


#if !BGFX_PLATFORM_SUPPORTS_DXBC
#undef BGFX_EMBEDDED_SHADER_DXBC
#define BGFX_EMBEDDED_SHADER_DXBC(...)
#endif


#undef BGFX_EMBEDDED_SHADER
#define BGFX_EMBEDDED_SHADER(_name)                                                        \
	{                                                                                      \
		#_name,                                                                            \
		{                                                                                  \
			BGFX_EMBEDDED_SHADER_PSSL (bgfx::RendererType::Agc,        _name)              \
			BGFX_EMBEDDED_SHADER_DXBC (bgfx::RendererType::Direct3D11, _name)              \
			BGFX_EMBEDDED_SHADER_DXBC (bgfx::RendererType::Direct3D12, _name)              \
			BGFX_EMBEDDED_SHADER_PSSL (bgfx::RendererType::Gnm,        _name)              \
			BGFX_EMBEDDED_SHADER_METAL(bgfx::RendererType::Metal,      _name)              \
			BGFX_EMBEDDED_SHADER_NVN  (bgfx::RendererType::Nvn,        _name)              \
			BGFX_EMBEDDED_SHADER_ESSL (bgfx::RendererType::OpenGLES,   _name)              \
			BGFX_EMBEDDED_SHADER_GLSL (bgfx::RendererType::OpenGL,     _name)              \
			BGFX_EMBEDDED_SHADER_SPIRV(bgfx::RendererType::Vulkan,     _name)              \
			{ bgfx::RendererType::Noop,  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10 }, \
			{ bgfx::RendererType::Count, NULL, 0 }                                         \
		}                                                                                  \
	}
