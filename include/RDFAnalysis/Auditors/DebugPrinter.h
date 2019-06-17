#ifndef RDFAnalysis_Auditors_DebugPrinter_H
#define RDFAnalysis_Auditors_DebugPrinter_H

#include "RDFAnalysis/Auditors/IAuditor.h"
#include <ROOT/RDF/ActionHelpers.hxx>
#include <tuple>
#include <iostream>

namespace RDFAnalysis {
  /**
   * @brief Auditor that prints before and every action is called
   * @tparam Detail The detail type of the node type being audited
   *
   * This node will print out before and after every action on every event. It
   * is designed to debug crashes for which ROOT::RDataFrame does not print out
   * anything useful. It obviously should *not* be used outside of debugging.
   */
  template <typename Detail>
    class DebugPrinter : public IAuditor<Detail> {
      public:
        /// The node type this applies to
        using node_t = typename IAuditor<Detail>::node_t;
        /// The RDF action responsible for printing on every event
        struct Action : public ROOT::Detail::RDF::RActionImpl<Action> {
          /// Empty return type
          using Result_t = std::tuple<>;
          /// Construct the action from the message that should be printed
          Action(const std::string& message) : message(message) {}
          /// Print the message
          void Exec(unsigned int slot)
          { std::cout << message << std::endl; }
          /// Init: no-op
          void InitTask(TTreeReader*, unsigned int) {}
          /// Init: no-op
          void Initialize() {}
          /// Finalize: no-op
          void Finalize() {}
          /// Get the nickname for this action
          std::string GetActionName()
          { return "DebugPrinter::Action"; }
          /// Get the 'result' for this action
          std::shared_ptr<Result_t> GetResultPtr()
          { return nullptr; }
          /// The message that this prints
          std::string message;
        }; //> end struct DebugPrinter

        ~DebugPrinter() override {}

        /// Load an action to print before a node
        void preAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string&) override
        { 
          std::string message = "Begin " +
            SchedulerBase::actionTypeToString(source.action.type) +
            " '" + source.action.name + "'";
          if (!target->isAnonymous() )
            message += " from '" + target->name() + "'.";
          // This isn't quite right as it won't print on systematic variations
          // created by a filter
          for (auto& rnodePair : target->rnodes() )
            m_results.push_back(
                rnodePair.second.Book(
                  Action(message + " (" + rnodePair.first + ")") ) );
        }

        /// Load an action to print after a node
        void postAuditNode(
            const SchedulerBase::ScheduleNode& source,
            node_t* target,
            const std::string&) override
        { 
          std::string message = "End " +
            SchedulerBase::actionTypeToString(source.action.type) +
            " '" + source.action.name + "'";
          for (auto& rnodePair : target->rnodes() )
            m_results.push_back(
                rnodePair.second.Book(
                  Action(message + " (" + rnodePair.first + ")") ) );
        }
      private:
        std::vector<ROOT::RDF::RResultPtr<std::tuple<>>> m_results;
    }; //> end class DebugPrinter<Detail>
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_Auditors_DebugPrinter_H
