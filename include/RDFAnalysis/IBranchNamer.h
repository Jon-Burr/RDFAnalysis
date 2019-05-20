#ifndef RDFAnalysis_IBranchNamer_H
#define RDFAnalysis_IBranchNamer_H

// TODO documentation
// TODO const version of name branch that only gets you branches that exist

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <map>

#include <ROOT/RDataFrame.hxx>

namespace RDFAnalysis {
  class IBranchNamer {
    public:
      virtual ~IBranchNamer() {}

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       * Search for a variation \ref systName on a branch \ref branch. If one
       * doesn't exist then it will return the nominal branch. If that doesn't
       * exist it will throw a std::out_of_range exception.
       */
      virtual std::string nameBranch(
          const std::string& branch,
          const std::string& systName = "") const = 0;

      /**
       * @brief Get the full names of a list of branches
       * @param branches The base name of the branches
       * @param systName The name of the variation
       * Search for a variation \ref systName on a branch \ref branch. If one
       * doesn't exist then it will return the nominal branch. If that doesn't
       * exist it will throw a std::out_of_range exception.
       */
      virtual std::vector<std::string> nameBranches(
          const std::vector<std::string>& branches,
          const std::string& systName = "") const;

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       * @param isRNodeSys whether the ROOT::RNode the new branch will be added
       * to is specific to this systematic.
       * Create a new variation \systName of branch \ref branch. If this already
       * exists then a std::runtime_error will be thrown.
       */
      virtual std::string createBranch(
          const std::string& branch,
          const std::string& systName = "") = 0;

      /**
       * @brief Test if a specific variation of a specific branch exists
       * @param branch The base name of the branch
       * @param systName The name of the variation
       */
      virtual bool exists(
          const std::string& branch,
          const std::string& systName = "") const = 0;

      virtual const std::string& nominalName() const = 0;

      /**
       * @brief Get all systematics.
       */
      virtual std::vector<std::string> systematics() const = 0;

      /**
       * @brief Get all systematics affecting a base branch name.
       * @param branch The name of the branch to test
       */
      virtual std::set<std::string> systematicsAffecting(
          const std::string& branch) const = 0;

      /**
       * @brief Get all the systematics affecting a set of columns.
       * @param branches The names of the branches to test
       */
      virtual std::set<std::string> systematicsAffecting(
          const std::vector<std::string>& branches) const;

      /**
       * @brief Get all branch base names
       */
      virtual std::vector<std::string> branches() const = 0;

      /**
       * @brief Read branch lists from a set of rnodes
       * @param rnodes The input rnodes
       */
      virtual void readBranchList(
          const std::map<std::string, ROOT::RDF::RNode>& rnodes) = 0;

      /// Make a copy of this class
      virtual std::unique_ptr<IBranchNamer> copy() const = 0;

      virtual std::pair<std::string, std::vector<std::string>> expandExpression(
          const std::string& expression) const;

      virtual std::string interpretExpression(
          const std::string& expression,
          const std::vector<std::string>& branches,
          const std::string& systematic);

  }; //> end class IBranchNamer
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_IBranchNamer_H
