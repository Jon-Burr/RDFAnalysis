#ifndef RDFAnalysis_OutputWriter_H
#define RDFAnalysis_OutputWriter_H

// package includes
#include "RDFAnalysis/INodeWriter.h"

// ROOT includes
#include <TDirectory.h>

// STL includes
#include <memory>

namespace RDFAnalysis {
  /**
   * @brief Class to write out objects from an RDFAnalysis
   * Each named filter in the graph is represented by a folder in the output
   * file. This class only takes care of creating the folder structure, it
   * receives other INodeWriter objects that create the output from any
   * individual node.
   */
  template <typename N>
    class OutputWriter {
      public:
        /// The node type we're templated on
        using node_t = N;
        /**
        * @brief Create the writer
        * @param directory The directory to write into
        */
        OutputWriter(const std::shared_ptr<TDirectory>& directory);

        /**
        * @brief Create the writer, opening a file to do it
        * @param fileName The file to create
        * @param overwrite Whether or not to overwrite the output file.
        */
        OutputWriter(
            const std::string& fileName,
            bool overwrite = false);

        /**
        * @brief Write information from the given node and all downstream.
        * @param node The node to write from
        */
        void write(N& node) { write(node, m_directory.get() ); }

        /// Add a writer  
        void addWriter(const std::shared_ptr<INodeWriter<N>>& writer)
        { m_writers.push_back(writer); }

        /// Add a writer
        template <typename T> std::enable_if_t<std::is_base_of<INodeWriter<N>, T>{},
          void> addWriter(T&& writer)
          { addWriter(std::make_shared<T>(std::move(writer) ) ); }

        /**
         * @brief Add a writer
         * @tparam T The type of writer to add
         * @tparam Args The types of arguments for the writer constructor
         * @param args The arguments for the constructor.
         * This version constructs the writer in place. The writer type T will
         * be templated on the write N class.
         */
        template <template<class> class T, typename... Args>
          std::enable_if_t<std::is_base_of<INodeWriter<N>, T<N>>{}, void> addWriter(
              Args&&... args)
          { addWriter(std::make_shared<T<N>>(std::forward<Args>(args)...) ); }

        /// Get the writers
        std::vector<std::shared_ptr<INodeWriter<N>>>& writers()
        { return m_writers; }

        /// Get the writers
        const std::vector<std::shared_ptr<INodeWriter<N>>>& writers() const
        { return m_writers; }

      private:
        /// The output directory
        std::shared_ptr<TDirectory> m_directory;

        /// The writers
        std::vector<std::shared_ptr<INodeWriter<N>>> m_writers;

        /// Write directly to a directory
        void write(
            N& node,
            TDirectory* directory,
            std::size_t depth = 0);
    }; //> end class OutputWriter
} //> end namespace RDFAnalysis
#include "RDFAnalysis/OutputWriter.icc"
#endif //> !RDFAnalysis_OutputWriter_H
