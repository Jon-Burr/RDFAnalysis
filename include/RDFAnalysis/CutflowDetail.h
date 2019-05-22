#ifndef RDFAnalysis_CutflowDetail_H
#define RDFAnalysis_CutflowDetail_H

#include "RDFAnalysis/NodeFwd.h"

namespace RDFAnalysis {
  class CutflowDetail {
    public:
      CutflowDetail(Node<CutflowDetail>& node) :
        m_stats(node.Count() ),
        m_weightedStats( 
            node.getWeight().empty() ?
            SysResultPtr<std::pair<float,float>>(node.namer().nominalName() ) :
            node.Aggregate(
              [] (const std::pair<float, float>& lhs, float rhs)
              { return std::make_pair(lhs.first + rhs, lhs.second + rhs*rhs); },
              [] (const std::pair<float, float>& lhs, const std::pair<float, float>& rhs)
              { return std::make_pair(lhs.first+rhs.first, lhs.second+rhs.second); },
              node.getWeight() ) )
      {}

      SysResultPtr<ULong64_t> stats() { return m_stats; }
      SysResultPtr<std::pair<float, float>> weightedStats()
      { return m_weightedStats; }

    private:
      SysResultPtr<ULong64_t> m_stats;
      SysResultPtr<std::pair<float, float>> m_weightedStats;
  }; //> end class CutflowDetail
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_CutflowDetail_H
