/*
  Jun 30 2004 carlc
  class to automate doing frames per second functionality.

  FPS fps;
  
  while(each frame)
  {
    // do your frame stuff...

    fps.OnFrame();
    // display the fps
    display(fps.GetFPSString());
  }

  call SetRecalcInterval() if you want to refresh the fps less frequently.
*/

#pragma once

#include <windows.h>
#include <string>


class FPS
{
public:
  FPS() :
    m_fps(0),
    m_lasttick(0),
    m_interval(0),
    m_frames(0),
    m_totallasttick(0),
    m_totalframes(0)
  {
    LARGE_INTEGER lifreq;
    QueryPerformanceFrequency(&lifreq);
    m_freq = (double)lifreq.QuadPart;
  }

  void SetRecalcInterval(double secs)
  {
    m_interval = (LONGLONG)(secs * m_freq);
  }

  inline void OnFrame()
  {
    LONGLONG ct = GetCurrentTick();
    LONGLONG delta = ct - m_lasttick;
    m_frames ++;
    m_totalframes ++;

    if(delta > m_interval)
    {
      // recalc fps and reset m_frames
      m_fps = (double)m_frames / TicksToSeconds(delta);
      m_frames = 0;
      m_lasttick = ct;
    }
  }

  inline void ResetTotal()
  {
    m_totalframes = 0;
    m_totallasttick = GetCurrentTick();
  }

  inline double GetAvgFPS() const
  {
    LONGLONG ct = GetCurrentTick();
    LONGLONG delta = ct - m_totallasttick;
    return (double)m_totalframes / TicksToSeconds(delta);
  }

  inline std::string GetAvgFPSString() const
  {
    char sz[100];
    sprintf(sz, "%4.2f", GetAvgFPS());
    return std::string(sz);
  }

  inline double GetFPS() const
  {
    return m_fps;
  }

  inline std::string GetFPSString() const
  {
    char sz[100];
    sprintf(sz, "%4.2f", m_fps);
    return std::string(sz);
  }

private:

  inline double TicksToSeconds(LONGLONG n) const
  {
    return (double)n / m_freq;
  }

  inline static LONGLONG GetCurrentTick()
  {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
  }
  double m_fps;
  double m_freq;// units per second
  long m_frames;// # of frames since last recalc
  LONGLONG m_interval;// how many units until we refresh m_fps
  LONGLONG m_lasttick;

  LONGLONG m_totallasttick;
  LONGLONG m_totalframes;
};




class Timer
{
public:
  Timer() :
    m_lasttick(0),
    m_lasttick2(0)
  {
    LARGE_INTEGER lifreq;
    QueryPerformanceFrequency(&lifreq);
    m_freq = (double)lifreq.QuadPart;
  }

  // call this to "tick" the timer... the time between the previous tick and this one is now stored.
  inline Tick()
  {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    m_lasttick2 = m_lasttick;
    m_lasttick = li.QuadPart;
    m_delta = m_lasttick - m_lasttick2;
  }

  inline double GetLastDelta() const
  {
    return TicksToSeconds(m_delta);
  }

  inline std::string GetLastDeltaString() const
  {
    char sz[100];
    sprintf(sz, "%4.2f", GetLastDelta());
    return std::string(sz);
  }

private:

  inline double TicksToSeconds(LONGLONG n) const
  {
    return (double)n / m_freq;
  }

  double m_freq;// units per second
  LONGLONG m_lasttick;// 1 tick ago
  LONGLONG m_lasttick2;// 2 ticks ago
  LONGLONG m_delta;// diff between lasttick and lasttick2
};

