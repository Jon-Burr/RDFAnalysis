#ifndef RDFAnalysis_SysVar_H
#define RDFAnalysis_SysVar_H

#include <string>
#include <type_traits>
#include <RDFAnalysis/IBranchNamer.h>

/**
 * @brief Helper functions and classes for adapting function arguments that
 * depend on the systematic being evaluated.
 *
 * Actions being applied to the Nodes need to be adapted and called for each
 * relevant systematic. Some of the arguments do not change through this process
 * but many do (for example, branch names and string expressions). We call this
 * process of changing arguments for different systematic calls of the same
 * action \a translation.
 *
 * The Node::Act and Node::ActResult functions determine whether or not to
 * translate an argument by using the \ref sysvar_traits struct which in turn
 * looks for a static constexpr bool member of the correspond argument class
 * type called is_rdf_sysvar. If this is present (and true) the argument will be
 * translated.
 *
 * For most purposes the SysVarBranch, SysVarBranchVector, SysVarNewBranch and
 * SysVarStringExpression classes provided here will be sufficient (in fact most
 * users will never need to even be aware of the existence of any of this), for
 * some actions it may be necessary to define additional argument classes
 * In order to do this, three things are necessary:
 *   -# A static constexpr bool member called is_rdf_sysvar which is set to true
 *   -# A translate(IBranchNamer&, const std::string&) function returning the
 *      translated argument
 *   -# A typedef called value_type which corresponds to the return type of the
 *      translate call
 */


namespace RDFAnalysis {

  /**
   * @brief Provide contextual information about a class.
   * @tparam T The class to provide information for.
   *
   * This is the default version that gets used for non-translatable arguments.
   */
  template <typename T, typename=void>
    struct sysvar_traits {
      static constexpr bool is_sysvar = false;
      using value_type = T;
    };

  /**
   * @brief Provide contextual information about a class.
   * @tparam T The class to provide information for.
   *
   * This version is used for translatable arguments.
   */
  template <typename T>
    struct sysvar_traits<T, std::enable_if_t<std::decay_t<T>::is_rdf_sysvar, void>>
    {
      static constexpr bool is_sysvar = true;
      using value_type = typename std::decay_t<T>::value_type;
    };

  /**
   * @brief Translate a variable
   * @tparam T The variable to translate
   *
   * This is called for translatable arguments and calls their translate
   * function.
   */
  template <typename T>
  std::enable_if_t<sysvar_traits<T>::is_sysvar, typename sysvar_traits<T>::value_type> sysVarTranslate(
      T&& t, IBranchNamer& namer, const std::string& syst) {
    return std::forward<typename sysvar_traits<T>::value_type>(t.translate(namer, syst) );
  }

  /**
   * @brief Translate a variable
   * @tparam T The variable to translate
   *
   * This is called for non-translatable arguments and just forwards the call.
   */
  template <typename T>
  std::enable_if_t<!sysvar_traits<T>::is_sysvar, T> sysVarTranslate(
      T&& t, IBranchNamer&, const std::string&) {
    return std::forward<T>(t);
  }

  /**
   * @brief Class to trigger translation of a single branch name
   */
  class SysVarBranch {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      /**
       * @brief Construct the branch
       * @param branchName The name of the untranslated branch
       */
      SysVarBranch(const std::string& branchName) :
        m_branch(branchName) {}

      /// Return the translated branch name
      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.nameBranch(m_branch, syst);
      }

    private:
      /// The untranslated branch
      std::string m_branch;
  };

  /**
   * @brief Class to trigger translation of a vector of branches
   */
  class SysVarBranchVector {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::vector<std::string>;

      /**
       * @brief Construct the vector
       * @param branchNames The names of the untranslated branches
       */
      SysVarBranchVector(const std::vector<std::string>& branchNames) :
        m_branchNames(branchNames) {}

      /// Return the translated branch names
      std::vector<std::string> translate(IBranchNamer& namer, const std::string& syst) {
        return namer.nameBranches(m_branchNames, syst);
      }

    private:
      /// The untranslated branches
      std::vector<std::string> m_branchNames;
  };

  /**
   * @brief Class to trigger translation of a new branch name.
   */
  class SysVarNewBranch {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      /**
       * @brief Create the branch
       * @param The untranslated branch name
       */
      SysVarNewBranch(const std::string& branchName) :
        m_branch(branchName) {}

      /// Return the translated branch name
      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.createBranch(m_branch, syst);
      }

    private:
      /// The untranslated branch names
      std::string m_branch;
  };

  /**
   * @brief Class to trigger translation of a string expression
   */
  class SysVarStringExpression {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      /**
       * @brief Construct the expression
       * @param expressionTemplate The pseudo-functional form
       * @param columnNames The input variables
       *
       * \ref expressionTemplate should be returned by the
       * IBranchNamer::expandExpression function.
       */
      SysVarStringExpression(
          const std::string& expressionTemplate,
          const std::vector<std::string>& columnNames) :
        m_template(expressionTemplate),
        m_columns(columnNames) {}

      /// Return the translated expression
      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.interpretExpression(m_template, m_columns, syst);
      }

    private:
      /// The expression template
      std::string m_template;
      /// The input columns to the expression
      std::vector<std::string> m_columns;
  };
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_SysVar_H
