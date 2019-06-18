# Modelling an Analysis as a Tree # {#MD_AnalysisAsTree}

The central idea of this package is to model an analysis as a series of operations on the data in a ROOT TTree, arranged in a tree structure (note that 'tree' here refers to the *data structure* which is a distinct concept to ROOT's TTree).
There are three main types of transformation in an analysis

* Defining a new variable (a **define**)
* Applying a selection (or **filter**)
* Creating some output (called a **fill**)
 
Applying new selections can create a branching structure as different filters divide the initial set of events into multiple **regions**.

As an example, consider an analysis looking for the Z&gamma; production with the subsequent decay Z &rarr; l<sup>+</sup>l<sup>-</sup>.
The analysis wishes to separate this into electron and muon decays, then fill histograms with the di-lepton mass spectrum in both cases.
A very basic tree structure for this analysis would be

![](example_basic_sel.png)

where diamonds represent filters and squares represent fills.
This convention will be used throughout the rest of this document.
When they appear, defines will be represented by ovals.
