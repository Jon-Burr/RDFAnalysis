#ifndef RDFAnalysis_IBranchNamer_H
#define RDFAnalysis_IBranchNamer_H

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <map>

#include <ROOT/RDataFrame.hxx>

/**
 * @file IBranchNamer.h
 * @brief The branch naming interface.
 */

namespace RDFAnalysis {
  /**
   * @brief Abstract base class that describes how a Node should name its
   * branches internally.
   *
   * The Node classes enforce a relationship between ROOT::RNode columns,
   * potentially having multiple variations (corresponding to different
   * systematics) of the same variable. These are stored as different columns in
   * the underlying ROOT:RNodes. Therefore the Node objects need to know how to
   * go from a variable name and a systematic name to the name of the
   * ROOT::RNode column.
   *
   * That mapping is provided by this class, along with other useful information
   * such as the full list of systematics, the name of the nominal systematic
   * and a list of all defined variables.
   */
  class IBranchNamer {
    public:
      virtual ~IBranchNamer() {}

      /**
       * @brief Get the full name of a branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       *
       * Search for a variation systName on a branch branch. If one
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
       *
       * Search for a variation systName on a branch branch. If one doesn't
       * exist then it will return the nominal branch. If that doesn't exist it
       * will throw a std::out_of_range exception.
       */
      virtual std::vector<std::string> nameBranches(
          const std::vector<std::string>& branches,
          const std::string& systName = "") const;

      /**
       * @brief Create a new branch
       * @param branch The base name of the branch
       * @param systName The name of the variation
       * @return The new branch name
       *
       * Create a new variation systName of branch branch. If this already
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

      /// Get the name of the nominal variation
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

      /**
       * @brief Expand a C++ expression into a pseudo-functional form.
       *
       * @param expression The expression to expand.
       * @return A pair, the first element is the pseudo-functional form, the
       * second the input variables to that function.
       *
       * This function receives an expression that may contain variable names
       * and then expands it into a form that can be varied for different
       * systematics. For instance a function 'jet_pt * cos(jet_phi)' (where
       * jet_pt and jet_phi are variables) would be expanded to '{0} *
       * cos({1})', {'jet_pt', 'jet_phi'}.
       */
      virtual std::pair<std::string, std::vector<std::string>> expandExpression(
          const std::string& expression) const;

      /**
       * @brief Interpret an expression for a given systematic variation.
       *
       * @param expression The pseudo-functional expression to use
       * @param branches The input variables to the expression
       * @param systematic The systematic variation to use
       * @return The expression for the given systematic
       *
       * Reinterpret a pseudo-functional form produced by expandExpression
       * for a specific systematic. For an expression '{0} * cos({1})' with
       * inputs {'jet_pt', 'jet_phi'} and systematic 'KIN_A' only affecting
       * jet_pt  (and making hopefully obvious assumptions about the nominal
       * name and the column naming pattern) this function would return 
       * 'KIN_A_jet_pt * cos(NOSYS_jet_phi)'
       */
      virtual std::string interpretExpression(
          const std::string& expression,
          const std::vector<std::string>& branches,
          const std::string& systematic);

  }; //> end class IBranchNamer
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_IBranchNamer_H
