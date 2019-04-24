#include "RDFAnalysis/DefaultBranchNamer.h"
#include "RDFAnalysis/Node.h"
#include <set>
#include <regex>
#include <algorithm>

// Anonymous namespace for helpers
// TODO - replace with boost
namespace {
  std::string strJoin(
      const std::vector<std::string>& inputs,
      std::string delim = "|")
  {
    if (inputs.size() == 0)
      return "";
    std::string output = inputs.at(0);
    for (auto itr = inputs.begin() + 1; itr != inputs.end(); ++itr)
      output += delim + *itr;
    return output;
  }
}

namespace RDFAnalysis {
  std::string DefaultBranchNamer::nameBranch(
      const std::string& branch,
      const std::string& systNameIn,
      bool create,
      bool isRNodeSys)
  {
    std::string systName = systNameIn.empty() ? m_nominalName : systNameIn;
    if (std::find(m_systematics.begin(), m_systematics.end(), systName)
        == m_systematics.end() ) {
      throw std::out_of_range("Unknown variation " + systName);
    }
    auto branchItr = m_branches.find(branch);
    if (branchItr == m_branches.end() ) {
      if (create) {
        std::string newBranch = newBranchName(branch, systName, isRNodeSys);
        m_branches[branch][systName] = newBranch;
        return newBranch;
      }
      else 
        throw std::out_of_range(
            "Branch " + branch + " requested but this branch does not exist!");
    }
    // Look for this variation of the branch
    auto systItr = branchItr->second.find(systName);
    if (systItr == branchItr->second.end() ) {
      if (create) {
        std::string newBranch = newBranchName(branch, systName, isRNodeSys);
        branchItr->second[systName] = newBranch;
        return newBranch;
      }
      else {
        // If it doesn't exist, look for the nominal
        systItr = branchItr->second.find(m_nominalName);
        if (systItr == branchItr->second.end() )
          throw std::out_of_range(
              "No nominal variation exists for branch " + branch );
      }
    }
    return systItr->second;
  }

  bool DefaultBranchNamer::exists(
      const std::string& branch,
      const std::string& systNameIn) const
  {
    std::string systName = systNameIn.empty() ? m_nominalName : systNameIn;
    auto branchItr = m_branches.find(branch);
    if (branchItr == m_branches.end() )
      return false;
    auto systItr = branchItr->second.find(systName);
    return systItr != branchItr->second.end();
  }

  std::string DefaultBranchNamer::newBranchName(
      const std::string& branch,
      const std::string& systNameIn,
      bool isRNodeSys) const
  {
    if (isRNodeSys) {
      // If the RNode is specific to the systematic there's no need to
      // disambiguate it.
      return branch;
    }
    else {
      std::string systName = systNameIn.empty() ? m_nominalName : systNameIn;
      if (m_systNameFirst)
        return systName + "_" + branch;
      else
        return branch + "_" + systName;
    }
  }

  std::set<std::string> DefaultBranchNamer::systematicsAffecting(
      const std::string& branch) const
  {
    auto itr = m_branches.find(branch);
    if (itr == m_branches.end() )
      return {};
    else {
      std::set<std::string> systs;
      for (const auto& systPair : itr->second)
        systs.insert(systPair.first);
      return systs;
    }
  }

  std::vector<std::string> DefaultBranchNamer::branches() const
  {
    std::vector<std::string> branchNames;
    for (const auto& branchPair : m_branches)
      branchNames.push_back(branchPair.first);
    return branchNames;
  }

  void DefaultBranchNamer::refresh()
  {
    // Clear our current branches
    m_branches.clear();
    if (!m_parent)
      return;
    std::string fullSystPattern = strJoin(m_systematics, "|");
    std::regex inputPattern(m_inputFromFriendTrees 
      ? "^("+fullSystPattern+")\\.(\\w+)$"
      : (m_systNameFirst 
        ? "^("+fullSystPattern+")_(\\w+)$"
        : "^(\\w+)_("+fullSystPattern+")$")); // TODO non greedy?
    std::regex internalPattern(m_systNameFirst
      ? "^("+fullSystPattern+")_(\\w+)$"
      : "^(\\w+)_("+fullSystPattern+")$"); // TODO non greedy?
    std::size_t inputBranchIndex = 
      (m_inputFromFriendTrees || m_systNameFirst) ? 2 : 1;
    std::size_t inputSystIndex = 3 - inputBranchIndex;
    std::size_t internalBranchIndex = m_systNameFirst ? 2 : 1;
    std::size_t internalSystIndex = 3 - internalBranchIndex;
    for (const auto& rnodePair : m_parent->rnodes() ) {
      // Eurgh - why on earth is GetColumnNames not const!?
      ROOT::RDF::RNode& rnode = const_cast<ROOT::RDF::RNode&>(rnodePair.second);
      for (const std::string& column : rnode.GetColumnNames() ) {
        std::smatch sm;
        if (std::regex_match(column, sm, inputPattern) )
          m_branches[sm[inputBranchIndex]][sm[inputSystIndex]] = column;
        else if (std::regex_match(column, sm, internalPattern) )
          m_branches[sm[internalBranchIndex]][sm[internalSystIndex]] = column;
        else
          m_branches[column][rnodePair.first] = column;
      }
    }
  }

  void DefaultBranchNamer::setNode(
      const Node& node,
      bool doRefresh)
  {
    m_parent = &node;
    if (doRefresh)
      refresh();
  }

  std::unique_ptr<IBranchNamer> DefaultBranchNamer::copy() const
  { 
    return std::make_unique<DefaultBranchNamer>(*this);
  }
} //> end namespace RDFAnalysis
