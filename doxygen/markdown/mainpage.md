# RDFAnalysis Documentation #      {#mainpage}

- [Modelling an Analysis as a Tree](MD_AnalysisAsTree.html)
- [Introduction to ROOT::RDataFrame](MD_RDataFrameIntro.html)
- [The RDFAnalysis::Node](MD_RDFNode.html)
- [The Job Scheduler](MD_JobScheduler.html)
- [Including Systematics](MD_Systematics.html)

------------------

This package is designed to integrate several important concepts for physics analysis into the [ROOT::RDataFrame](https://root.cern/doc/v616/classROOT_1_1RDataFrame.html).
This is designed to go from having a ROOT [TTree](https://root.cern/doc/v616/classTTree.html) to a file containing histograms and other objects that can then form the inputs to a statistical analysis.  
The basic analysis model is covered in [Modelling an Analysis as a Tree](MD_AnalysisAsTree.html).
An overview of ROOT::RDataFrame is provided in [Introduction to ROOT::RDataFrame](MD_RDataFrameIntro.html). 
An introduction to the core analysis object is in [The RDFAnalysis::Node](MD_RDFNode.html), with a more advanced but hopefully more user friendly interface described in [The Job Scheduler](MD_JobScheduler.html).
How systematic variations are integrated into the system is described in [Including Systematics](MD_Systematics.html).
