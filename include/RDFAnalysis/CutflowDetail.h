#ifndef RDFAnalysis_CutflowDetail_H
#define RDFAnalysis_CutflowDetail_H

#include "RDFAnalysis/NodeFwd.h"

namespace RDFAnalysis {
  /**
   * @brief Detail class containing cutflow information.
   *
   * This detail keeps track of cutflow and weighted cutflow information.
   */
  class CutflowDetail {
    public:
      /// Create the detail from its parent node
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

      /// Get the unweighted cutflow information.
      SysResultPtr<ULong64_t> stats() { return m_stats; }

      /// Get the weighted cutflow information (sum of weights, sum of weights
      /// squared.
      SysResultPtr<std::pair<float, float>> weightedStats()
      { return m_weightedStats; }

    private:
      /// Unweighted cutflow information
      SysResultPtr<ULong64_t> m_stats;

      /// Weighted cutflow information
      SysResultPtr<std::pair<float, float>> m_weightedStats;
  }; //> end class CutflowDetail
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_CutflowDetail_H
