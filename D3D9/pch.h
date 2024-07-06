#pragma once

// STL
#include <vector>
#include <list>
#include <memory>

// WRL
#include <wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// External
#include <spdlog/spdlog.h>

#define ASSERT_UNTESTED() assert(false && "Untested code path")
#define ASSERT_UNIMPLEMENTED() assert(false && "Feature unimplemented")

#define CheckHresult(expr) \
	{ \
		HRESULT _hr = expr; \
		if (FAILED(_hr)) \
		{ \
			spdlog::error("Error: {} failed with error code {}", #expr, _hr); \
			return; \
		} \
	}

#define CheckHresultReturn(expr) \
	{ \
		HRESULT _hr = expr; \
		if (FAILED(_hr)) \
		{ \
			spdlog::error("Error: {} failed with error code {}", #expr, _hr); \
			return _hr; \
		} \
	}

#define CheckHresultReturnBool(expr) \
	{ \
		HRESULT _hr = expr; \
		if (FAILED(_hr)) \
		{ \
			spdlog::error("Error: {} failed with error code {}", #expr, _hr); \
			return GL_FALSE; \
		} \
	}