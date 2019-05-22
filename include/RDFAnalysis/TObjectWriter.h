#ifndef RDFAnalysis_TObjectWriter_H
#define RDFAnalysis_TObjectWriter_H

// package includes
#include "RDFAnalysis/INodeWriter.h"
#include "RDFAnalysis/Helpers.h"
#include <string>

// ROOT includes
#include "TObject.h"
#include <TDirectory.h>

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

      private:
        /// The subdirectory name
        std::string m_subDirName;
    }; //> end class TObjectWriter
} //> end namespace 
#include "RDFAnalysis/TObjectWriter.icc"
#endif //> !RDFAnalysis_TObjectWriter_H
