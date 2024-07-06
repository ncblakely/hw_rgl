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
	hr = expr; \
	if (FAILED(hr)) \
	{ \
		spdlog::error("Error: {} failed with error code {}", #expr, hr); \
		return; \
	}

#define CheckHresultReturn(expr) \
	hr = expr; \
	if (FAILED(hr)) \
	{ \
		spdlog::error("Error: {} failed with error code {}", #expr, hr); \
		return hr; \
	}

#define CheckHresultReturnBool(expr) \
	hr = expr; \
	if (FAILED(hr)) \
	{ \
		spdlog::error("Error: {} failed with error code {}", #expr, hr); \
		return GL_FALSE; \
	}
