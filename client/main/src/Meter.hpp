//
// Created by Leonid  on 2019-03-28.
//

#ifndef Meter_hpp
#define Meter_hpp

#include "oatpp/core/base/Environment.hpp"

#include <queue>

/**
 * Class to calculate stats.
 */
class Meter {
private:
  std::queue<v_int64> m_ticks; // in micro
  std::queue<v_int64> m_values;
  v_int32 m_maxPoints;
public:

  Meter(v_int32 maxPoints)
   : m_maxPoints(maxPoints)
  {}

  void addPoint(v_int64 tick, v_int64 value) {
    m_ticks.push(tick);
    m_values.push(value);
    if(m_ticks.size() > m_maxPoints) {
      m_ticks.pop();
      m_values.pop();
    }
  }

  v_float64 getRatio() {
    if(m_ticks.size() > 1) {
      v_float64 ticksDiff = m_ticks.back() - m_ticks.front();
      v_float64 valueDiff = m_values.back() - m_values.front();
      return valueDiff / ticksDiff;
    }
    return 0;
  }

  v_float64 perSecond() {
    return getRatio() * 1000 * 1000;
  }

  v_float64 perMinute() {
    return getRatio() * 60 * 1000 * 1000;
  }


};


#endif //Meter_hpp
