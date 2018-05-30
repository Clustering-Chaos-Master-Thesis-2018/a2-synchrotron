

#source("Completion.R")
library(functional)
library(foreach)
library(doMC)
library(data.table)
library(memoise)

db <- cache_filesystem("~/.rcache")

source("TestResult.R")
source("utils.R")
source("LocationsMap.R")
source("Latency.R")
source("ApplicationHeatmap.R")
source("Reliability.R")
registerDoMC(4)


evaluation_directory <- "/home/kerp/Exjobb/Evaluation/"
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
      energy_data = load_all_nodes_round_data(file.path(row[3],"log/power")),
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
    stop(paste("Bad path, testsuite does not exist: ", testSuitePath))
  }
  tests <- testNames(testSuitePath)
  if(length(tests) == 0) {
    stop(paste("No tests found. Are the simulation files present? Test: ", testSuitePath))
  }
  
  rows <- lapply(tests, Curry(createTestInfoRow, testSuitePath))
  lapply(rows, loadResultFromTestInfoRow)
}

findAllTestsFromPath <- function(path) {
  allTests <- list.files(path, recursive = T, full.names = T, pattern = "cooja.log")
  allTestsWithLocations <- list.files(path, recursive = T, full.names = T, pattern = "locations.pdf")
  
  allTests <- unlist(lapply(allTests, dirname))
  allTestsWithLocations <- unlist(lapply(allTestsWithLocations, dirname))
  
  allTests <- unlist(allTests, use.names=FALSE)
  allTestsWithLocations <- unlist(allTestsWithLocations, use.names=FALSE)
  
  filteredTests <- setdiff(allTests, allTestsWithLocations)
  unique(unlist(lapply(filteredTests, dirname)))
}

generateAllLocationPlots <- function(path) {
  testSuites <- findAllTestsFromPath(evaluation_directory)
  testResults <- unlist(lapply(testSuites, loadResultsFromTestSuitePath))
  testResults <- testResults[!is.na(testResults)]
  browser()
  foreach(result = testResults) %dopar% {
    prepareAndPlotNodeLocations(result)
  }
 
}

main <- function(testSuitePath) {
  testResults <- loadResultsFromTestSuitePath(testSuitePath)

  #Remove NAs, errors of badly read tests should already have been logged above.
  testResults <- testResults[!is.na(testResults)]

  # Order by spread. 
  testResults <- testResults[order(sapply(testResults, calculateSpread))]

  foreach(result = testResults) %dopar% {
    print(paste("Location: ", result@testName))
    prepareAndPlotNodeLocations(result)
  }
}

# example:
# plotReliabilityForTestSuites(c(
#    "/Users/tejp/tests/evaluation-clustering-comp-1_2018-05-18_14:38:24/",
#    "/Users/tejp/tests/evaluation-clustering-comp-1-second-run_2018-05-19_17:56:34/",
#    "/Users/tejp/tests/evaluation-clustering-comp-1-third-run_2018-05-20_22:29:12")
# )
plotReliabilityForTestSuites <- function(testSuitePaths) {
  # Load data
  testResultss <- lapply(testSuitePaths, function(testSuitePath) loadResultsFromTestSuitePath(testSuitePath))
  
  probabilityStats <- mapply(function(testResult1, testResult2, testResult3) {

    res <- c(testResult1, testResult2, testResult3)
    res <- res[!is.na(res)]
    if(length(res[!is.na(res)]) == 0) {
      return(NA)
    }
    
    print(paste(lapply(res, function(r) {r@testName}), sep=" "))
    reliabilities <- sapply(res, function(r) reliability_of_test(r))
    data.frame(testName=res[[1]]@testName, mean=mean(reliabilities), sd=sd(reliabilities), spread=calculateSpread(res[[1]]))
    
  }, testResultss[[1]], testResultss[[2]], testResultss[[3]], SIMPLIFY = F)
  
  stats <- rbindlist(probabilityStats[!is.na(probabilityStats)])
  
  #order by spread
  stats$testName <- factor(stats$testName, levels = stats$testName[order(stats$spread)])
  
  p <- ggplot(stats, aes(testName, mean, ymin=mean-sd, ymax=mean+sd)) +
    geom_pointrange() +
    coord_cartesian(ylim = c(0,1)) +
    theme(axis.text.x=element_text(angle=45, hjust=1))
  print(p)
  return(p)
}

# example
# plotPowerUsageForTestSuites(c(
#    "/Users/tejp/tests/Cluster Testing/2018-05-19 30 min MinClusterSize2 test 2/",
#    "/Users/tejp/tests/Cluster Testing/2018-05-20 30 min MinClusterSize2 test 3/")
# )
plotPowerUsageForTestSuites <- function(testSuitePaths) {
  # Load data
  testResultss <- lapply(testSuitePaths, function(testSuitePath) loadResultsFromTestSuitePath(testSuitePath))
  
  powerStats <- mapply(function(testResult1, testResult2, testResult3) {
    
    res <- c(testResult1, testResult2, testResult3)
    res <- res[!is.na(res)]
    if(length(res[!is.na(res)]) == 0) {
      return(NA)
    }
    
    print(paste(lapply(res, function(r) {r@testName}), sep=" "))
    total_power_usages <- sapply(res, function(r) totalPowerUsage(r))
    data.frame(testName=res[[1]]@testName, mean=mean(total_power_usages), sd=sd(total_power_usages), spread=calculateSpread(res[[1]]))
    
  }, testResultss[[1]], testResultss[[2]], testResultss[[3]], SIMPLIFY = F)
  
  stats <- rbindlist(powerStats[!is.na(powerStats)])
  
  #order by spread
  stats$testName <- factor(stats$testName, levels = stats$testName[order(stats$spread)])
  
  p <- ggplot(stats, aes(testName, mean, ymin=mean-sd, ymax=mean+sd)) +
    geom_pointrange() +
    coord_cartesian(ylim = c(0,2.5*10^9)) + #max(stats$mean+stats$sd+(stats$mean+stats$sd)/10, na.rm = T)) ) +
    theme(axis.text.x=element_text(angle=45, hjust=1))
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
