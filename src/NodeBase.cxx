#include "RDFAnalysis/NodeBase.h"

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
      const std::string& parentWeight)
  {
    if (expression.empty() )
      // If we've not provided an expression then use the parent's weight if
      // it's there, otherwise keep an empty string
      return parentWeight;
    if (!parentWeight.empty() )
      // Adapt the expression to include the parent weight and then call this
      // function again with the new expression and no parentWeight
      return setWeight("("+expression+") * " + parentWeight, "");
    std::string weight;
    if (m_namer->exists(expression) ) {
      // For this version only, if the new weight is already a column, no need
      // to recalculate it...
      weight = expression;
    }
    else {
      weight = nameWeight();
      Define(weight, expression);
    }
    return weight;
  }

  NodeBase::NodeBase(
      const RNode& rnode,
      std::unique_ptr<IBranchNamer>&& namer,
      const std::string& name,
      const std::string& cutflowName,
      const std::string& weight) :
    m_rnodes({{namer->nominalName(), rnode}}),
    m_namer(std::move(namer) ),
    m_namerInit(*m_namer, m_rnodes),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(&m_rnodes.at(m_namer->nominalName() ) ),
    m_weight(setWeight(weight, "") )
    /* m_stats(m_namer->nominalName() ), */
    /* m_weightedStats(m_namer->nominalName() ) */
  {
    /* m_stats.addResult( */
    /*     m_namer->nominalName(), */ 
    /*     m_rnodes.at(m_namer->nominalName() ).Count() ); */
    /* if (!m_weight.empty() ) */
    /*   setupWeightedStatistics(); */
  }

  NodeBase::NodeBase(
      NodeBase& parent,
      std::map<std::string, RNode>&& rnodes,
      const std::string& name,
      const std::string& cutflowName) :
    m_rnodes(std::move(rnodes) ),
    m_namer(parent.m_namer->copy() ),
    m_name(name),
    m_cutflowName(cutflowName),
    m_rootRNode(parent.m_rootRNode),
    m_weight(parent.getWeight() )
    /* m_stats(m_namer->nominalName() ), */
    /* m_weightedStats(m_namer->nominalName() ) */
  {
    for (auto& nodePair : m_rnodes) {
    }
    /* if (!m_weight.empty() ) */
    /*   setupWeightedStatistics(); */
  }

  std::string NodeBase::nameWeight()
  {
    // Construct the name of the node by hashing the pointer
    return "_NodeWeight_"+std::to_string(std::hash<NodeBase*>()(this) )+"_";
  }

/*   void NodeBase::setupWeightedStatistics() */
/*   { */
/*     // Clear out anything that was already there. */
/*     m_weightedStats.reset(); */
/*     // Work out which systematics we care about */
/*     std::set<std::string> affecting = m_namer->systematicsAffecting(m_weight); */

/*     auto aggrFunc = [] (const std::pair<float, float>& lhs, float rhs) */
/*     { return std::make_pair(lhs.first + rhs, lhs.second + rhs*rhs); }; */
/*     auto mergeFunc = */
/*       [] (const std::pair<float, float>& lhs, const std::pair<float, float>& rhs) */
/*     { return std::make_pair(lhs.first+rhs.first, lhs.second+rhs.second); }; */

/*     m_weightedStats = Aggregate(aggrFunc, mergeFunc, m_weight); */

/*     /1* for (auto& rnodePair : m_rnodes) { *1/ */
/*     /1*   m_weightedStats.addResult( *1/ */
/*     /1*       rnodePair.first, *1/ */
/*     /1*       rnodePair.second.Aggregate( *1/ */
/*     /1*         aggrFunc, *1/ */
/*     /1*         mergeFunc, *1/ */
/*     /1*         m_namer->nameBranch(m_weight, rnodePair.first) ) ); *1/ */
/*     /1*   affecting.erase(rnodePair.first); *1/ */
/*     /1* } *1/ */
/*     /1* RNode& nominalNode = m_rnodes.at(m_namer->nominalName() ); *1/ */
/*     /1* // For all the remaining systematics make them from the nominal *1/ */
/*     /1* for (const std::string& syst : affecting) *1/ */
/*     /1*   m_weightedStats.addResult( *1/ */
/*     /1*       syst, *1/ */
/*     /1*       nominalNode.Aggregate( *1/ */
/*     /1*         aggrFunc, *1/ */
/*     /1*         mergeFunc, *1/ */
/*     /1*         m_namer->nameBranch(m_weight, syst) ) ); *1/ */
/*   } */
}; //> enad namespace RDFAnalysis
