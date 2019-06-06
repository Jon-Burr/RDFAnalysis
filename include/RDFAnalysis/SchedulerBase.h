#ifndef RDFAnalysis_SchedulerBase_H
#define RDFAnalysis_SchedulerBase_H

#include "RDFAnalysis/IBranchNamer.h"

#include <string>
#include <map>
#include <set>
#include <vector>

namespace RDFAnalysis {
  class SchedulerBase {
    public:
      enum ActionType {FILTER, VARIABLE, FILL};
      static std::string actionTypeToString(ActionType type);

      struct RegionDef {
        std::vector<std::string> filterList;
        std::set<std::string> fills;
        void addFill(const std::string& fill) {fills.insert(fill); }
      }; //> end struct RegionDef

      RegionDef& addRegion(
          const std::string& name,
          const std::vector<std::string>& filterList);

      std::map<std::string, RegionDef>& regionDefs()
      { return m_regionDefs;}
      const std::map<std::string, RegionDef>& regionDefs() const
      { return m_regionDefs;}

      struct Action {
        Action(ActionType type, const std::string& name, float cost = 0) :
          type(type), name(name), cost(cost) {}
        ActionType type;
        std::string name;
        float cost;
        bool operator<(const Action& rhs) const {
          return (type == rhs.type ? name < rhs.name : type < rhs.type);
        }
        bool operator==(const Action& rhs) const {
          return type == rhs.type && name == rhs.name;
        }
        struct CostOrdering {
          bool operator()(const Action& lhs, const Action& rhs) const
          {
            return (lhs.cost == rhs.cost ? lhs < rhs : lhs.cost < rhs.cost);
          }
        }; //> end struct CostOrdering
        std::map<Action, std::set<Action>, CostOrdering> expand(
            const SchedulerBase& scheduler,
            const std::set<Action>& preExisting = {}) const;
        std::map<Action, std::set<Action>, CostOrdering> expand(
            const SchedulerBase& scheduler,
            const std::set<Action>& preExisting,
            std::vector<Action>& processing) const;
        void retrieveCost(const SchedulerBase& scheduler);
      }; //> end struct Action

      struct ScheduleNode {
        ScheduleNode(const Action& action) : action(action) {}
        Action action;
        std::map<Action, std::set<Action>, Action::CostOrdering> dependencies;
        std::vector<ScheduleNode> children;
        std::string region;

        const Action& next() const {
          auto itr = dependencies.begin();
          for (; itr != dependencies.end(); ++itr)
            if (itr->second.empty() )
              break;
          if (itr == dependencies.end() ) {
            for (const auto& dep : dependencies) {
              std::cout << dep.first.name << ": ";
              for (const auto& dep2 : dep.second)
                std::cout << dep2.name << ", ";
              std::cout << std::endl;
            }
            throw std::out_of_range(
                "No next action left on " + action.name);
          }
          return itr->first;
        }

        /// Remove a dependency from consideration
        void removeDependency(const Action& action) {
          // Remove this action if it's a direct dependency
          dependencies.erase(action);
          // Remove this action from any indirect dependencies
          for (auto& dep : dependencies)
            dep.second.erase(action);
        }

        /// Expand this node's dependencies
        void expand(
            const SchedulerBase& scheduler,
            const std::set<Action>& preExisting = {})
        { dependencies = action.expand(scheduler, preExisting); }
      }; //> end struct ScheduleNode

      /**
       * @brief Print a schedule to a graphviz file
       * @param os The output stream
       * @param root the root node of the schedule
       */
      static void printSchedule(std::ostream& os, const ScheduleNode& root);
    protected:

      /**
       * @brief Add a new action to the record
       * @param action The action to add
       * @param dependencies Its dependencies
       * @exception std::runtime_error If that type/name combination exists
       */
      void addAction(
          const Action& action,
          const std::set<Action>& dependencies);

      /**
       * @brief Get the dependency corresponding to an action.
       * @exception std::out_of_range if the action is unknown
       */
      const std::set<Action>& getDependencies(const Action& action) const;

      /**
       * @brief Get the cost of an action
       * @exception std::out_of_range if the action is unknown
       */
      float getCost(const Action& action) const;

      /**
       * @brief Build the schedule
       * @param namer IBranchNamer that provides the list of predefined
       * variables.
       * @return The root node of the full schedule
       */
      ScheduleNode schedule(const IBranchNamer& namer) const;

      /**
       * @brief Build the 'raw' schedule
       * @return The root node of the raw schedule
       *
       * The raw schedule is derived only from the requested regions.
       */
      ScheduleNode rawSchedule() const;

      void addChildren(
          std::vector<ScheduleNode>&& sources,
          ScheduleNode* target,
          std::set<Action> preExisting) const;

      /// The region definitions
      std::map<std::string, RegionDef> m_regionDefs;

      /// Defined actions and their dependencies
      std::map<Action, std::set<Action>> m_dependencies;
  }; //> end namespace SchedulerBase
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_SchedulerBase_H
