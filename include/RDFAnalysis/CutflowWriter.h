#ifndef RDFAnalysis_CutflowWriter_H
#define RDFAnalysis_CutflowWriter_H

// package includes
#include "RDFAnalysis/INodeWriter.h"
#include "RDFAnalysis/Helpers.h"
#include "RDFAnalysis/CutflowDetail.h"
#include <string>
#include <vector>
#include <map>
#include "Rtypes.h"
#include <TH1.h>
#include <TDirectory.h>

/**
 * @file CutflowWriter.h
 * @brief Writer class for cutflow information.
 */

namespace RDFAnalysis {
  /**
   * @brief Class to write cutflows from Nodes.
   */
  template <typename Detail>
    class CutflowWriter : public INodeWriter<Detail> {
      static_assert(std::is_base_of<CutflowDetail, Detail>::value, "The CutflowWriter requires a cutflow detail!!");
      public:
        ~CutflowWriter() override {}

        /**
        * @brief Create the writer.
        * @param subDirName The name of the directory to save the cutflows to. If
        * the empty string is provided then the cutflows will not be saved to a
        * subdirectory.
        */
        CutflowWriter(const std::string& subDirName="cutflows");

        /**
        * @brief Write cutflows from \ref node to \ref directory.
        * @param node The node to write.
        * @param directory The directory to write to.
        * @param depth How deep down the node structure we are.
        */
        void write(
            Node<Detail>& node,
            TDirectory* directory,
            std::size_t depth) override;
        
      private:
        /// The subdirectory name
        std::string m_subDirName;

    }; //> end class CutflowWriter
} //> end namespace RDFAnalysis

#include "RDFAnalysis/CutflowWriter.icc"
#endif //> !RDFAnalysis_CutflowWriter_H
