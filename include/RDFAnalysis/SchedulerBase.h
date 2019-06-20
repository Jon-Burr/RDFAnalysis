#ifndef RDFAnalysis_SchedulerBase_H
#define RDFAnalysis_SchedulerBase_H

#include "RDFAnalysis/IBranchNamer.h"

#include <string>
#include <map>
#include <set>
#include <vector>

namespace RDFAnalysis {
  /**
   * @brief Base class for the scheduler
   *
   * This is the class that does most of the heavy-lifting, being responsible
   * for figuring out the order of the actions used.
   */
  class SchedulerBase {
    public:
      /// Enum to describe the different types of action
      enum ActionType {
        /// A filter imposes a selection on the events it sees and can also
        /// create a branch in the tree structure
        FILTER,
        /// A variable defines a new variable
        VARIABLE,
        /// A fill is required to return a SysResultPtr to a TObject
        FILL,
        /// Not a valid action, used to construct a placeholder action
        INVALID
      };
      /// Convert an ActionType to a string
      static std::string actionTypeToString(ActionType type);

      /// Helper struct to define a region
      struct RegionDef {
        /// Ordered list of filters defining the region
        std::vector<std::string> filterList;
        /// Set of fills to be performed on that region
        std::set<std::string> fills;
        /// Add a fill to the region
        void addFill(const std::string& fill) {fills.insert(fill); }
      }; //> end struct RegionDef

      /**
       * @brief Add a region to be scheduled
       * @param name The name of the region to be added
       * @param filterList The ordered list of filters to be added.
       * @exception std::runtime_error An invalid region name is given. Invalid
       * region names are the empty string '', 'ROOT' or any pre-existing region
       * or filter name.
       */
      RegionDef& addRegion(
          const std::string& name,
          const std::vector<std::string>& filterList);

      /// Access the current region definitions
      std::map<std::string, RegionDef>& regionDefs()
      { return m_regionDefs;}
      /// (const) access the current region definitions
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

      /**
       * @brief Helper struct to represent all the information that the
       * scheduler needs to know about an action in order to form the ordering.
       *
       * An action is uniquely identified by its type and name.
       * Note that often when passing around actions the cost value is not
       * filled, as it is only relevant at one point in the schedule building.
       */
      struct Action {
        /// Create the action
        Action(ActionType type, const std::string& name, float cost = 0) :
          type(type), name(name), cost(cost) {}

        /// The type of this action
        ActionType type;
        /// The name of this action
        std::string name;
        /// The cost-estimate of this action
        float cost;

        /// Operator used to construct sets of actions. Order by type first,
        /// then name
        bool operator<(const Action& rhs) const {
          return (type == rhs.type ? name < rhs.name : type < rhs.type);
        }
        /// Equality comparison operator for actions
        bool operator==(const Action& rhs) const {
          return type == rhs.type && name == rhs.name;
        }
        /// Ordering used in the actual scheduling process. Order by cost first,
        /// then type, then name.
        struct CostOrdering {
          /// The actual ordering
          bool operator()(const Action& lhs, const Action& rhs) const
          {
            return (lhs.cost == rhs.cost ? lhs < rhs : lhs.cost < rhs.cost);
          }
        }; //> end struct CostOrdering

        /**
         * @brief Expand the dependencies of this action
         * @param scheduler The scheduler instance to use
         * @param preExisting Any actions that already exist by the point in the
         * schedule in which this action is to be inserted
         * @exception std::runtime_error if a circular dependency is found
         * @return A map of this action and all its direct and indirect
         * dependencies to their direct dependencies
         */
        std::map<Action, std::set<Action>, CostOrdering> expand(
            const SchedulerBase& scheduler,
            const std::set<Action>& preExisting = {}) const;
        /**
         * @brief Expand the dependencies of this action
         * @param scheduler The scheduler instance to use
         * @param preExisting Any actions that already exist by the point in the
         * schedule in which this action is to be inserted
         * @param processing A list of the actions currently being processed.
         * This is used to catch circular dependencies that would otherwise
         * cause an infinite loop
         * @exception std::runtime_error if a circular dependency is found
         * @return A map of this action and all its direct and indirect
         * dependencies to their direct dependencies
         */
        std::map<Action, std::set<Action>, CostOrdering> expand(
            const SchedulerBase& scheduler,
            const std::set<Action>& preExisting,
            std::vector<Action>& processing) const;

        /// Get the cost of this action from the scheduler
        void retrieveCost(const SchedulerBase& scheduler);
      }; //> end struct Action

      /**
       * @brief Helper struct used to build and express the schedule.
       *
       * Each node performs one action. When building the raw schedule, each
       * node is loaded with its dependencies which are then expanded in the
       * full schedule.
       */
      struct ScheduleNode {
        /// Build the node from the action it performs
        ScheduleNode(const Action& action) : action(action) {}
        /// The action performed by this node
        Action action;
        /// The dependencies of that action
        std::map<Action, std::set<Action>, Action::CostOrdering> dependencies;
        /// The children of this node (i.e. the ones that follow it)
        std::vector<ScheduleNode> children;
        /// The region, if any, that this node defines (i.e. is the final action
        /// listed for that region)
        std::string region;

        /**
         * @brief Get the next dependency from this action. This is defined as
         * the 'smallest' action (using Action::CostOrdering) that has no
         * remaining dependencies
         * @exception std::out_of_range If no such dependency exists. This is a
         * logic error as it should be impossible to occur.
         */
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

      /// Get the ROOT of the output filter schedule. Will be empty if schedule
      /// has not been called.
      ScheduleNode& getSchedule() { return m_schedule; }
      /// Get the ROOT of the output filter schedule. Will be empty if schedule
      /// has not been called.
      const ScheduleNode& getSchedule() const { return m_schedule; }

      /// Get the variables used by this schedule. Will be empty if schedule has
      /// not been called.
      const std::vector<std::string>& usedVariables() const { return m_usedVars; }

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
       * @param considerSelf Whether or not to count action satsified if it
       * occurs in candidates
       */
      bool isActionSatisfiedBy(
          const Action& action,
          const std::set<Action>& candidates,
          bool considerSelf = true) const;

      /**
       * @brief Check whether an action has already been satisfied by one of a
       * list of candidates.
       * @param action The action to check for
       * @param candidates Candidates that could have satisfied it.
       * @param[out] satisfiedBy If the action has already been satisfied, fill
       * this reference with the candidate that satisfied it
       * @param considerSelf Whether or not to count action satsified if it
       * occurs in candidates
       */
      bool isActionSatisfiedBy(
          const Action& action,
          const std::set<Action>& candidates,
          Action& satisfiedBy,
          bool considerSelf = true) const;
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
       * @brief Look through a list of filters for any that satisfy each other
       *
       * Imagine an action that ends up with the dependencies (writing filters
       * only) {pT > 100, n_B > 1, n_B > 2}. Clearly running both of the n_B
       * selections is wasteful, and if scale factors are applied as part of
       * those selections it might well be actively harmful. Given this set of
       * inputs the return value of this function would be
       * {n_B > 1 : n_B > 2}
       */
      std::map<Action, Action> buildReplacementMap(
          const std::set<Action>& filters) const;

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
       * @return A reference to the root node of the full schedule
       */
      ScheduleNode& schedule(const IBranchNamer& namer);


      /**
       * @brief Build the 'raw' schedule
       * @return The root node of the raw schedule
       *
       * The raw schedule is derived only from the requested regions.
       */
      ScheduleNode rawSchedule() const;

    private:
      void addChildren(
          std::vector<ScheduleNode>&& sources,
          ScheduleNode* target,
          std::set<Action> preExisting);
      /// The region definitions
      std::map<std::string, RegionDef> m_regionDefs;

      /// Defined actions and their dependencies
      std::map<Action, std::set<Action>> m_dependencies;

      /// Keep track of actions that can be satisfied by other dependencies.
      /// This can happen when a single action defines multiple variables or
      /// when one filter is necessarily tighter than another
      std::map<Action, std::set<Action>> m_satisfiedBy;

      /// Keep the root node of the output filter schedule here
      ScheduleNode m_schedule{{FILTER, "ROOT"}};

      /// The variables used by this schedule
      std::vector<std::string> m_usedVars;

      void expandSatisfiesRelations(
          std::map<Action, std::set<Action>>::iterator itr,
          std::set<Action>& processed);
  }; //> end namespace SchedulerBase
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_SchedulerBase_H
