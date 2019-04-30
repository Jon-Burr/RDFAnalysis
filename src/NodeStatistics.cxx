#include "RDFAnalysis/NodeStatistics.h"
#include <iostream>

namespace RDFAnalysis {
  NodeStatistics::NodeStatistics(
      const std::shared_ptr<ULong64_t>& result,
      unsigned int nSlots) :
    m_result(result),
    m_resultSlots(nSlots, ULong64_t{})
    {}

  void NodeStatistics::Exec(unsigned int slot)
  {
    ++m_resultSlots[slot];
  }

  void NodeStatistics::Finalize()
  {
    for (ULong64_t count : m_resultSlots)
      *m_result += count;
  }

  WeightedNodeStatistics::WeightedNodeStatistics(
      const std::shared_ptr<Result_t>& result,
      unsigned int nSlots) :
    m_result(result),
    m_resultSlots(nSlots, Result_t{})
    {}

  void WeightedNodeStatistics::Exec(unsigned int slot, float weight)
  {
    Result_t& result = m_resultSlots[slot];
    result.first += weight;
    result.second += weight*weight;
  }

  void WeightedNodeStatistics::Finalize()
  {
    for (const Result_t& count : m_resultSlots) {
      m_result->first += count.first;
      m_result->second += count.second;
    }
  }
} //> end namespace RDFAnalysis
