

#source("Completion.R")
library(functional)
source("utils.R")
source("LocationsMap.R")


working_directory <- "~/tests"

test_suite_path <-
  paste(working_directory,
        "50_nodes-comp_radius_2_2018-03-29_11:42:11",
        sep = "/")


testNames <- function(testSuitePath) {
  simulationFilePaths <-
    list.files(paste(testSuitePath, "simulation_files", sep = "/"))
  simulationFilePaths <-
    tools::file_path_sans_ext(simulationFilePaths)
  return(simulationFilePaths)
}

createTestInfoRow <- function(testSuitePath, testName) {
  c(
    testName,
    tools::file_path_as_absolute(file.path(testSuitePath, "simulation_files", paste(testName,"csc", sep = "."))),
    tools::file_path_as_absolute(file.path(testSuitePath, testName))
  )
}

#' Loads paths of a test suite
#' @param testSuitePath The path to a test suite.
#' @return a data field with columns testName, simulationFile and the test directory path.
generateLocationPlotsForTestSuite <- function(testSuitePath) {
  if (!dir.exists(testSuitePath)) {
    stop("Bad path, testsuite does not exists.")
  }
  tests <- testNames(testSuitePath)
  rows <- lapply(tests, Curry(createTestInfoRow, testSuitePath))
  
  df <- as.data.frame( do.call(rbind, rows), stringsAsFactors=F )
  colnames(df) <- c("testName", "simulationFile", "testDirectory")
  
  apply(df, 1, function(row) {
    
    print(row["testName"])
    roundData <- load_all_nodes_round_data(row["testDirectory"])
    clusters <- clusterHeadIds(roundData)
    
    # Create node to cluster map
    a <- !duplicated(roundData[c("node_id","cluster_id")])
    roundDataSub <- subset(roundData, a)
    node_cluster_map <- roundDataSub[c("node_id","cluster_id")]
    
    pdf(file = file.path(row["testDirectory"], "locations.pdf"))
    plotNodeLocations(row["simulationFile"], clusters, node_cluster_map)
    dev.off()
    
  })
  
  return(1)
}


