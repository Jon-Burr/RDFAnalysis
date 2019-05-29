#ifndef RDFAnalysis_Helpers_H
#define RDFAnalysis_Helpers_H

#include <type_traits>
#include <TDirectory.h>
#include <ROOT/RDataFrame.hxx>
#include <random>

/**
 * @file Helpers.h
 * File containing helper classes and functions.
 */

namespace RDFAnalysis {
  /**
   * @brief Helper class to allow iterating through a container without allowing
   * users to modify that container.
   *
   * @tparam Iterator The type of the input iterator.
   */
  template <typename Iterator>
    class range_t {
      public:
        using itr_t = Iterator;
        /// Construct the range from two iterators
        range_t(Iterator begin, Iterator end) :
          m_begin(begin),
          m_end(end) {}

        /// The start of the range
        Iterator begin() const { return m_begin; }
        /// The end of the range
        Iterator end() const { return m_end; }
        /// The size of the wrapped range
        std::size_t size() const { return std::distance(begin(), end() ); }
      private:
        const Iterator m_begin;
        const Iterator m_end;
    };

  /// Make a \ref range_t from a container
  template <typename Container>
    auto as_range(Container& container) {
      return range_t<std::decay_t<decltype(container.begin() )>>(
          std::begin(container),
          std::end(container) );
    }

  /// Make a \ref range_t from a const container
  template <typename Container>
    auto as_range(const Container& container) {
      return range_t<std::decay_t<decltype(container.begin() )>>(
          std::begin(container),
          std::end(container) );
    }

  /**
   * @brief Get a directory, making it if it isn't there already.
   *
   * @param dir The directory from which to get/make the new one
   * @param name The name of the new directory
   * @param doThrow Whether to throw an exception if the function fails
   */
  inline TDirectory* getMkdir(
      TDirectory* dir,
      const std::string& name, 
      bool doThrow = true)
  {
    TDirectory* newDir = dir->GetDirectory(name.c_str() );
    if (newDir)
      return newDir;
    newDir = dir->mkdir(name.c_str() );
    if (!newDir && doThrow)
      throw std::runtime_error("Failed to get/make directory " + name);
    // When making a hierarchy mkdir has an unexpected return type (the root of
    // the new directories, not the bottom step so we have to return the right
    // thing
    return dir->GetDirectory(name.c_str() );
  }

  /**
   * @brief Get a value by key, defaulting to a backup key if it is not there.
   *
   * @tparam Map The map type being interrogated
   * @param theMap The map being interrogated
   * @param key The key to search for
   * @param defaultKey The fallback key
   *
   * Try and return the item keyed by \ref key from \ref theMap. If it isn't
   * there, return the item keyed by \ref defaultKey instead.
   */
  template <typename Map>
    typename Map::mapped_type getDefaultKey(
        const Map& theMap,
        const typename Map::key_type& key,
        const typename Map::key_type& defaultKey)
    {
      auto itr = theMap.find(key);
      if (itr == theMap.end() )
        return theMap.at(defaultKey);
      else
        return itr->second;
    }

  /// Get the number of slots used in this session.
  inline unsigned int getNSlots() {
    unsigned int poolSize = ROOT::GetImplicitMTPoolSize();
    return poolSize == 0 ? 1 : poolSize;
  }

  /// Reduce size of enable_if statements
  template <typename F, typename T>
    using enable_ifn_string_t = std::enable_if_t<!std::is_convertible<F, std::string>{}, T>;

  /// Apply is C++17... 
  template <typename F, typename... Ts, std::size_t... Is>
    constexpr decltype(auto) apply_impl(F&& f, std::tuple<Ts...>&& args, std::index_sequence<Is...>)
    {
      return f(std::get<Is>(args)...);
    }
  template <typename F, typename...Ts>
    constexpr decltype(auto) apply(F&& f, std::tuple<Ts...>&& args)
    {
      return apply_impl(std::forward<F&&>(f), std::forward<std::tuple<Ts...>&&>(args), std::make_index_sequence<sizeof...(Ts)>());
    }

  /// Could perhaps be more natural in the IBranchNamer?
  std::string uniqueBranchName(const std::string& stub = "GenBranch");

  template <typename F>
    struct is_std_function : public std::false_type {};

  template <typename R, typename... Ts>
    struct is_std_function<std::function<R(Ts...)>> : public std::true_type {};

}; //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_Helpers_H
