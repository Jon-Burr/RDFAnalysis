#ifndef RDFAnalysis_SysVar_H
#define RDFAnalysis_SysVar_H

#include <string>
#include <type_traits>
#include <RDFAnalysis/IBranchNamer.h>

namespace RDFAnalysis {
  template <typename T, typename=void>
    struct sysvar_traits {
      static constexpr bool is_sysvar = false;
      using value_type = T;
    };

  template <typename T>
    struct sysvar_traits<T, std::enable_if_t<std::decay_t<T>::is_rdf_sysvar, void>>
    {
      static constexpr bool is_sysvar = true;
      using value_type = typename std::decay_t<T>::value_type;
    };



  template <typename T>
  std::enable_if_t<sysvar_traits<T>::is_sysvar, typename sysvar_traits<T>::value_type> sysVarTranslate(
      T&& t, IBranchNamer& namer, const std::string& syst) {
    return std::forward<typename sysvar_traits<T>::value_type>(t.translate(namer, syst) );
  }

  template <typename T>
  std::enable_if_t<!sysvar_traits<T>::is_sysvar, T> sysVarTranslate(
      T&& t, IBranchNamer&, const std::string&) {
    return std::forward<T>(t);
  }

/*   template <typename T, bool B=is_sysvar<T>::value> */
/*     struct sysvar_traits; */

/*   template <typename T> */
/*     struct sysvar_traits<T, false> { */
/*       using value_type = T; */
/*       static value_type translate(T t, IBranchNamer&, const std::string&) */
/*       { return t; } */
/*     }; */

/*   template <typename T> */
/*     struct sysvar_traits<T, true> { */
/*       using value_type = std::decay_t<typename std::result_of<decltype(&std::decay_t<T>::translate)(std::decay_t<T>, IBranchNamer&, const std::string&)>::type>; */
/*       static value_type translate(std::decay_t<T> t, IBranchNamer& namer, const std::string& syst) */ 
/*       { return t.translate(namer, syst); } */
/*     }; */

/*   template <typename T> */
/*     typename sysvar_traits<T>::value_type sysVarTranslate(T&& t, IBranchNamer& namer, const std::string& syst) { */
/*       return sysvar_traits<T>::translate(std::forward<T>(t), namer, syst); */
/*     } */

/*   template <typename... Ts> */
/*     using sysVarTuple_t = std::tuple<typename sysvar_traits<Ts>::value_type...>; */

/*   template <typename... Ts, std::size_t... Is> */
/*     sysVarTuple_t<Ts...> sysVarTranslateTupleImpl(const std::tuple<Ts...>& args, IBranchNamer& namer, const std::string& syst, std::index_sequence<Is...>) { */
/*       return sysVarTuple_t<Ts...>(sysVarTranslate(std::get<Is>(args), namer, syst)...); */
/*     } */

/*   template <typename... Ts> */
/*     sysVarTuple_t<Ts...> sysVarTranslateTuple(const std::tuple<Ts...>& args, IBranchNamer& namer, const std::string& syst) { */
/*       return sysVarTranslateTupleImpl(args, namer, syst, std::make_index_sequence<sizeof...(Ts)>() ); */
/*     } */


  class SysVarBranch {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      SysVarBranch(const std::string& branchName) :
        m_branch(branchName) {}

      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.nameBranch(m_branch, syst);
      }

    private:
      std::string m_branch;
  };

  class SysVarBranchVector {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::vector<std::string>;

      SysVarBranchVector(const std::vector<std::string>& branchNames) :
        m_branchNames(branchNames) {}

      std::vector<std::string> translate(IBranchNamer& namer, const std::string& syst) {
        return namer.nameBranches(m_branchNames, syst);
      }

    private:
      std::vector<std::string> m_branchNames;
  };

  class SysVarNewBranch {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      SysVarNewBranch(const std::string& branchName) :
        m_branch(branchName) {}

      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.createBranch(m_branch, syst);
      }

    private:
      std::string m_branch;
  };

  class SysVarStringExpression {
    public:
      static constexpr bool is_rdf_sysvar = true;
      using value_type = std::string;

      SysVarStringExpression(
          const std::string& expressionTemplate,
          const std::vector<std::string>& columnNames) :
        m_template(expressionTemplate),
        m_columns(columnNames) {}

      std::string translate(IBranchNamer& namer, const std::string& syst) {
        return namer.interpretExpression(m_template, m_columns, syst);
      }

    private:
      std::string m_template;
      std::vector<std::string> m_columns;
  };
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_SysVar_H
