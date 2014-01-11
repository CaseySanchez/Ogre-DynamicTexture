#include "stdafx.h"

#include "TimerManager.h"

TimerManager::TimerManager()
{
	current = nullptr;
}

TimerManager::~TimerManager()
{
	for(std::list<TimerData *>::iterator it = timers.begin(); it != timers.end(); ++it)
	{
		delete (*it);
	}
}

TimerData *TimerManager::Set(TimerCallback callback, void *params, DWORD interval, bool repeat)
{
	TimerData
		*data = new TimerData;

	data->callback = callback;
	data->params = params;
	data->interval = interval;
	data->ticks = GetTickCount() + interval;
	data->repeat = repeat;
	data->kill = false;
		
	timers.push_back(data);

	return data;
}

void TimerManager::Kill(TimerData *timer)
{
	if(timer)
	{
		timer->kill = true;
	}
	else if(current)
	{
		current->kill = true;
	}
}

void TimerManager::Update()
{
	DWORD
		ticks = GetTickCount();
		
	for(std::list<TimerData *>::iterator it = timers.begin(); it != timers.end(); ++it)
	{
		if((*it)->ticks <= ticks)
		{
			current = (*it);

			(*it)->callback((*it)->params);
				
			if(!(*it)->kill && (*it)->repeat)
			{
				(*it)->ticks = ticks + (*it)->interval;
			}
			else
			{
				delete (*it);

				it = timers.erase(it);

				if(it == timers.end())
				{
					break;
				}
			}
		}
	}

	current = nullptr;
}

Ticker::Ticker(TickerCallback callback) : timer(nullptr), callback(callback)
{
}

Ticker::~Ticker()
{
	Stop();
}

void Ticker::Start(DWORD rate, void *params)
{
	this->ticks = 0;
	this->params = params;

	if(timer)
	{
		TimerManager::GetInstance()->Kill(timer);
	}

	timer = TimerManager::GetInstance()->Set(Tick, this, rate, true);
}

void Ticker::Stop()
{
	if(timer)
	{
		TimerManager::GetInstance()->Kill(timer);

		timer = nullptr;
	}
}

DWORD Ticker::GetTicks()
{
	return ticks;
}

TIMER_MEMBER_SRC(Ticker, Tick)
{
	callback(this, ticks++, params);
}

Counter::Counter()
{
	Reset();
}

Counter::~Counter()
{
}
	
void Counter::Reset()
{
	ticks = GetTickCount();
}

DWORD Counter::Elapsed()
{
	return GetTickCount() - ticks;
}