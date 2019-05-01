// package includes
#include "RDFAnalysis/TObjectWriter.h"
#include "RDFAnalysis/Node.h"
#include "RDFAnalysis/Helpers.h"

#include <TDirectory.h>

namespace RDFAnalysis {

  TObjectWriter::TObjectWriter(const std::string& subDirName) :
    m_subDirName(subDirName) 
  {}

  void TObjectWriter::write(
      Node& node, 
      TDirectory* directory,
      std::size_t /* depth */)
  {
    /* if (!m_subDirName.empty() && node.objects().size() > 0) */
    /*   directory = getMkdir(directory, m_subDirName); */
    // Loop over each object and save them
    for (SysResultPtr<TObject>& object : node.objects() ) {
      // Get the directory for each systematic and save that object there
      for (auto& objPair : object.asMap() ) {
        TDirectory* systDir = getMkdir(directory, objPair.first+"/"+m_subDirName);
        systDir->WriteTObject(objPair.second.get() );
      }
    }
  }
} //> end namespace RDFAnalysis
