#include "RDFAnalysis/Helpers.h"

namespace RDFAnalysis {
  std::string uniqueBranchName(const std::string& stub) {
    static unsigned int n = 0;
    return "_"+stub+std::to_string(n++)+"_";
  }
} //> end namespace RDFAnalysis
