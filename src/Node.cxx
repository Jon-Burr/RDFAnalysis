// package includes
#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/Helpers.h"
#include <iostream>

namespace RDFAnalysis {

  Node& Node::Define(
      const std::string& name,
      const std::string& expression)
  {
    auto expanded = m_namer->expandExpression(expression);
    return Define(name, expanded.first, expanded.second);
  }

  Node& Node::Define(
      const std::string& name,
      const std::string& expression,
      const ColumnNames_t& columns)
  {
    // First, work out which systematics this affects
    std::set<std::string> affecting = m_namer->systematicsAffecting(columns);
    // Make sure that we at least affect the nominal
    if (affecting.size() == 0)
      affecting.insert(m_namer->nominalName() );

    // For each of our RNodes, update it to be the one with the new variable
    // defined on it.
    for (auto& rnode : m_rnodes) {
      // Remove this systematic from consideration
      affecting.erase(rnode.first);
      std::string systExpression = m_namer->interpretExpression(
          expression, columns, rnode.first);
      rnode.second = rnode.second.Define(
          m_namer->createBranch(name, rnode.first, true), systExpression);
    }
    // Add any left over to the nominal
    RNode& nominal = m_rnodes.at(m_namer->nominalName());
    for (const std::string& syst : affecting) {
      std::string systExpression = m_namer->interpretExpression(
          expression, columns, syst);
      nominal = nominal.Define(
          m_namer->createBranch(name, syst, false), systExpression);
    }
    return *this;
  }

  std::shared_ptr<Node> Node::Filter(
      const std::string& expression,
      const std::string& name,
      const std::string& cutflowName)
  {
    auto expanded = m_namer->expandExpression(expression);
    return Filter(expanded.first, expanded.second, name, cutflowName);
  }

  std::shared_ptr<Node> Node::Filter(
      const std::string& expression,
      const ColumnNames_t& columns,
      const std::string& name,
      const std::string& cutflowName)
  {
    // If this is going to make an anonymous node, make sure there aren't
    // other children
    if (name.empty() && m_children.size() > 0)
      throw std::runtime_error(
          "Attempting to define an anonymous node on a node that already "
          "has children. This is not allowed.");
    // First, work out which systematics this affects
    std::set<std::string> affecting = m_namer->systematicsAffecting(columns);
    // Make sure that we at least affect the nominal
    if (affecting.size() == 0)
      affecting.insert(m_namer->nominalName() );

    // Make a child node
    // Note - use 'new' rather than make_unique as this is a protected
    // constructor
    std::shared_ptr<Node> child(new Node(*this, name, cutflowName) );
    m_children.push_back(child);

    // Apply the filters to the child node
    for (auto& rnodePair : child->m_rnodes) {
      std::string systExpression = m_namer->interpretExpression(
        expression, columns, rnodePair.first);
      rnodePair.second = rnodePair.second.Filter(systExpression, cutflowName);
      affecting.erase(rnodePair.first);
    }

    RNode& nominalNode = m_rnodes.at(m_namer->nominalName() );
    // Make new child rnodes for every remaining systematic
    for (const std::string& syst : affecting) {
      std::string systExpression = m_namer->interpretExpression(
          expression, columns, syst);
      child->m_rnodes.insert(std::make_pair(
            syst, nominalNode.Filter(systExpression, cutflowName) ) );
    }
    return child;
  }

  std::shared_ptr<Node> Node::setWeight(
      const std::string& expression,
      bool multiplicative)
  {
    if (multiplicative && m_parent && !m_parent->getWeight().empty() ) {
      // Adapt the expression to include the parent weight and then call this
      // function again with the new expression and multiplicative set to false
      return setWeight("("+expression+") * " + m_parent->getWeight(), false);
    }
    else if (m_namer->exists(expression) ) {
      // For this version only, if the new weight is already a column, no need
      // to recalculated it...
      m_weight = expression;
      setupWeightedStatistics();
    }
    else {
      m_weight = nameWeight();
      Define(m_weight, expression);
      // Reset the weighted node statistics
      setupWeightedStatistics();
    }
    return shared_from_this();
  }

  std::shared_ptr<Node> Node::setWeight(
      const std::string& expression,
      const ColumnNames_t& columns,
      bool multiplicative)
  {
    if (multiplicative && m_parent && !m_parent->getWeight().empty() ) {
      // Adapt the expression to include the parent weight and then call this
      // function again with the new expression and multiplicative set to false
      ColumnNames_t newColumns = columns;
      newColumns.push_back(m_parent->getWeight() );
      return setWeight(
          "("+expression+") * {" + std::to_string(columns.size() ) + "}",
          newColumns,
          false);
    }
    else {
      m_weight = nameWeight();
      Define(m_weight, expression, columns);
      // Reset the weighted node statistics
      setupWeightedStatistics();
    }
    return shared_from_this();
  }

  Node::Node(
      const RNode& rnode,
      std::unique_ptr<IBranchNamer> namer,
      const std::string& name,
      const std::string& cutflowName) :
    m_rnodes({{namer->nominalName(), rnode}}),
    m_namer(std::move(namer) ),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(&m_rnodes.at(m_namer->nominalName() ) ),
    m_stats(m_namer->nominalName() ),
    m_weightedStats(m_namer->nominalName() )
  {
    m_namer->setNode(*this, true);
    m_stats.addResult(
        m_namer->nominalName(), 
        m_rnodes.at(m_namer->nominalName() ).Count() );
  }

  Node::Node(
      Node& parent,
      const std::string& name,
      const std::string& cutflowName) :
    m_parent(&parent),
    m_rnodes(parent.rnodes() ),
    m_namer(parent.namer().copy() ),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(parent.m_rootRNode),
    m_stats(m_namer->nominalName() ),
    m_weightedStats(m_namer->nominalName() ),
    m_weight(parent.getWeight() )
  {
    m_namer->setNode(*this, false);
    for (auto& nodePair : m_rnodes) {
      m_stats.addResult(nodePair.first, nodePair.second.Count() );
    }
    if (!m_weight.empty() )
      setupWeightedStatistics();
  }

  std::string Node::nameWeight()
  {
    // Construct the name of the node by hashing the pointer
    return "_NodeWeight_"+std::to_string(std::hash<Node*>()(this) )+"_";
  }

  void Node::setupWeightedStatistics()
  {
    // Clear out anything that was already there.
    m_weightedStats.reset();
    // Work out which systematics we care about
    std::set<std::string> affecting = m_namer->systematicsAffecting(m_weight);

    auto aggrFunc = [] (const std::pair<float, float>& lhs, float rhs)
    { return std::make_pair(lhs.first + rhs, lhs.second + rhs*rhs); };
    auto mergeFunc =
      [] (const std::pair<float, float>& lhs, const std::pair<float, float>& rhs)
    { return std::make_pair(lhs.first+rhs.first, lhs.second+rhs.second); };

    for (auto& rnodePair : m_rnodes) {
      m_weightedStats.addResult(
          rnodePair.first,
          rnodePair.second.Aggregate(
            aggrFunc,
            mergeFunc,
            m_namer->nameBranch(m_weight, rnodePair.first) ) );
      affecting.erase(rnodePair.first);
    }
    RNode& nominalNode = m_rnodes.at(m_namer->nominalName() );
    // For all the remaining systematics make them from the nominal
    for (const std::string& syst : affecting)
      m_weightedStats.addResult(
          syst,
          nominalNode.Aggregate(
            aggrFunc,
            mergeFunc,
            m_namer->nameBranch(m_weight, syst) ) );
  }

  std::shared_ptr<Node> createROOT(
      const RNode& rnode,
      std::unique_ptr<IBranchNamer>  namer,
      const std::string& name,
      const std::string& cutflowName)
  {
    return std::shared_ptr<Node>(
        new Node(rnode, std::move(namer), name, cutflowName) ); 
  } 
} //> end namespace RDFAnalysis
