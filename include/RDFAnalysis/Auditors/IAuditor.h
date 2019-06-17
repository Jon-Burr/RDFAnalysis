#ifndef RDFAnalysis_IAuditor_H
#define RDFAnalysis_IAuditor_H

#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/SchedulerBase.h"

namespace RDFAnalysis {
  template <typename Detail>
    class IAuditor {
      public:
        using node_t = Node<Detail>;
        virtual ~IAuditor() = 0;
        virtual void auditSchedule(const SchedulerBase::ScheduleNode& root) {}
        virtual void preAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string& regionName) {}
        virtual void postAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string& regionName) {}
        virtual void report() {}
    }; //> end class IAuditor<Detail>
  template <typename Detail>
    IAuditor<Detail>::~IAuditor() {}
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_IAuditor_H
