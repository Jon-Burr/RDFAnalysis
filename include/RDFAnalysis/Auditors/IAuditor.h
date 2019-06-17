#ifndef RDFAnalysis_IAuditor_H
#define RDFAnalysis_IAuditor_H

#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/SchedulerBase.h"

namespace RDFAnalysis {
  template <typename Detail>
    /**
     * @brief Interface class for auditors.
     * @tparam Detail The detail type of the node type being audited
     *
     * Auditors are helper objects used by the scheduler to access some extra
     * information about the schedule/nodes beyond the physics information that
     * they output. Examples include debug printing (before and after each node
     * executes) or drawing a representation of the tree with graphviz.
     *
     * Auditors are allowed to insert extra nodes into the computation tree but
     * this should not happen in a way that changes the physics result!
     */
    class IAuditor {
      public:
        /// The node type this applies to
        using node_t = Node<Detail>;
        virtual ~IAuditor() = 0;
        /**
         * @brief Extract information from the full schedule produced by the
         * SchedulerBase class
         * @param root The root node of the created schedule.
         */
        virtual void auditSchedule(const SchedulerBase::ScheduleNode& root) {}
        /**
         * @brief Insert an operation before an action is added into the output
         * tree
         * @param source The schedule node that is about to be added
         * @param target The Node to which it will be added
         * @param regionName The name of the current output region (if any) that
         * we are in.
         */
        virtual void preAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string& regionName) {}
        /**
         * @brief Insert an operation after an action is added into the output
         * tree
         * @param source The schedule node that has just been added
         * @param target The Node to which it was added
         * @param regionName The name of the current output region (if any) that
         * we are in.
         */
        virtual void postAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string& regionName) {}

        /// Do anything that should be done after the full loop has run
        virtual void report() {}
    }; //> end class IAuditor<Detail>
  template <typename Detail>
    IAuditor<Detail>::~IAuditor() {}
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_IAuditor_H
