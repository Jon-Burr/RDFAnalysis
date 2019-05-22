#ifndef RDFAnalysis_EmptyDetail_H
#define RDFAnalysis_EmptyDetail_H

#include "RDFAnalysis/NodeFwd.h"

namespace RDFAnalysis {
  class EmptyDetail {
    public:
      EmptyDetail(Node<EmptyDetail>&) {}
  }; //> end class EmptyDetail
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_EmptyDetail_H
