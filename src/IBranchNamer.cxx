#include "RDFAnalysis/IBranchNamer.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <algorithm>
#include <regex>

namespace RDFAnalysis {

  std::set<std::string> IBranchNamer::systematicsAffecting(
      const std::vector<std::string>& branches) const
  {
    std::set<std::string> allAffecting;
    for (const std::string& branch : branches) {
      std::set<std::string> affecting = systematicsAffecting(branch);
      allAffecting.insert(affecting.begin(), affecting.end() );
    }
    return allAffecting;
  }

  std::pair<std::string, std::vector<std::string>> IBranchNamer::expandExpression(
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

  std::string IBranchNamer::interpretExpression(
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
