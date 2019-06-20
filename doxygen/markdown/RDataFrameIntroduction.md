# Introduction to RDataFrame # {#MD_RDataFrameIntro}

For a full introduction to RDataFrame (including tutorials) go to the [official documentation](https://root.cern/doc/v616/classROOT_1_1RDataFrame.html).
This section is intended only as a brief overview, covering the most relevant points for this package

The RDataFrame uses the same analysis model described [previously](MD_AnalysisAsTree.html).
The RDataFrame makes a separation between **transformations** which take a data frame as an input and return a new data frame, either with some new information included (through a **define** call) or with some selection applied (through a **filter** call) and **actions** which take a data frame as an input and return some other piece of information such as a filled histogram.
These map exactly onto the concepts introduced before, with **defines** and **filters** the same as before and **actions** referred to as **fills**.

In the RDataFrame, **filters** and **defines** can be provided either as a function and an accompanying list of arguments or as a string expression that is just-in-time (JIT) compiled by the ROOT interpreter.

A nice feature of RDataFrame is that all define calls are lazy.
This means that while they may be registered anywhere in the computational graph, they will only be evaluated where they are used.
This means that typically there is no reason not to place all define calls at the start of the graph, so long that care is taken to ensure that they are only used where they are valid.
