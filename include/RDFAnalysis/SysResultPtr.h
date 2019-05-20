#ifndef RDFAnalysis_SysResultPtr_H
#define RDFAnalysis_SysResultPtr_H

// Package includes
#include "RDFAnalysis/ResultWrapper.h"

namespace RDFAnalysis {
  /**
   * @brief Class to wrap together RResultPtrs for different systematic
   * variations
   * @tparam T The type held by the RResultPtrs
   */
  template <typename T>
    class SysResultPtr {
      public:
        /**
         * @brief Create the result
         * @param nominalName The name of the nominal variation
         */
        SysResultPtr(const std::string& nominalName) :
          m_nominal(nominalName) {}

        /// Allow access to the underlying map
        std::map<std::string, ResultWrapper<T>>& asMap() { return m_wrappers; }

        /// Set from a map
        void setMap(const std::map<std::string, ResultWrapper<T>>& newMap) { m_wrappers = newMap; }

        /// Reset all results
        void reset() { m_wrappers.clear(); }

        /**
         * @brief Get the result pointed to
         * @param systematic The variation to retrieve
         * If the variation doesn't exist then return the nominal.
         */
        T* get(const std::string& syst) 
        {
          auto itr = m_wrappers.find(syst);
          return itr == m_wrappers.end()
            ? m_wrappers.at(m_nominal)
            : itr->second;
        }

        /**
         * @brief Add the result wrapper for a given systematic
         * @param systematic The variation to add
         * @param result The result to add.
         * @return if a new result was provided
         */ 
        // TODO (maybe) - make it impossible to overwrite a result with this
        // method, or rather to make it throw an error if you try...
        bool addResult(
            const std::string& systematic,
            const ResultWrapper<T>& result)
        {
          return m_wrappers.insert(std::make_pair(systematic, result) ).second;
        }

        /**
         * @brief Allow type conversions in copy construction
         * @tparam U The held type of the other object
         * @param The object to copy
         */
        template <typename U,
                  typename = std::enable_if_t<std::is_base_of<T, U>{}, void>
                 >
          SysResultPtr(SysResultPtr<U>& other) 
          {
            for (const auto& p : other.asMap() )
              m_wrappers.insert(std::make_pair(
                    p.first, ResultWrapper<T>(p.second) ) );
          }


      private:
        std::string m_nominal;
        std::map<std::string, ResultWrapper<T>> m_wrappers;
    };
}
#endif //> !RDFAnalysis_SysResultPtr_H
