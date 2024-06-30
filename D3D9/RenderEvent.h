#pragma once

class RenderEventTracker
{
public:
	explicit RenderEventTracker() = delete;

	RenderEventTracker(RenderEventTracker&& other) = delete;
	RenderEventTracker(const RenderEventTracker& other) = delete;

	RenderEventTracker& operator=(const RenderEventTracker& other) = delete;
	RenderEventTracker& operator=(const RenderEventTracker&& other) = delete;

	RenderEventTracker(const wchar_t* eventName)
	{
		D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0, 0, 1.0f), eventName);
	}

	virtual ~RenderEventTracker()
	{
		D3DPERF_EndEvent();
	}
};

#if defined (_DEBUG) | defined (ENABLE_GPU_PROFILING)
#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)
#define WFUNCTION WIDE1(__FUNCTION__)

inline std::unique_ptr<RenderEventTracker> BeginRenderEvent(const wchar_t* name)
{
	return std::make_unique<RenderEventTracker>(name);
}

inline void SetRenderEventMarker(const wchar_t* name)
{
	D3DPERF_SetMarker(D3DCOLOR_COLORVALUE(1.0f, 0, 0, 1.0f), name);
}

#define RETrackFunction() auto __tracker##__LINE__ = (BeginRenderEvent(WFUNCTION))
#define RETrackEvent(n) auto __tracker##__LINE__ = (BeginRenderEvent(n))
#define RESetMarker(n) (SetRenderEventMarker(n))
#else
#define RETrackFunction()
#define RETrackEvent(n)
#define RESetMarker(n)
#endif 