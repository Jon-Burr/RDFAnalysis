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
          m_namer->nameBranch(name, rnode.first, true, true), systExpression);
    }
    // Add any left over to the nominal
    RNode& nominal = m_rnodes.at(m_namer->nominalName());
    for (const std::string& syst : affecting) {
      std::string systExpression = m_namer->interpretExpression(
          expression, columns, syst);
      nominal = nominal.Define(
          m_namer->nameBranch(name, syst, true, false), systExpression);
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
        m_rnodes.at(m_namer->nominalName() ).Book(
          NodeStatistics(
            std::make_shared<ULong64_t>(),
            getNSlots() ) ) );
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
    m_weightedStats(m_namer->nominalName() )
  {
    m_namer->setNode(*this, false);
    for (auto& nodePair : m_rnodes) {
      m_stats.addResult(
          nodePair.first,
          nodePair.second.Book(
            NodeStatistics(
              std::make_shared<ULong64_t>(),
              getNSlots() ) ) );
    }
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
