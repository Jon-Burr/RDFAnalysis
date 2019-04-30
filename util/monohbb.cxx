#include "RDFAnalysis/Node.h"
#include <boost/program_options.hpp>
#include <TChain.h>
#include <iostream>
#include <TLorentzVector.h>
#include <memory>
#include <ROOT/RDFHelpers.hxx>
#include "RDFAnalysis/OutputWriter.h"
#include "RDFAnalysis/TObjectWriter.h"
#include "RDFAnalysis/CutflowWriter.h"
#include "RDFAnalysis/DefaultBranchNamer.h"
#include <boost/algorithm/string/join.hpp>
#include <fstream>
namespace {
  TH1F HPt("HiggsPt", ";p_T(H, true) [MeV]", 500, 0, 2e6);
  TH1F Mass("HiggsMass", ";m(H candidate) [MeV]", 80, 70e3, 150e3);
  TH2F MassPt("HiggsMassPt", ";p_T(H, true) [MeV]; m(H candidate) [MeV]", 20, 0, 2e6, 80, 70e3, 150e3);
  void basicPlots(std::shared_ptr<RDFAnalysis::Node> node, std::string massVar, std::string folder="massWindow") {
    auto massWindow = node->Filter("70e3 < " + massVar + " && " + massVar + " < 150e3", folder, "70 < m_H < 150 GeV");
    massWindow->Fill(HPt, {"pTbbTruth"});
    massWindow->Fill(Mass, {massVar});
    massWindow->Fill(MassPt, {"pTbbTruth", massVar});
  }
}
int main(int argc, char* argv[]) {
  std::string treeName;
  Long64_t nEvents;
  std::string output;
  bool overwrite{false};
  std::string sysListFile;
  std::vector<std::string> inputFiles;
  std::string inTxt;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("sysListFile,s", po::value(&sysListFile), "The systematics file")
    ("treeName,t", po::value(&treeName)->default_value("AnalysisTree"),
     "The name of the tree")
    ("nEvents,n", po::value(&nEvents)->default_value(-1), "The number of events over which to run. -1 will run all available events")
    ("output,o", po::value(&output)->default_value("hist-out.root"), "The output ROOT file")
    ("overwrite,w", po::bool_switch(&overwrite), "Enable overwriting of the output file")
    ("inTxt,I", po::value(&inTxt), "Text file containing inputs")
    ("help,h", "Print this message and exit");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("inputFiles", po::value(&inputFiles) );
  po::positional_options_description positional;
  positional.add("inputFiles", -1);
  po::variables_map vm;
  po::options_description allOptions;
  allOptions.add(desc).add(hidden);
  po::store(po::command_line_parser(argc, argv).options(allOptions).positional(positional).run(), vm);
  po::notify(vm);
  if (vm.count("help") ) {
    std::cout << desc << std::endl;
    return 0;
  }
  if (!inTxt.empty() ) {
    std::string line;
    std::ifstream inTxtFin(inTxt);
    if (!inTxtFin.is_open() ) {
      std::cerr << "Failed to open input text file " << inTxt << std::endl;
      return 1;
    }
    while (std::getline(inTxtFin, line) )
      if (!line.empty() )
        inputFiles.push_back(line);
  }
  TChain input(treeName.c_str() );
  for (const std::string& fin : inputFiles)
    if (input.Add(fin.c_str(), -1) == 0) {
      std::cerr << "Tree " << treeName << " not found in file " << fin << std::endl;
      return 1;
    }

  std::vector<std::string> systematics;
  std::ifstream sysFin(sysListFile);
  if (!sysFin.is_open() ) {
    std::cerr << "Failed to open systematics list file " << sysListFile << std::endl;
    return 1;
  }
  {
    std::string line;
    while (std::getline(sysFin, line) )
      if (!line.empty() )
        systematics.push_back(line);
  }

  auto root = RDFAnalysis::createROOT(
      ROOT::RDataFrame(input),
      std::make_unique<RDFAnalysis::DefaultBranchNamer>(systematics, true, false) );

  root->Define("SelJetPt", "JetPt[JetIsSignal]").
    Define("SelJetEta", "JetEta[JetIsSignal]").
    Define("SelJetPhi", "JetPhi[JetIsSignal]").
    Define("SelJetMass", "JetMass[JetIsSignal]").
    Define("SelJetIsBJet", "JetIsBJet[JetIsSignal]").
    Define("SelFatJetPt", "FatJetPt[FatJetIsSignal]").
    Define("SelFatJetEta", "FatJetEta[FatJetIsSignal]").
    Define("SelFatJetPhi", "FatJetPhi[FatJetIsSignal]").
    Define("SelFatJetMass", "FatJetMass[FatJetIsSignal]").
    Define("SelMuCorrFatJetPt", "FatJetMuonInJetPt[FatJetIsSignal]").
    Define("SelMuCorrFatJetEta", "FatJetMuonInJetEta[FatJetIsSignal]").
    Define("SelMuCorrFatJetPhi", "FatJetMuonInJetPhi[FatJetIsSignal]").
    Define("SelMuCorrFatJetMass", "FatJetMuonInJetMass[FatJetIsSignal]").
    Define("SelFatJetNTrackBJets", "FatJetNTrackBJets[FatJetIsSignal]").
    Define("SelFatJetPassDRCut", "FatJetPassDRCut[FatJetIsSignal]");
  auto presel = root->Filter("Preselection", "Preselection", "Preselection");
  auto veto = [] (const std::vector<char>& vec) {
    return std::none_of(vec.begin(), vec.end(), [] (char c) { return bool(c); });
  };
  auto leptonVeto = presel->Filter(veto, {"ElePassOR"})->Filter(veto, {"MuonPassOR"}, "LeptonVeto", "Lepton Veto");
  auto met150 = leptonVeto->Filter("MissingET > 150e3", "MET150", "MET > 150 GeV");

  met150->Fill(HPt, {"pTbbTruth"});

  auto resolved = met150->Filter(
      "SelJetPt.size() > 1 && SelJetPt[0] > 45e3 && SelJetPt[1] > 45e3", "TwoJets", "p_T (j1,2) > 45 GeV");
  resolved->Fill(HPt, {"pTbbTruth"});
  basicPlots(resolved, "mjjR04");
  auto resolved2b = resolved->Filter("NbjjR04 >= 2", "TwoBJets", "n_B (jj) >=2");
  basicPlots(resolved2b, "mjjR04");
  auto merged = met150->Filter("SelFatJetPt.size() > 0 && SelFatJetPt[0] > 200e3", "OneFatJet", "p_T (J0) > 200 GeV");
  merged->Define("mJ", "SelFatJetMass[0]");
  merged->Define("mJMuCorr", "SelMuCorrFatJetMass[0]");
  basicPlots(merged, "mJ");
  basicPlots(merged, "mJMuCorr", "massWindowMuCorr");
  auto merged2b = merged->Filter("SelFatJetNTrackBJets[0] >= 2 && SelFatJetPassDRCut[0]", "TwoBVRJets", "n_B (VR) >= 2");
  basicPlots(merged2b, "mJ");
  basicPlots(merged2b, "mJMuCorr", "massWindowMuCorr");

  RDFAnalysis::OutputWriter outputWriter(output, overwrite);
  outputWriter.addWriter(RDFAnalysis::TObjectWriter() );
  outputWriter.addWriter(RDFAnalysis::CutflowWriter() );
  std::cout << "Trigger run" << std::endl;
  root->rootRNode().Foreach(
      [nEvents = input.GetEntries()] (ULong64_t entry) {
        if (entry%5000 == 0) 
          std::cout << entry << "/" << nEvents << std::endl;
      }, {"rdfentry_"});
  outputWriter.write(*root);

  return 0;
}
