#ifndef RDFAnalysis_Auditors_DebugPrinter_H
#define RDFAnalysis_Auditors_DebugPrinter_H

#include "RDFAnalysis/Auditors/IAuditor.h"
#include <ROOT/RDF/ActionHelpers.hxx>
#include <tuple>
#include <iostream>

namespace RDFAnalysis {
  template <typename Detail>
    class DebugPrinter : public IAuditor<Detail> {
      public:
        using node_t = typename IAuditor<Detail>::node_t;
        struct Action : public ROOT::Detail::RDF::RActionImpl<Action> {
          // Empty return type
          using Result_t = std::tuple<>;
          Action(const std::string& message) : message(message) {}
          void Exec(unsigned int slot)
          { std::cout << message << std::endl; }
          void InitTask(TTreeReader*, unsigned int) {}
          void Initialize() {}
          void Finalize() {}
          std::string GetActionName()
          { return "DebugPrinter::Action"; }
          std::shared_ptr<Result_t> GetResultPtr()
          { return nullptr; }
          std::string message;
        }; //> end struct DebugPrinter

        ~DebugPrinter() override {}

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
