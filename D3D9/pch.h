#pragma once

#include <vector>
#include <list>
#include <memory>

#include <wrl/client.h>
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#define ASSERT_UNTESTED() assert(false && "Untested code path")
#define ASSERT_UNIMPLEMENTED() assert(false && "Feature unimplemented")