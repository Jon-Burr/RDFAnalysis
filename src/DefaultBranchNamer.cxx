#include "RDFAnalysis/DefaultBranchNamer.h"
#include "RDFAnalysis/Node.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
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
      const std::string& systNameIn) const
  {
    std::string systName = systNameIn.empty() ? m_nominalName : systNameIn;
    if (std::find(m_systematics.begin(), m_systematics.end(), systName)
        == m_systematics.end() ) {
      throw std::out_of_range("Unknown variation " + systName);
    }
    auto branchItr = m_branches.find(branch);
    if (branchItr == m_branches.end() )
      throw std::out_of_range(
          "Branch " + branch + " requested but this branch does not exist!");
    // Look for this variation of the branch
    auto systItr = branchItr->second.find(systName);
    if (systItr == branchItr->second.end() ) {
      // If it doesn't exist, look for the nominal
      systItr = branchItr->second.find(m_nominalName);
      if (systItr == branchItr->second.end() )
        throw std::out_of_range(
            "No nominal variation exists for branch " + branch );
    }
    return systItr->second;
  }

  std::vector<std::string> DefaultBranchNamer::nameBranches(
      const std::vector<std::string>& branches,
      const std::string& systName) const
  {
    std::vector<std::string> out;
    out.reserve(branches.size() );
    for (const std::string& branch : branches)
      out.push_back(nameBranch(branch, systName) );
    return out;
  }

  std::string DefaultBranchNamer::createBranch(
      const std::string& branch,
      const std::string& systNameIn,
      bool isRNodeSys)
  {
    std::string systName = systNameIn.empty() ? m_nominalName : systNameIn;
    if (std::find(m_systematics.begin(), m_systematics.end(), systName)
        == m_systematics.end() ) {
      throw std::out_of_range("Unknown variation " + systName);
    }
    auto branchItr = m_branches.find(branch);
    if (branchItr == m_branches.end() ) {
      std::string newBranch = newBranchName(branch, systName, isRNodeSys);
      m_branches[branch][systName] = newBranch;
      return newBranch;
    }
    // Look for this variation of the branch
    auto systItr = branchItr->second.find(systName);
    if (systItr == branchItr->second.end() ) {
      std::string newBranch = newBranchName(branch, systName, isRNodeSys);
      branchItr->second[systName] = newBranch;
      return newBranch;
    }
    else 
      throw std::runtime_error("Trying to create variation " + systName +
          " of branch " + branch + " but this already exists!");
    return "";
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

  std::set<std::string> DefaultBranchNamer::systematicsAffecting(
      const std::vector<std::string>& branches) const
  {
    std::set<std::string> allAffecting;
    for (const std::string& branch : branches) {
      std::set<std::string> affecting = systematicsAffecting(branch);
      allAffecting.insert(affecting.begin(), affecting.end() );
    }
    return allAffecting;
  }

  std::vector<std::string> DefaultBranchNamer::branches() const
  {
    std::vector<std::string> branchNames;
    for (const auto& branchPair : m_branches)
      branchNames.push_back(branchPair.first);
    return branchNames;
  }

  void DefaultBranchNamer::readBranchList(
      const std::map<std::string, RNode>& rnodes)
  {
    // Clear our current branches
    m_branches.clear();
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
    for (const auto& rnodePair : rnodes ) {
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

  std::pair<std::string, std::vector<std::string>> DefaultBranchNamer::expandExpression(
      const std::string& expression) const
  {
    // Get all the base branch names
    std::vector<std::string> names = branches();
    // Sort them according to length (longest first)
    std::sort(names.begin(), names.end(), 
        [] (const std::string& lhs, const std::string& rhs)
        { return lhs.size() > rhs.size(); });
    // Build a regexp that matches any branch name. We sorted the branches
    // because in an alternating match regex favours the earliest option it
    // sees.
    std::regex regexpr("("+boost::algorithm::join(names, "|")+")\\b");
    // Now go through the expression and replace anything that is a branch name
    // with a placeholder. The placeholder is of the form '{i}' where i is the
    // index in the vector of branch names we will output
    std::vector<std::string> usedBranchNames;
    std::string newExp = expression;
    std::smatch sm;
    while (std::regex_search(newExp, sm, regexpr) ) {
      std::string branch = sm.str(1);
      std::size_t idx = usedBranchNames.size();
      usedBranchNames.push_back(branch);
      boost::algorithm::replace_all(newExp, branch, "{"+std::to_string(idx)+"}");
    }
    return std::make_pair(newExp, std::move(usedBranchNames) );
  }

  std::string DefaultBranchNamer::interpretExpression(
      const std::string& expression,
      const std::vector<std::string>& branches,
      const std::string& systematic)
  {
    std::regex regexpr("\\{(\\d+)\\}");
    std::string newExp = expression;
    std::smatch sm;
    while (std::regex_search(newExp, sm, regexpr) ) {
      std::size_t idx = std::stoi(sm.str(1) );
      std::string branch = branches.at(idx);
      std::string systBranch = nameBranch(branch, systematic);
      boost::algorithm::replace_all(newExp, sm.str(0), systBranch);
    }
    return newExp;
  }
} //> end namespace RDFAnalysis
