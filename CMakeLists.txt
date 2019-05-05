cmake_minimum_required( VERSION 3.9 )

# ROOT version 6.16 at least is required as this introduces the RNode base class
# that makes this approach possible.
find_package( ROOT 6.16 REQUIRED COMPONENTS ROOTDataFrame Physics )
find_package( Boost COMPONENTS program_options REQUIRED )
include(${ROOT_USE_FILE})

# Define the library created here
add_library( RDFAnalysis )

# Add the public headers
target_include_directories( RDFAnalysis
    PUBLIC
      include
    )

# Add the sources
target_sources( RDFAnalysis
    PRIVATE
      src/DefaultBranchNamer.cxx
    )

# Link ROOT
target_link_libraries( RDFAnalysis
    PUBLIC
      ROOT::ROOTDataFrame
    PRIVATE
      Boost::boost
    )

# Enforce C++14 features
target_compile_features( RDFAnalysis
    PUBLIC
      cxx_std_14
    )

add_executable( monohbb )
target_sources( monohbb
    PRIVATE
      util/monohbb.cxx
    )
target_link_libraries( monohbb
    PRIVATE
      RDFAnalysis
      Boost::program_options
    )

add_executable( jetres )
target_sources( jetres
    PRIVATE
      util/jetres.cxx
    )
target_link_libraries( jetres
    PRIVATE
      RDFAnalysis
      Boost::program_options
    )
