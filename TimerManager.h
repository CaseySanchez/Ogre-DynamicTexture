#pragma once

#include "stdafx.h"

#include <Windows.h>
#include <list>

#include "Singleton.h"

/* PARAMS: None */
#define TIMER_MEMBER_HDR(NAME) static void NAME(void *params); void _##NAME()
/* PARAMS: None */
#define TIMER_MEMBER_SRC(CLASS, NAME) void CLASS##::##NAME(void *params) { ((CLASS *)params)->_##NAME(); } void CLASS##::##_##NAME()
/* PARAMS: None */
#define TIMER_MEMBER_INL(CLASS, NAME) static void NAME(void *params) { ((CLASS *)params)->_##NAME(); } void _##NAME()

typedef void (*TimerCallback)(void *);

struct TimerData
{
	TimerCallback
		callback;
	void
		*params;
	DWORD
		interval,
		ticks;
	bool
		repeat,
		kill;
};

class TimerManager : public Singleton<TimerManager>
{
	std::list<TimerData *>
		timers;
	TimerData
		*current;

public:
	TimerManager();
	~TimerManager();

	TimerData *Set(TimerCallback callback, void *params, DWORD interval, bool repeat = false);
	void Kill(TimerData *timer = nullptr);

	void Update();
};

/* PARAMS: Ticker *ticker, DWORD ticks */
#define TICKER_MEMBER_HDR(NAME) static void NAME(Ticker *ticker, DWORD ticks, void *params); void _##NAME(Ticker *ticker, DWORD ticks)
/* PARAMS: Ticker *ticker, DWORD ticks */
#define TICKER_MEMBER_SRC(CLASS, NAME) void CLASS##::##NAME(Ticker *ticker, DWORD ticks, void *params) { ((CLASS *)params)->_##NAME(ticker, ticks); } void CLASS##::##_##NAME(Ticker *ticker, DWORD ticks)
/* PARAMS: Ticker *ticker, DWORD ticks */
#define TICKER_MEMBER_INL(CLASS, NAME) static void NAME(Ticker *ticker, DWORD ticks, void *params) { ((CLASS *)params)->_##NAME(ticker, ticks); } void _##NAME(Ticker *ticker, DWORD ticks)

class Ticker
{
	typedef void (*TickerCallback)(Ticker *, DWORD, void *);

	TimerData
		*timer;
	DWORD
		ticks;
	TickerCallback
		callback;
	void
		*params;

public:
	Ticker(TickerCallback callback);
	~Ticker();

	void Start(DWORD rate, void *params = nullptr);
	void Stop();

	DWORD GetTicks();

	TIMER_MEMBER_HDR(Tick);
};

class Counter
{
	DWORD
		ticks;

public:
	Counter();
	~Counter();
	
	void Reset();
	DWORD Elapsed();
};