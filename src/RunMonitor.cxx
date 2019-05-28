#include "RDFAnalysis/RunMonitor.h"
#include "RDFAnalysis/Helpers.h"

namespace RDFAnalysis {
  RunMonitor::RunMonitor(ULong64_t printEvery) :
    m_printEvery(printEvery) 
  {
    for (std::size_t i = 0; i < getNSlots(); ++i)
      m_seen[i] = 0;
  }

  RunMonitor::RunMonitor(ULong64_t printEvery, ULong64_t total) :
    m_printEvery(printEvery),
    m_total(total)
  {
    for (std::size_t i = 0; i < getNSlots(); ++i)
      m_seen[i] = 0;
  }

  void RunMonitor::beginRun()
  {
    std::cout << "Beginning run with " << getNSlots() << " slots." << std::endl;
  }

  void RunMonitor::operator()(unsigned int slot)
  {
    if (++m_seen[slot] % m_printEvery == 0) {
      std::ostringstream os;
      os << "slot " << slot << " seen " << m_seen[slot] << " events.";
      if (m_total) {
        // Add up everything seen so far. This is a read-only operation so it's
        // essentially safe. The printed result is not determinsitic as other
        // threads will update their m_seen values while this is happening but
        // we don't care too much about that because this is just a progress
        // counter
        ULong64_t totalSeen = 0;
        for (const std::pair<unsigned int, ULong64_t>& counts : m_seen)
          totalSeen += counts.second;
        os << " " << totalSeen << "/" << *m_total;
      }
      std::cout << os.str() << std::endl;
    }
  }
} //> end namespace RDFAnalysis
