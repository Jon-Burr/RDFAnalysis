#ifndef RDFAnalysis_TObjectWriter_H
#define RDFAnalysis_TObjectWriter_H

// package includes
#include "RDFAnalysis/INodeWriter.h"
#include "RDFAnalysis/Helpers.h"
#include <string>

// ROOT includes
#include "TObject.h"
#include <TDirectory.h>

/**
 * @file TObjectWriter.h
 * @brief Class to write out the TObjects defined on the Nodes.
 */

namespace RDFAnalysis {
  /**
   * @brief Class to write out the TObjects from a Node
   */
  template <typename Detail>
    class TObjectWriter : public INodeWriter<Detail> {
      public:
        ~TObjectWriter() override {}

        /**
        * @brief Create the writer.
        * @param subDirName The name of the directory to save the plots to. If
        * the empty string is provided then the plots will not be saved to a
        * subdirectory.
        */
        TObjectWriter(const std::string& subDirName="plots");

        /**
        * @brief Write the contents of \ref node to \ref directory.
        * @param node The node to write
        * @param directory The directory to write
        * @param depth How deep down the node structure we are.
        */
        void write(
            Node<Detail>& node,
            TDirectory* directory,
            std::size_t /* depth */) override;

        /**
         * @brief Write the contents of a region to a directory.
         * @param region The region to write
         * @param directory The directory to write to
         * @param depth How deep down the node structure we are.
         */
        void write(
            typename Scheduler<Detail>::Region& region,
            TDirectory* directory,
            std::size_t /* depth */) override;

      private:
        /// The subdirectory name
        std::string m_subDirName;
    }; //> end class TObjectWriter
} //> end namespace 
#include "RDFAnalysis/TObjectWriter.icc"
#endif //> !RDFAnalysis_TObjectWriter_H
