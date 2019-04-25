#ifndef RDFAnalysis_Helpers_H
#define RDFAnalysis_Helpers_H

#include <type_traits>
#include <TDirectory.h>

namespace RDFAnalysis {
  /// Helper class to allow iterating through a container without allowing
  /// users to modify that container
  template <typename Iterator>
    class range_t {
      public:
        using itr_t = Iterator;
        range_t(Iterator begin, Iterator end) :
          m_begin(begin),
          m_end(end) {}

        Iterator begin() const { return m_begin; }
        Iterator end() const { return m_end; }
        std::size_t size() const { return std::distance(begin(), end() ); }
      private:
        const Iterator m_begin;
        const Iterator m_end;
    };

  template <typename Container>
    auto as_range(Container& container) {
      return range_t<std::decay_t<decltype(container.begin() )>>(
          std::begin(container),
          std::end(container) );
    }

  template <typename Container>
    auto as_range(const Container& container) {
      return range_t<std::decay_t<decltype(container.begin() )>>(
          std::begin(container),
          std::end(container) );
    }

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
    return newDir;
  }

}; //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_Helpers_H
