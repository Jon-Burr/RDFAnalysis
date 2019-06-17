#ifndef RDFAnalysis_RunMonitor_H
#define RDFAnalysis_RunMonitor_H

#include <map>
#include <boost/optional.hpp>
#include <Rtypes.h>

namespace RDFAnalysis {
  /**
   * @brief The default run monitor.
   * 
   * Will print out a fixed number of events (treating each slot separately)
   */
  class RunMonitor {
    public:
      /**
       * @brief Create the monitor
       * @param printEvery How often to print (for each slot)
       *
       * Using this constructor will mean that no 'total' number is printed
       */
      RunMonitor(ULong64_t printEvery);
      /**
       * @brief Create the monitor
       * @param printEvery How often to print (for each slot)
       * @param total The total number of events in the sample
       */
      RunMonitor(ULong64_t printEvery, ULong64_t total);

      /// Print at the beginning of the run
      void beginRun();

      /// Print on an event
      void operator()(unsigned int slot);
    private:
      std::map<unsigned int, ULong64_t> m_seen;
      ULong64_t m_printEvery;
      boost::optional<ULong64_t> m_total;
  }; //> end class RunMonitor
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_RunMonitor_H
