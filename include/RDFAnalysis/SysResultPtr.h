#ifndef RDFAnalysis_SysResultPtr_H
#define RDFAnalysis_SysResultPtr_H

// Package includes
#include "RDFAnalysis/ResultWrapper.h"

/**
 * @file SysResultPtr.h
 * @brief Class to wrap systematic variations of RResultPtrs.
 */

namespace RDFAnalysis {
  /**
   * @brief Class to wrap together RResultPtrs for different systematic
   * variations
   * @tparam T The type held by the RResultPtrs
   *
   * This class acts as a specialised map class holding result pointers
   * corresponding to systematic variations of the same quantity. If a
   * systematic variation is requested that does not affect this variable the
   * nominal is returned.
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

        /**
         * @brief Create the result from an existing map
         * @tparam The type of the result ptr
         * @param nominalName The name of the nominal variation
         * @param resultMap Map of systematics to RResultPtrs
         */
        template <typename U,
                 typename = std::enable_if_t<std::is_base_of<T, U>{} || std::is_same<T, U>{}, void>>
          SysResultPtr(
              const std::string& nominalName,
              const std::map<std::string, ROOT::RDF::RResultPtr<U>>& resultMap) :
            m_nominal(nominalName)
          {
            for (const auto& p : resultMap)
              addResult(p.first, p.second);
          }

        /// Iterator to the start of the underlying map
        auto begin() { return m_wrappers.begin(); }

        /// Const iterator to the start of the underlying map
        auto begin() const { return m_wrappers.begin(); }

        /// Iterator to the end of the underlying map
        auto end() { return m_wrappers.end(); }

        /// Const iterator to the end of the underlying map
        auto end() const { return m_wrappers.end(); }

        /// Size of the map (i.e. the number of systematics affecting this
        /// result)
        std::size_t size() const { return m_wrappers.size(); }

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
            ? m_wrappers.at(m_nominal).get()
            : itr->second.get();
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
          SysResultPtr(const SysResultPtr<U>& other) :
            m_nominal(other.m_nominal)
          {
            for (const auto& p : other)
              m_wrappers.insert(std::make_pair(
                    p.first, ResultWrapper<T>(p.second) ) );
          }

        /// Allow conversion to bool - returns true if anything is filled
        operator bool() const { return m_wrappers.size() != 0; }

      private:
        template <typename U>
          friend class SysResultPtr;
        std::string m_nominal;
        std::map<std::string, ResultWrapper<T>> m_wrappers;
    };
}
#endif //> !RDFAnalysis_SysResultPtr_H
