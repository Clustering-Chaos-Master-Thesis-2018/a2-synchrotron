

#source("Completion.R")
library(functional)
source("utils.R")
source("LocationsMap.R")
source("Latency.R")


working_directory <- "~/tests"

main <- function(testSuitePath) {
  if (!dir.exists(testSuitePath)) {
    stop("Bad path, testsuite does not exists.")
  }
  tests <- testNames(testSuitePath)
  rows <- lapply(tests, Curry(createTestInfoRow, testSuitePath))

  testResults <- lapply(rows, function(row) {
    tryCatch({
      TestResult(
        testName = row[1],
        simulationFile = row[2],
        testDirectory = row[3],
        data = load_all_nodes_round_data(row[3]),
        location_data = load_location_data(row[2])
      )
    }, error = function(e) {
      message(paste(e, row[1], sep=""))
      return(NA)
    }
    )
  })

  #Remove NAs, errors of badly read tests should already have been logged above.
  testResults <- testResults[!is.na(testResults)]

  # Order by spread. 
  testResults <- testResults[order(sapply(testResults, calculateSpread))]

  
  pdf(file = file.path(testSuitePath, "latency.pdf"))
  plotLatency(testResults)
  dev.off()
}

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
    tryCatch({
        loadAndPlot(row)
      }, error = function(e) {
        message(e)
        return(NA)
      }
    )


  })

  return(1)
}

loadAndPlot <- function(row) {
  print(row["testName"])

  roundData <- load_all_nodes_round_data(row["testDirectory"])

  max_round <- max(roundData$rd, na.rm = TRUE)
  pdf(file = file.path(row["testDirectory"], "locations.pdf"))
  for (round in 1:max_round){
    filteredRoundData <- roundData[roundData$rd > round,]
    clusters <- clusterHeadIds(filteredRoundData)

    # Create node to cluster map
    a <- !duplicated(filteredRoundData[c("node_id")])
    roundDataSub <- subset(filteredRoundData, a)
    node_cluster_map <- roundDataSub[c("node_id","cluster_id")]
    plotNodeLocations(row["simulationFile"], clusters, node_cluster_map)

  }
  dev.off()
}

TestResult <- setClass(
  "TestResult",
  
  slots = c(
    testName = "character",
    simulationFile = "character",
    testDirectory = "character",
    data = "data.frame",
    location_data = "data.frame"
    )
)



setGeneric(name="calculateSpread", def=function(theObject) {standardGeneric("calculateSpread")})

setMethod(f="calculateSpread", signature = "TestResult", definition = function(theObject) {
  loc <- theObject@location_data
  dist(cbind(loc$x,loc$y))
  coord <- cbind(loc$x, loc$y)
  return(max(as.matrix(dist(coord))))
})
