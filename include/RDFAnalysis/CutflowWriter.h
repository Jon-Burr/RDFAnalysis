#ifndef RDFAnalysis_CutflowWriter_H
#define RDFAnalysis_CutflowWriter_H

// package includes
#include "RDFAnalysis/INodeWriter.h"
#include <string>
#include <vector>
#include <map>
#include "RTypes.h"

class TH1;

namespace RDFAnalysis {
  /**
   * @brief Class to write cutflows from Nodes
   */
  class CutflowWriter : public INodeWriter {
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
          Node& node,
          TDirectory* directory,
          std::size_t depth) override;
      
    private:
      /// The subdirectory name
      std::string m_subDirName;

      /// Build up the cutflow into this vector
      std::vector<std::map<std::string,std::pair<std::string, ULong64_t>>> m_cutflow;

  }; //> end class CutflowWriter
} //> end namespace RDFAnalysis

#endif //> !RDFAnalysis_CutflowWriter_H
