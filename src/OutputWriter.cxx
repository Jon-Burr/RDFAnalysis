// package includes
#include "RDFAnalysis/OutputWriter.h"

// ROOT includes
#include <TFile.h>

namespace RDFAnalysis {
  OutputWriter::OutputWriter(
      const std::shared_ptr<TDirectory>& directory) :
    m_directory(directory)
  {}

  OutputWriter::OutputWriter(
      const std::string& fileName,
      bool overwrite) :
    m_directory(std::make_unique<TFile>(
          fileName.c_str(), overwrite ? "RECREATE" : "CREATE") )
  {
    if (m_directory->IsZombie() ) {
      throw std::runtime_error("Failed to open " + fileName);
    }
  }

  void OutputWriter::write(
      Node& node,
      TDirectory* directory,
      std::size_t depth)
  {
    directory->cd();
    // Apply all of our writers - writers are not applied to anonymous nodes!
    if (!node.isAnonymous() )
      for (std::shared_ptr<INodeWriter>& writer : m_writers)
        writer->write(node, directory, depth);

    // Now go through all of this node's children
    for (std::shared_ptr<Node>& child : node.children() ) {
      // If the child is anonymous then just pass down what we have now
      if (child->isAnonymous() )
        write(*child, directory, depth);
      else {
        TDirectory* newDirectory = getMkdir(directory, child->name() );
        write(*child, newDirectory, depth+1);
      }
    }
  }
} //> end namespace RDFAnalysis
