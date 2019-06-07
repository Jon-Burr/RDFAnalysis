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

      /**
       * @brief Tell the scheduler that a filter also satisfies the condition of
       * other filters
       * @param filter The filter
       * @param satisfied Other filters that this filter satisfies.
       *
       * For example, x == 4 clearly satisfies the filter x > 2 so any action
       * that depends on this selection does not need to sequence 'x > 2' if 'x
       * == 4' has already been scheduled.
       */
      void filterSatisfies(
          const std::string& filter,
          const std::vector<std::string>& satisfied);

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

        const Action& next() const;

        /// Remove a dependency from consideration
        void removeDependency(Action action, const SchedulerBase& scheduler);

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
       * @brief Check whether an action has already been satisfied by one of a
       * list of candidates.
       * @param action The action to check for
       * @param candidates Candidates that could have satisfied it.
       */
      bool isActionSatisfiedBy(
          const Action& action,
          const std::set<Action>& candidates) const;

      /**
       * @brief Check whether an action has already been satisfied by one of a
       * list of candidates.
       * @param action The action to check for
       * @param candidates Candidates that could have satisfied it.
       * @param[out] satisfiedBy If the action has already been satisfied, fill
       * this reference with the name of the candidate that satisfied it
       */
      bool isActionSatisfiedBy(
          const Action& action,
          const std::set<Action>& candidates,
          std::string& satisfiedBy) const;

      /**
       * @brief Tell the scheduler that an action is defining multiple variables
       * @param name The name of the action
       * @param defined The list of variables.
       */
      void actionDefinesMultipleVariables(
          const std::string& name,
          const std::vector<std::string>& defined);

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
    private:

      /// The region definitions
      std::map<std::string, RegionDef> m_regionDefs;

      /// Defined actions and their dependencies
      std::map<Action, std::set<Action>> m_dependencies;

      /// Keep track of actions that can be satisfied by other dependencies.
      /// This can happen when a single action defines multiple variables or
      /// when one filter is necessarily tighter than another
      std::map<Action, std::set<Action>> m_satisfiedBy;
  }; //> end namespace SchedulerBase
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_SchedulerBase_H
