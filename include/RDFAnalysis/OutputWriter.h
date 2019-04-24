#ifndef RDFAnalysis_OutputWriter_H
#define RDFAnalysis_OutputWriter_H

// package includes
#include "RDFAnalysis/Node.h"
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
  class OutputWriter {
    public:
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
      void write(Node& node) { write(node, m_directory.get() ); }

      /// Add a writer  
      void addWriter(const std::shared_ptr<INodeWriter>& writer)
      { m_writers.push_back(writer); }

      /// Add a writer
      template <typename T> std::enable_if_t<std::is_base_of<INodeWriter, T>{},
        void> addWriter(T&& writer)
        { addWriter(std::make_shared<T>(std::move(writer) ) ); }

      /// Get the writers
      std::vector<std::shared_ptr<INodeWriter>>& writers()
      { return m_writers; }

      /// Get the writers
      const std::vector<std::shared_ptr<INodeWriter>>& writers() const
      { return m_writers; }

    private:
      /// The output directory
      std::shared_ptr<TDirectory> m_directory;

      /// The writers
      std::vector<std::shared_ptr<INodeWriter>> m_writers;

      /// Write directly to a directory
      void write(
          Node& node,
          TDirectory* directory,
          std::size_t depth = 0);
  }; //> end class OutputWriter
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_OutputWriter_H
