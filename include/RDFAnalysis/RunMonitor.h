#ifndef RDFAnalysis_RunMonitor_H
#define RDFAnalysis_RunMonitor_H

#include <map>
#include <boost/optional.hpp>
#include <Rtypes.h>

namespace RDFAnalysis {
  class RunMonitor {
    public:
      RunMonitor(ULong64_t printEvery);
      RunMonitor(ULong64_t printEvery, ULong64_t total);

      void beginRun();

      void operator()(unsigned int slot);
    private:
      std::map<unsigned int, ULong64_t> m_seen;
      ULong64_t m_printEvery;
      boost::optional<ULong64_t> m_total;
  }; //> end class RunMonitor
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_RunMonitor_H
