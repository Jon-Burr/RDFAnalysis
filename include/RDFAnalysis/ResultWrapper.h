#ifndef RDFAnalysis_ResultWrapper_H
#define RDFAnalysis_ResultWrapper_H

// ROOT includes
#include <ROOT/RDF/InterfaceUtils.hxx>

// STL includes
#include <type_traits>
#include <functional>

namespace RDFAnalysis {
  /**
   * @brief Wrapper class for RResultPtrs.
   * @tparam T the type wrapped.
   * This rather complicated structure is only necessary in ROOT v6.16 where
   * RResultPtrs cannot convert between accessible types.
   */
  template <typename T>
    class ResultWrapper  {
      public:
        /// Destructor
        ~ResultWrapper() {}

        /**
         * @brief Constructor
         * @tparam U The concrete type of the RResultPtr
         * @param The RResultPtr
         */
        template <typename U, 
                 typename = std::enable_if_t<std::is_base_of<T, U>{}, void>>
          /* ResultWrapper(const ROOT::RDF::RResultPtr<U>& ptr) : */
          ResultWrapper(ROOT::RDF::RResultPtr<U> ptr) :
            m_holder([ptr] () mutable -> T* {return ptr.GetPtr();}) {}

        /**
         * @brief Copy constructor
         * @tparam U The held type of the other object
         * @param The object to copy.
         */
        template <typename U,
                 typename = std::enable_if_t<
                   std::is_base_of<T, U>{} && !std::is_same<T, U>{}, void>>
          ResultWrapper(ResultWrapper<U> other) :
            m_holder([other] () mutable -> T* { return other.get(); }) {}

        /**
         * @brief Move constructor
         * @tparam U The held type of the other object
         * @param The object to move.
         */
        template <typename U,
                 typename = std::enable_if_t<
                   std::is_base_of<T, U>{} && !std::is_same<T, U>{}, void>>
          ResultWrapper(ResultWrapper<U>&& other) :
            m_holder([other] () { return other.get(); }) {}

        /// Non-template copy constructor
        ResultWrapper(const ResultWrapper&) = default;
        /// Non-template move constructor
        ResultWrapper(ResultWrapper&&) = default;

        /**
         * @brief Get the held value.
         */
        T* get() {
          return m_holder();
        }
      private:
        /// Function that returns the held object.
        std::function<T*()> m_holder;


    }; //> end class ResultWrapper
} //> end namespace RDFAnalysis
#endif //> !RDFAnalysis_ResultWrapper_H
