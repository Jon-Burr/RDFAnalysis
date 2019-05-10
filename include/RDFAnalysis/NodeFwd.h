#ifndef RDFAnalysis_NodeFwd_H
#define RDFAnalysis_NodeFwd_H

namespace RDFAnalysis {
  class DefaultBranchNamer;
  class EmptyDetail;
  template <
    typename BranchName = DefaultBranchNamer,
    typename Detail = EmptyDetail
      > class Node;
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_NodeFwd_H
