#ifndef RDFAnalysis_EmptyDetail_H
#define RDFAnalysis_EmptyDetail_H

namespace RDFAnalysis {
  class EmptyDetail {
    public:
      template <typename N>
        EmptyDetail(N&) {}
  }; //> end class EmptyDetail
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_EmptyDetail_H
