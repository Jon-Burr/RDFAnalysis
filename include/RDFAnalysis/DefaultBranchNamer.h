#ifndef RDFAnalysis_BranchNamer_H
#define RDFAnalysis_BranchNamer_H

// STL includes
#include <map>
#include <string>
#include <vector>
#include <set>
#include <memory>

// ROOT includes
#include <ROOT/RDataFrame.hxx>

namespace RDFAnalysis {
  class DefaultBranchNamer {
    public:
      /**
      * @brief Construct the namer
      * @param systematics The list of all variations
      * @param systNameFirst When building column names whether to put the
      * sytematic name first.
      * @param inputFromFriends Whether or not the inputs come from friend
      * trees.
      * @param nominalName The name of the nominal systematic
      */
      DefaultBranchNamer(
          const std::vector<std::string>& systematics,
          bool systNameFirst = true,
          bool inputFromFriends = true,
          const std::string& nominalName = "NOSYS") :
        m_systematics(systematics),
        m_systNameFirst(true),
        m_inputFromFriendTrees(true),
        m_nominalName(nominalName) {}

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       * Search for a variation \ref systName on a branch \ref branch. If one
       * doesn't exist then it will return the nominal branch. If that doesn't
       * exist it will throw a std::out_of_range exception.
       */
      std::string nameBranch(
          const std::string& branch,
          const std::string& systName = "") const;

      /**
       * @brief Get the full names of a list of branches
       * @param branches The base name of the branches
       * @param systName The name of the variation
       * Search for a variation \ref systName on a branch \ref branch. If one
       * doesn't exist then it will return the nominal branch. If that doesn't
       * exist it will throw a std::out_of_range exception.
       */
      std::vector<std::string> nameBranches(
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
      std::string createBranch(
          const std::string& branch,
          const std::string& systName = "",
          bool isRNodeSys = false);

      /**
       * @brief Test if a specific variation of a specific branch exists
       * @param branch The base name of the branch
       * @param systName The name of the variation
       */
      bool exists(
          const std::string& branch,
          const std::string& systName = "") const;

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       * @param isRNodeSys If this branch is to be added to a ROOT::RNode
       * specific to this systematic.
       */
      std::string newBranchName(
          const std::string& branch,
          const std::string& systName = "",
          bool isRNodeSys = false) const;

      const std::string& nominalName() const { return m_nominalName; }

      /**
       * @brief Get all systematics.
       */
      std::vector<std::string> systematics() const
      { return m_systematics; }

      /**
       * @brief Get all systematics affecting a base branch name.
       * @param branch The name of the branch to test
       */
      std::set<std::string> systematicsAffecting(
          const std::string& branch) const;

      /**
       * @brief Get all the systematics affecting a set of columns.
       * @param branches The names of the branches to test
       */
      std::set<std::string> systematicsAffecting(
          const std::vector<std::string>& branches) const;

      /**
       * @brief Get all branch base names
       */
      std::vector<std::string> branches() const;

      std::pair<std::string, std::vector<std::string>> expandExpression(
          const std::string& expression) const;

      std::string interpretExpression(
          const std::string& expression,
          const std::vector<std::string>& branches,
          const std::string& systematic);

      /**
       * @brief Set the node that this namer is looking at
       * @param rnodes The input rnodes.
       */
       void readBranchList( const std::map<std::string, ROOT::RDF::RNode>& rnodes );
    private:

      /// The branches
      std::map<std::string, std::map<std::string, std::string>> m_branches;

      /// The list of systematics
      std::vector<std::string> m_systematics;

      /// Whether when naming new branches (or reading existing ones) the
      /// systematic name should come first.
      bool m_systNameFirst{true};

      /// Whether the input is being read from friend trees
      bool m_inputFromFriendTrees{true};

      /// The name of the nominal variation
      std::string m_nominalName{"NOSYS"};

  }; //> end class BranchNamer
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_BranchNamer_H
