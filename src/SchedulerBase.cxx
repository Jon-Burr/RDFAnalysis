#include "RDFAnalysis/SchedulerBase.h"
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>

namespace {
  using vertex_t = std::pair<RDFAnalysis::SchedulerBase::ActionType, std::string>;
  struct BGLVertex {
    BGLVertex() {}
    BGLVertex(const RDFAnalysis::SchedulerBase::Action& action) :
      action(action.type, action.name) {}
    vertex_t action;
  };
  using graph_t = boost::adjacency_list<
    boost::vecS, boost::vecS, boost::directedS, BGLVertex>;
  using vert_desc_t = boost::graph_traits<graph_t>::vertex_descriptor;
  using prop_map_t = typename boost::property_map<graph_t, vertex_t BGLVertex::*>::type;

  void addToGraph(
      const RDFAnalysis::SchedulerBase::ScheduleNode& node,
      const vert_desc_t& parent,
      graph_t& graph)
  {
    vert_desc_t v = boost::add_vertex(node.action, graph);
    boost::add_edge(parent, v, graph);
    for (const auto& child : node.children)
      addToGraph(child, v, graph);
  }

  graph_t buildGraph(const RDFAnalysis::SchedulerBase::ScheduleNode& root)
  {
    graph_t graph;
    vert_desc_t v = boost::add_vertex(root.action, graph);
    for (const auto& child : root.children)
      addToGraph(child, v, graph);
    return graph;
  }

  class ActionWriter {
    public:
      ActionWriter(const prop_map_t& propMap) : m_propMap(propMap) {}
      template <class VertexOrEdge>
        void operator()(std::ostream& out, const VertexOrEdge& v) const {
          const auto& prop = m_propMap[v];
          std::string shape;
          switch(prop.first) {
            case RDFAnalysis::SchedulerBase::FILTER:
              shape = "diamond";
              break;
            case RDFAnalysis::SchedulerBase::VARIABLE:
              shape = "oval";
              break;
            default:
              shape = "box";
          }
          out << "[label=\"" << prop.second << "\" shape=" << shape << "]";
        }
    private:
      prop_map_t m_propMap;
  };
} //> end anonymous namespace

namespace RDFAnalysis {
  std::string SchedulerBase::actionTypeToString(ActionType type)
  {
    switch(type) {
      case FILTER:
        return "Filter";
      case VARIABLE:
        return "Variable";
      case FILL:
        return "Fill";
      default:
        return "UNKNOWN";
    }
  }

  SchedulerBase::RegionDef& SchedulerBase::addRegion(
      const std::string& name,
      const std::vector<std::string>& filterList)
  {
    // Empty region names are not allowed
    if (name.empty() )
      throw std::runtime_error("Empty region names are not allowed!");
    if (name == "ROOT")
      throw std::runtime_error("'ROOT' is a special name!");
    // Make sure that this region has not been defined
    if (m_regionDefs.count(name) || m_dependencies.count({FILTER, name}) )
      throw std::runtime_error("Region name '" + name + "' already used!");
    // Create the new region def
    RegionDef& region = m_regionDefs[name];
    // Then check to see if the first filter is actually a region
    if (filterList.size() > 0) {
      auto itr = m_regionDefs.find(filterList.at(0) );
      if (itr != m_regionDefs.end() ) {
        std::vector<std::string> newFilterList = itr->second.filterList;
        newFilterList.insert(
            newFilterList.end(), filterList.begin()+1, filterList.end() );
        region.filterList = std::move(newFilterList);
        return region;
      }
    }
    region.filterList = filterList;
    return region;
  }

  void SchedulerBase::filterSatisfies(
      const std::string& filter,
      const std::vector<std::string>& satisfied)
  {
    for (const std::string& f : satisfied)
      m_satisfiedBy[Action(FILTER, f)].insert({FILTER, filter});
  }
  
  std::map<SchedulerBase::Action, std::set<SchedulerBase::Action>, SchedulerBase::Action::CostOrdering> SchedulerBase::Action::expand(
      const SchedulerBase& scheduler,
      const std::set<Action>& preExisting,
      std::vector<Action>& processing) const 
  {
    if (std::count(processing.begin(), processing.end(), *this) )
      throw std::runtime_error(
          "Circular dependency found on " + name + "!");

    // Create a copy of this and make sure that we have the correct cost from
    // the scheduler
    Action thisCopy(*this);
    if (type == VARIABLE) {
      std::set<Action> loopTracker;
      while (!scheduler.m_dependencies.count(thisCopy) ) {
        if (!loopTracker.insert(thisCopy).second)
          throw std::runtime_error(
              "Closed loop found in satisfaction relations!");
        // Actions that can be satisfied by other actions may have no entry in
        // the dependencies map. In this case, look for them in the 'satisfied
        // by' map
        auto itr = scheduler.m_satisfiedBy.find(thisCopy);
        if (itr == scheduler.m_satisfiedBy.end() || itr->second.size() == 0)
          throw std::runtime_error(
              "No action of type '" +
              actionTypeToString(type) +
              "' and name '" + name + "' defined!");
        // Just use the first one
        thisCopy = *itr->second.begin();
      }
    }
    thisCopy.retrieveCost(scheduler);
    // Prepare the output
    std::map<Action, std::set<Action>, CostOrdering> output;
    // Make sure that there is an entry for this, even if it doesn't have any
    // dependencies
    output[thisCopy];
    // Add this to the processing list
    processing.push_back(thisCopy);
    // Read the dependencies from the scheduler
    for (const Action& dep : scheduler.getDependencies(thisCopy) ) {
      // Skip any dependency that is already satisfied
      if (scheduler.isActionSatisfiedBy(dep, preExisting) )
        continue;
      // Expand this dependency
      std::map<Action, std::set<Action>, CostOrdering> expanded = dep.expand(
          scheduler, preExisting, processing);
      // Add these to both the output and our dependencies
      output.insert(expanded.begin(), expanded.end() );
      for (const auto& dep : expanded) {
        output[thisCopy].insert(dep.first);
        output[thisCopy].insert(dep.second.begin(), dep.second.end() );
      }
    }
    processing.pop_back();
    return output;
  }

  std::map<SchedulerBase::Action, std::set<SchedulerBase::Action>, SchedulerBase::Action::CostOrdering> SchedulerBase::Action::expand(
      const SchedulerBase& scheduler,
      const std::set<Action>& preExisting) const
  {
    std::vector<Action> processing;
    return expand(scheduler, preExisting, processing);
  }

  void SchedulerBase::Action::retrieveCost(const SchedulerBase& scheduler) {
    cost = scheduler.getCost(*this);
  }

  const SchedulerBase::Action& SchedulerBase::ScheduleNode::next() const
  {
    auto itr = dependencies.begin();
    for (; itr != dependencies.end(); ++itr)
      if (itr->second.empty() )
        break;
    if (itr == dependencies.end() ) {
      throw std::out_of_range(
          "No next action left on " + action.name);
    }
    return itr->first;
  }

  void SchedulerBase::ScheduleNode::removeDependency(
      Action action, const SchedulerBase& scheduler)
  {
    auto directItr = dependencies.begin();
    while (directItr != dependencies.end() ) {
      if (scheduler.isActionSatisfiedBy(directItr->first, {action}) )
        // remove the action as a direct dependency
        directItr = dependencies.erase(directItr);
      else {
        auto indirectItr = directItr->second.begin();
        while (indirectItr != directItr->second.end() ) {
          if (scheduler.isActionSatisfiedBy(*indirectItr, {action}) )
            // remove the action as an indirect dependency
            indirectItr = directItr->second.erase(indirectItr);
          else
            ++indirectItr;
        }
        ++directItr;
      }
    }
  }

  void SchedulerBase::addAction(
      const Action& action,
      const std::set<Action>& dependencies)
  {
    // Empty action names are not allowed
    if (action.name.empty() )
      throw std::runtime_error("Empty action names are not allowed!");
    if (action.name == "ROOT")
      throw std::runtime_error("'ROOT' is a special name!");
    // If this is a filter, make sure there isn't a region with this name.
    if (action.type == FILTER && m_regionDefs.count(action.name) )
      throw std::runtime_error(
          "Filter name '"+action.name+"' is already used as a region name!");
    // Try to insert this into the map
    if (!m_dependencies.insert(std::make_pair(action, dependencies) ).second)
      throw std::runtime_error(actionTypeToString(action.type) +
          " name '" + action.name + "' is already defined!");
  }

  const std::set<SchedulerBase::Action>& SchedulerBase::getDependencies(
      const Action& action) const
  {
    auto itr = m_dependencies.find(action);
    if (itr == m_dependencies.end() )
      throw std::out_of_range(
          "No action of type '" +
          actionTypeToString(action.type) +
          "' and name '" + action.name + "' defined!");
    return itr->second;
  }

  float SchedulerBase::getCost(
      const Action& action) const
  {
    auto itr = m_dependencies.find(action);
    if (itr == m_dependencies.end() )
      throw std::out_of_range(
          "No action of type '" +
          actionTypeToString(action.type) +
          "' and name '" + action.name + "' defined!");
    return itr->first.cost;
  }

  bool SchedulerBase::isActionSatisfiedBy(
      const Action& action,
      const std::set<Action>& candidates) const
  {
    std::string satisfiedBy;
    return isActionSatisfiedBy(action, candidates, satisfiedBy);
  }

  bool SchedulerBase::isActionSatisfiedBy(
      const Action& action,
      const std::set<Action>& candidates,
      std::string& satisfiedBy) const
  {
    if (candidates.count(action) ) {
      satisfiedBy = action.name;
      return true;
    }
    auto itr = m_satisfiedBy.find(action);
    if (itr == m_satisfiedBy.end() )
      // This action isn't satisfied by anything else.
      return false;
    std::vector<Action> isSatisfiedBy;
    std::set_intersection(
        candidates.begin(), candidates.end(),
        itr->second.begin(), itr->second.end(),
        std::back_inserter(isSatisfiedBy) );
    if (isSatisfiedBy.size() > 0) {
      satisfiedBy = isSatisfiedBy.begin()->name;
      return true;
    }
    else
      return false;
  }

  void SchedulerBase::actionDefinesMultipleVariables(
      const std::string& name,
      const std::vector<std::string>& defined)
  {
    for (const std::string& def : defined) {
      m_satisfiedBy[Action(VARIABLE, def)].insert({VARIABLE, name});
    }
  }

  SchedulerBase::ScheduleNode SchedulerBase::schedule(
      const IBranchNamer& namer) const
  {
    ScheduleNode rawRoot = rawSchedule();
    // Prepare the output
    ScheduleNode root({FILTER, "ROOT"});
    // What pre-existing dependencies are there (i.e. variables in the input
    // namer).
    std::set<Action> preExisting;
    for (const std::string& branch : namer.branches() )
      preExisting.insert({VARIABLE, branch});

    // Expand all of the children of the raw root node
    for (ScheduleNode& child : rawRoot.children)
      child.expand(*this, preExisting);
    addChildren(std::move(rawRoot.children), &root, preExisting);
    return root;
  }

  SchedulerBase::ScheduleNode SchedulerBase::rawSchedule() const
  {
    // First step is to take our lists of filter steps and convert them into a
    // tree-like structure
    // For example a list like
    // A -- B -- C
    // A -- B -- D
    // A -- E -- F
    //
    // becomes
    //           C
    //      B -- |
    //      |    D
    // A -- |
    //      |
    //      E -- F
    ScheduleNode root({FILTER, "ROOT"});
    for (const auto& regionPair : m_regionDefs) {
      // Start each region def from the root
      ScheduleNode* current = &root;
      for (auto filterItr = regionPair.second.filterList.begin();
          filterItr != regionPair.second.filterList.end();
          ++filterItr) {
        // Loop over filters until we find one that has not been added to the
        // graph already.
        auto childItr = std::find_if(
            current->children.begin(),
            current->children.end(),
            [name=*filterItr] (const ScheduleNode& node) { return node.action.name == name;});
        if (childItr == current->children.end() ) {
          // This selection has not been made, add all remaining filters
          while( filterItr != regionPair.second.filterList.end() ) {
            current->children.emplace_back(Action(FILTER, *filterItr) );
            current = &current->children.back();
            ++filterItr;
          }
          break; // break out of the loop over filters
        }
        else {
          // We already have this selection on this node so use it rather than
          // adding a new one
          current = &*childItr;
        }
      }
      // Set the region of this node
      if (current->region.empty() )
        current->region = regionPair.first;
      else
        throw std::runtime_error("Region definitions for '"+
            current->region + "' and '" + regionPair.first + "' are identical!");
      // Add the fills as children of this node
      for (const std::string& fill : regionPair.second.fills)
        current->children.emplace_back(Action(FILL, fill) );
    }
    // Return the root
    return root;
  } //> end function SchedulerBase::rawSchedule

  void SchedulerBase::addChildren(
      std::vector<ScheduleNode>&& sources,
      ScheduleNode* target,
      std::set<Action> preExisting) const
  {
    // End early if there's nothing to do
    if (sources.empty() )
      return;
    // If any of the sources actions are in the preExisting set then this is an
    // inconsistent set up (the filter order won't be what the user wanted).
    for (const ScheduleNode& source : sources) {
      std::string name;
      if (source.action.type == FILTER &&
          isActionSatisfiedBy(source.action, preExisting, name) ) {
        std::string err = "Filter '" + source.action.name;
        if (name == source.action.name)
          err += "' already exists in the schedule!";
        else
          err += "' was already satisfied by '" + name + "'!";
        err += " This was probably added as a dependency.";
        throw std::runtime_error(err);
      }
    }
    // Note - this assumes that the last thing in each source is a filter. This
    // is true so long as the rawSchedule function doesn't change
    ScheduleNode* current = target;
    while (true) {
      auto itr = sources.begin();
      // if any of the sources are trying to add a variable then add it
      for (; itr != sources.end(); ++itr) {
        const Action& toAdd = itr->next();
        if (toAdd.type == VARIABLE) {
          current->children.emplace_back(toAdd);
          current = &current->children.back();
          preExisting.insert(toAdd);
          for (ScheduleNode& source : sources)
            source.removeDependency(toAdd, *this);
          break;
        }
      }
      // Continue until none of the sources satisfy the condition
      if (itr == sources.end() )
        break;
    }
    // When we've reached this point it's guaranteed that every source is trying
    // to add a filter next.
    // The next step is to group them by the filter they're trying to add
    std::map<Action, std::vector<ScheduleNode>> grouped;
    for (ScheduleNode& node : sources)
      grouped[node.next()].push_back(std::move(node) );
    for (auto& groupedPair : grouped) {
      // Add the action to the output
      current->children.emplace_back(groupedPair.first);
      std::vector<ScheduleNode> nextChildren;
      auto itr = groupedPair.second.begin();
      while (itr != groupedPair.second.end() ) {
        // Remove the filter from our dependencies
        itr->removeDependency(groupedPair.first, *this);
        if (itr->dependencies.empty() ) {
          if (!itr->region.empty() ) {
            if (current->children.back().region.empty() )
              current->children.back().region = itr->region;
            else
              throw std::runtime_error("Region definitions for '"+
                  current->children.back().region + "' and '" + itr->region +
                  "' are identical after dependency resolution!");
          }
          // We've done everything we need to for this source
          std::move(itr->children.begin(), itr->children.end(),
              std::back_inserter(nextChildren) );
          itr = groupedPair.second.erase(itr);
        }
        else
          ++itr;
      }
      std::set<Action> preExistingNext = preExisting;
      preExistingNext.insert(groupedPair.first);
      for (ScheduleNode& child : nextChildren)
        child.expand(*this, preExistingNext);
      std::move(nextChildren.begin(), nextChildren.end(),
          std::back_inserter(groupedPair.second) );
      addChildren(std::move(groupedPair.second),
                  &current->children.back(),
                  preExistingNext);
    }
  }

  void SchedulerBase::printSchedule(
      std::ostream& os, const ScheduleNode& root)
  {
    // Have to build the BGL representation
    graph_t graph = buildGraph(root);
    prop_map_t propMap =  boost::get(&BGLVertex::action, graph);
    boost::write_graphviz(os, graph, ActionWriter(propMap) );
  }
} //> end namespace RDFAnalysis
