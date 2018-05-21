

#source("Completion.R")
library(functional)
library(foreach)
library(doMC)
source("TestResult.R")
source("utils.R")
source("LocationsMap.R")
source("Latency.R")
source("ApplicationHeatmap.R")
source("Reliability.R")
registerDoMC(4)


working_directory <- "~/tests"

loadResultFromTestInfoRow <- function(row) {
  tryCatch({
    TestResult(
      testName = row[1],
      simulationFile = row[2],
      testDirectory = row[3],
      reliability = -1,
      data = load_all_nodes_round_data(file.path(row[3],"log/round")),
      max_data = load_all_nodes_round_data(file.path(row[3],"log/max")),
      location_data = load_location_data(row[2])
    )
  }, error = function(e) {
    message(paste(e, row[1], sep=""))
    return(NA)
  }
  )
}

loadResultsFromTestSuitePath <- function(testSuitePath) {
  if (!dir.exists(testSuitePath)) {
    stop("Bad path, testsuite does not exists.")
  }
  tests <- testNames(testSuitePath)
  if(length(tests) == 0) {
    stop("No tests found. Are the simulation files present?")
  }
  
  rows <- lapply(tests, Curry(createTestInfoRow, testSuitePath))
  lapply(rows, loadResultFromTestInfoRow)
}

main <- function(testSuitePath) {
  
  testResults <- loadResultsFromTestSuitePath(testSuitePath)

  #Remove NAs, errors of badly read tests should already have been logged above.
  testResults <- testResults[!is.na(testResults)]

  # Order by spread. 
  testResults <- testResults[order(sapply(testResults, calculateSpread))]

  pdf(file = file.path(testSuitePath, "latency.pdf"))
  plotLatency(testResults)
  dev.off()
  
  
  # foreach(result = testResults)  %dopar% {
  #   print(paste("Location: ", result@testName))
  #   prepareAndPlotNodeLocations(result)
  # }
  for (result in testResults) {
    print(paste("Location: ", result@testName))
    prepareAndPlotNodeLocations(result)
  }
  
  for (result in testResults) {
    print(paste("Heatmap: ", result@testName))
    p <- plotHeatmap(result)
    ggsave(file.path(result@testDirectory,"applications.pdf"), plot=p)
  }
  
  # foreach(result = testResults) %dopar% {
  #   print(paste("Heatmap: ", result@testName))
  #   plotHeatmap(result)
  # }
  
  
}

# example:
# plotReliabilityForTestSuites(c(
#    "/Users/tejp/tests/Cluster Testing/2018-05-19 30 min MinClusterSize2 test 2/",
#    "/Users/tejp/tests/Cluster Testing/2018-05-20 30 min MinClusterSize2 test 3/")
# )
plotReliabilityForTestSuites <- function(testSuitePaths) {
  # Load data
  testResultss <- lapply(testSuitePaths, function(testSuitePath) loadResultsFromTestSuitePath(testSuitePath))
  
  probabilityStats <- mapply(function(testResult1, testResult2) {
    
    res <- c(testResult1, testResult2)
    res <- res[!is.na(res)]
    if(length(res[!is.na(res)]) == 0) {
      return(NA)
    }
    
    print(paste(lapply(res, function(r) {r@testName}), sep=" "))
    reliabilities <- sapply(res, function(r) reliability_of_test(r))
    data.frame(testName=res[[1]]@testName, mean=mean(reliabilities), sd=sd(reliabilities), spread=calculateSpread(testResult1))
    
  }, testResultss[[1]], testResultss[[2]])
  
  stats <- rbindlist(probabilityStats[!is.na(probabilityStats)])
  
  #order by spread
  stats$testName <- factor(stats$testName, levels = stats$testName[order(stats$spread)])
  
  p <- ggplot(stats, aes(testName, mean, ymin=mean-sd, ymax=mean+sd)) + geom_pointrange()
  print(p)
  return(p)
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
