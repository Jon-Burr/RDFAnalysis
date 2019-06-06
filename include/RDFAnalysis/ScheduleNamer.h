#ifndef RDFAnalysis_ScheduleNamer_H
#define RDFAnalysis_ScheduleNamer_H

#include "RDFAnalysis/IBranchNamer.h"

/**
 * @file ScheduleNamer.h
 * @brief The namer used by the scheduler
 */

namespace RDFAnalysis {
  /**
   * @brief Namer class to be used by the scheduler.
   *
   * This is required to recognise variables in the scheduling step.
   */
  class ScheduleNamer : public IBranchNamer {
    public:

      ScheduleNamer(const IBranchNamer& other) :
        m_branches(other.branches() ),
        m_nominal("") {}

      ~ScheduleNamer() {}

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       *
       * The scheduler does not know about systematics so just returns the same
       * name.
       */
      std::string nameBranch(
          const std::string& branch,
          const std::string& = "") const override { return branch; }

      /**
       * @brief Create a new branch
       * @param branch The base name of the branch
       */
      std::string createBranch(
          const std::string& branch,
          const std::string& = "") override
      { 
        m_branches.push_back(branch);
        return branch;
      }

      /**
       * @brief Test if a specific variation of a specific branch exists
       * @param branch The base name of the branch
       * @param systName The name of the variation
       */
      bool exists(
          const std::string& branch,
          const std::string& = "") const override
      { return std::count(m_branches.begin(), m_branches.end(), branch) > 0; }

      const std::string& nominalName() const override { return m_nominal;}

      /**
       * @brief Get all systematics.
       */
      std::vector<std::string> systematics() const override
      { return std::vector<std::string>{}; }

      /**
       * @brief Get all systematics affecting a base branch name.
       * @param branch The name of the branch to test
       */
      std::set<std::string> systematicsAffecting(
          const std::string& = "") const override
      { return std::set<std::string>{}; }

      /**
       * @brief Get all branch base names
       */
      std::vector<std::string> branches() const override
      { return m_branches; }

      /**
       * @brief Read branch lists from a set of rnodes
       */
      void readBranchList(
          const std::map<std::string, ROOT::RDF::RNode>&) override {}

      /// Make a copy of this class
      std::unique_ptr<IBranchNamer> copy() const override
      { return std::make_unique<ScheduleNamer>(*this); }

    private:
      /// The branches
      std::vector<std::string> m_branches;
      /// The (dummy) nominal systematic
      std::string m_nominal;

  }; //> end class ScheduleNamer
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_ScheduleNamer_H
