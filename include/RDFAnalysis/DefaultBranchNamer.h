#ifndef RDFAnalysis_BranchNamer_H
#define RDFAnalysis_BranchNamer_H

// STL includes
#include <map>

// package includes
#include <RDFAnalysis/IBranchNamer.h>

/**
 * @file DefaultBranchNamer.h
 * @brief Default implementation of the IBranchNamer interface.
 */

namespace RDFAnalysis {
  /**
   * @brief Default implementation of the IBranchNamer interface.
   *
   * Allows for reading and writing branches of the form SYSNAME_BRANCHNAME or
   * BRANCHNAME_SYSNAME. Can also read in files where the different systematics
   * are saved as friend trees. In this case the input format is
   * SYSNAME.BRANCHNAME.
   */
  class DefaultBranchNamer : public IBranchNamer {
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
          bool inputFromFriends = false,
          const std::string& nominalName = "NOSYS") :
        m_systematics(systematics),
        m_systNameFirst(systNameFirst),
        m_inputFromFriendTrees(inputFromFriends),
        m_nominalName(nominalName) {}

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       *
       * Search for a variation systName on a branch branch. If one doesn't
       * exist then it will return the nominal branch. If that doesn't exist it
       * will throw a std::out_of_range exception.
       */
      std::string nameBranch(
          const std::string& branch,
          const std::string& systName = "") const override;

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       *
       * Create a new variation systName of branch branch. If this already
       * exists then a std::runtime_error will be thrown.
       */
      std::string createBranch(
          const std::string& branch,
          const std::string& systName = "") override;

      /**
       * @brief Test if a specific variation of a specific branch exists
       * @param branch The base name of the branch
       * @param systName The name of the variation
       */
      bool exists(
          const std::string& branch,
          const std::string& systName = "") const override;

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       */
      std::string newBranchName(
          const std::string& branch,
          const std::string& systName = "") const;

      /// Print the name of the nominal variation
      const std::string& nominalName() const override { return m_nominalName; }

      /**
       * @brief Get all systematics.
       */
      std::vector<std::string> systematics() const override
      { return m_systematics; }

      /**
       * @brief Get all systematics affecting a base branch name.
       * @param branch The name of the branch to test
       */
      std::set<std::string> systematicsAffecting(
          const std::string& branch) const override;

      /**
       * @brief Get all branch base names
       */
      std::vector<std::string> branches() const override;

      /**
       * @brief Set the node that this namer is looking at
       * @param rnodes The input rnodes.
       */
       void readBranchList( const std::map<std::string, ROOT::RDF::RNode>& rnodes ) override;

       std::unique_ptr<IBranchNamer> copy() const override
       { return std::make_unique<DefaultBranchNamer>(*this); }
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

  }; //> end class DefaultBranchNamer
} //> end namespace RDFAnalysis
#endif //gb> !RDFAnalysis_BranchNamer_H
