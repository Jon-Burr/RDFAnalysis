cmake_minimum_required( VERSION 3.9 )

project( RDFAnalysis LANGUAGES CXX )

# ROOT version 6.16 at least is required as this introduces the RNode base class
# that makes this approach possible.
find_package( ROOT 6.16 REQUIRED COMPONENTS ROOTDataFrame )
find_package( Boost COMPONENTS program_options REQUIRED )

# Define the library created here
add_library( RDFAnalysis SHARED )

# Add the public headers
target_include_directories( RDFAnalysis
    PUBLIC
      include
      ${ROOT_INCLUDE_DIRS}
    PRIVATE
      ${Boost_INCLUDE_DIRS}
    )

# Add the sources
target_sources( RDFAnalysis
    PRIVATE
      src/DefaultBranchNamer.cxx
      src/Node.cxx
      src/OutputWriter.cxx
      src/TObjectWriter.cxx
      src/IBranchNamer.cxx
    )

# Link ROOT
target_link_libraries( RDFAnalysis
    PUBLIC
      ${ROOT_LIBRARIES}
    )

# Enforce C++14 features
target_compile_features( RDFAnalysis
    PUBLIC
      cxx_std_14
    )

add_executable( test-tree )
target_sources( test-tree
    PRIVATE
      util/test-tree.cxx
    )
target_include_directories( test-tree
    PRIVATE
      include
      ${ROOT_INCLUDE_DIRS}
      ${Boost_INCLUDE_DIRS}
    )
target_link_libraries( test-tree
    PRIVATE
      RDFAnalysis
      ${Boost_LIBRARIES}
    )

add_executable( rdf-test )
target_sources( rdf-test
    PRIVATE
      util/rdf-test)
target_include_directories( rdf-test
    PRIVATE
      ${ROOT_INCLUDE_DIRS}
      ${Boost_INCLUDE_DIRS}
    )
target_link_libraries( rdf-test
    PRIVATE
      ${ROOT_LIBRARIES}
      ${Boost_LIBRARIES}
    )
target_compile_features( rdf-test
    PRIVATE
      cxx_std_14
    )

add_executable( monohbb )
target_sources( monohbb
    PRIVATE
      util/monohbb.cxx
    )
target_include_directories( monohbb
    PRIVATE
      include
      ${ROOT_INCLUDE_DIRS}
      ${Boost_INCLUDE_DIRS}
    )
target_link_libraries( monohbb
    PRIVATE
      RDFAnalysis
      ${ROOT_LIBRARIES}
      ${Boost_LIBRARIES}
    )
