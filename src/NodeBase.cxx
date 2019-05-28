#include "RDFAnalysis/NodeBase.h"
#include <typeinfo>

namespace RDFAnalysis {
  NodeBase* NodeBase::Define(
      const std::string& name,
      const std::string& expression)
  {
    auto expanded = m_namer->expandExpression(expression);
    return Define(name, expanded.first, expanded.second);
  }

  NodeBase* NodeBase::Define(
      const std::string& name,
      const std::string& expression,
      const ColumnNames_t& columns)
  {
    // We don't actually use the output of the action so we don't store it.
    // However we need there to be a return value for the lambda.
    // Note that this action updates the node passed in.
    Act([] (RNode& rnode, const std::string& name, const std::string& expression) {
        return rnode = rnode.Define(name, expression); },
        columns,
        SysVarNewBranch(name),
        SysVarStringExpression(expression, columns) );
    return this;
  }

  std::map<std::string, RNode> NodeBase::makeChildRNodes(
      const std::string& expression,
      const std::string& cutflowName)
  {
    auto expanded = m_namer->expandExpression(expression);
    return makeChildRNodes(expanded.first, expanded.second, cutflowName);
  }

  std::map<std::string, RNode> NodeBase::makeChildRNodes(
      const std::string& expression,
      const ColumnNames_t& columns,
      const std::string& cutflowName)
  {
    return Act(
        [] (RNode& rnode, const std::string& expression, const std::string& cutflowName) -> RNode{
        return rnode.Filter(expression, cutflowName); },
        columns,
        SysVarStringExpression(expression, columns),
        cutflowName);
  }

  std::string NodeBase::setWeight(
      const std::string& expression,
      NodeBase* parent,
      WeightStrategy strategy)
  {
    if (expression.empty() ||
        (!isMC() && !!(strategy & WeightStrategy::MCOnly ) ) )
      // If we've not provided an expression or this is an MC-only weight and
      // we're not in the MC mode then do whatever the parent is doing. If
      // there's no parent then just use the empty string (no weight)
      return parent ? parent->getWeight() : "";

    if (!!(strategy & WeightStrategy::Multiplicative) && 
        parent && parent->getWeight() != "")
      // If we've requested a multiplicative weight and there is a parent weight
      // by which to multiply call this again with the multiplied weight and the
      // multiplicative part of the weight turned off
      return setWeight(
          "("+expression+") * " + parent->getWeight(),
          parent,
          strategy & ~WeightStrategy::Multiplicative);

    // If we reach here then we no longer need to worry about the strategy -
    // we've already taken care of all of that
    if (m_namer->exists(expression) )
      // For this version only, if the new weight is already a column, no need
      // to recalculate it...
      return expression;
    // Otherwise create the new branch and return it
    std::string weight = nameWeight();
    Define(weight, expression);
    return weight;
  }

  NodeBase::NodeBase(
      const RNode& rnode,
      std::unique_ptr<IBranchNamer>&& namer,
      bool isMC,
      const std::string& name,
      const std::string& cutflowName,
      const std::string& weight,
      WeightStrategy strategy) :
    m_rnodes({{namer->nominalName(), rnode}}),
    m_namer(std::move(namer) ),
    m_namerInit(*m_namer, m_rnodes),
    m_isMC(isMC),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(&m_rnodes.at(m_namer->nominalName() ) ),
    m_weight(setWeight(weight, nullptr, strategy) )
  {
  }

  NodeBase::NodeBase(
      NodeBase& parent,
      std::map<std::string, RNode>&& rnodes,
      const std::string& name,
      const std::string& cutflowName,
      const std::string& weight,
      WeightStrategy strategy) :
    m_rnodes(std::move(rnodes) ),
    m_namer(parent.namer().copy() ),
    m_isMC(parent.isMC() ),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(parent.m_rootRNode),
    m_weight(setWeight(weight, &parent, strategy) )
  {
  }

  std::string NodeBase::nameWeight()
  {
    // Construct the name of the node by hashing the pointer
    return "_NodeWeight_"+std::to_string(std::hash<NodeBase*>()(this) )+"_";
  }
}; //> enad namespace RDFAnalysis
