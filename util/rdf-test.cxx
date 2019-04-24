#include <boost/program_options.hpp>
#include <TChain.h>
#include <TFile.h>
#include <TKey.h>
#include <iostream>
#include <TLorentzVector.h>
#include <memory>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RDFHelpers.hxx>

int main(int argc, char* argv[]) {
  std::string nominalTree;
  Long64_t nEvents;
  std::string output;
  bool overwrite{false};
  std::vector<std::string> inputFiles;
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("nominalTree,t", po::value(&nominalTree)->default_value("NOSYS"), "The name of the nominal tree (and path to it if necessary) within the input ROOT files")
    ("nEvents,n", po::value(&nEvents)->default_value(-1), "The number of events over which to run. -1 will run all available events")
    ("output,o", po::value(&output)->default_value("hist-out.root"), "The output ROOT file")
    ("overwrite,w", po::bool_switch(&overwrite), "Enable overwriting of the output file")
    ("help,h", "Print this message and exit");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("inputFiles", po::value(&inputFiles), "The input files");
  po::positional_options_description positional;
  positional.add("inputFiles", -1); // All positional arguments are input files
  po::variables_map vm;
  po::options_description allOptions;
  allOptions.add(desc).add(hidden);
  po::store(po::command_line_parser(argc, argv).options(allOptions).positional(positional).run(), vm);
  po::notify(vm);
  if (vm.count("help") ) {
    std::cout << desc << std::endl;
    return 0;
  }
  TFile testFile(inputFiles.at(0).c_str() );
  std::vector<std::unique_ptr<TChain>> chains;
  std::vector<std::string> systematics;
  TChain* nominalChain = nullptr;
  for (const TObject* ikey : *testFile.GetListOfKeys() ) {
    const TKey* key = dynamic_cast<const TKey*>(ikey);
    if (!key) {
      std::cerr << "Failed to cast key " << ikey->GetName() << std::endl;
      return 1;
    }
    if (key->GetClassName() == std::string("TTree") ) {
      chains.emplace_back(new TChain(key->GetName()));
      systematics.push_back(key->GetName() );
      if (key->GetName() == nominalTree)
        nominalChain = chains.back().get();
    }
  }
  if (!nominalChain) {
    std::cerr << "Nominal tree " << nominalTree << " not found in file "
      << inputFiles.at(0) << std::endl;
    return 1;
  }
  for (const std::string& fileName : inputFiles) {
    for (std::unique_ptr<TChain>& chain : chains) {
      if (chain->Add(fileName.c_str(), -1) < 0) {
        std::cerr << "Failed to add file " << fileName << " to chain "
          << chain->GetName() << std::endl;
        return 1;
      }
    }
  }
  for (std::unique_ptr<TChain>& chain : chains) {
    if (chain.get() == nominalChain)
      continue;
    nominalChain->AddFriend(chain.get() );
  }

  ROOT::RDataFrame rdf0(*nominalChain);
  auto rdf = rdf0.Alias("systJetPt", "JET_EtaIntercalibration_NonClosure_highE__1down.JetPt");
  auto histo = rdf.Histo1D("systJetPt");
  /* auto histo = rdf.Histo1D("JET_EtaIntercalibration_NonClosure_highE__1down.JetPt"); */

  TFile fout("rdf-test-out.root", "RECREATE");
  fout.WriteTObject(histo.GetPtr() );

  /* std::cout << "Make namer" << std::endl; */
  /* auto namerU = std::make_unique<RDFAnalysis::DefaultBranchNamer>(systematics); */
  /* std::cout << "createROOT" << std::endl; */
  /* auto root = RDFAnalysis::createROOT( */
  /*     ROOT::RDataFrame(*nominalChain), */ 
  /*     std::move(namerU) ); */
  /*     /1* std::make_unique<RDFAnalysis::DefaultBranchNamer>(systematics) ); *1/ */
  /* std::cout << "Define" << std::endl; */
  /* root->Define( */
  /*       "SelectedJets", */
  /*       [] ( */
  /*         const std::vector<float>& jetPt, */
  /*         const std::vector<float>& jetEta, */
  /*         const std::vector<float>& jetPhi, */
  /*         const std::vector<float>& jetMass, */
  /*         const std::vector<char>& jetSel) */
  /*       { */
  /*         std::vector<TLorentzVector> out; */
  /*         for (std::size_t ii = 0; ii < jetPt.size(); ++ii) { */
  /*           if (jetSel.at(ii) ) { */
  /*             TLorentzVector vec; */
  /*             vec.SetPtEtaPhiM(jetPt.at(ii), jetEta.at(ii), jetPhi.at(ii), jetMass.at(ii)); */
  /*             out.push_back(vec); */
  /*           } */
  /*         } */
  /*         return out; */
  /*       }, */
  /*       {"JetPt", "JetEta", "JetPhi", "JetMass", "JetIsSignal"}); */

  /* std::cout << "Filter" << std::endl; */
  /* auto twoJets = root->Filter( */
  /*     [] (const std::vector<TLorentzVector>& jets) */
  /*     { return jets.size() >=2; }, */
  /*     { "SelectedJets" }, */
  /*     "TwoJets", */
  /*     "nJets >= 2"); */

  /* std::cout << "Histo1D" << std::endl; */
  /* twoJets->Histo1D( */
  /*     {"jetPt", ";p_{T} (j) [MeV]", 100, 0, 1e5}, */
  /*     "JetPt"); */

  /* std::cout << "Create writer" << std::endl; */
  /* RDFAnalysis::OutputWriter outputWriter("test-hists.root", true); */
  /* outputWriter.addWriter(RDFAnalysis::TObjectWriter() ); */
  /* std::cout << "Write" << std::endl; */
  /* outputWriter.write(*root); */


  /* const auto& namer = root->namer(); */
  /* std::cout << "Found systematics: " << std::endl; */
  /* for (const std::string& syst :  namer.systematics() ) */
  /*   std::cout << "\t" << syst << std::endl; */
  /* std::cout << "Found branches: " << std::endl; */
  /* for (const std::string& branchName : namer.branches() ) { */
  /*   std::cout << branchName << std::endl; */
  /*   for (const std::string& syst : namer.systematicsAffecting(branchName) ) */
  /*     std::cout << "\t" << syst << std::endl; */
  /* } */
  return 0;
}
