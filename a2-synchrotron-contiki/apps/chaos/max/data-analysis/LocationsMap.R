require(xml2)

#' Generates plots for each test setup in a test suite.
#'
#' @param testSuitePath Absolute path to a test suite.
#' @param testToClusterHeadsMap a mapping of a test name to the cluster heads elected in that test.
generateLocationPlotsForTestSuite <- function(testSuitePath, testToClusterHeadMap) {
  
  simulationFilePaths <- list.files(paste(wd, testSuitePath, "simulation_files", sep="/"), full.names = T)
  
  
  sapply(simulationFilePaths, plotNodeLocations)
}


#' Plots a tests mote locations and marks the cluster heads
#'
#' @param testPath Absolute path to a single test
#' @param clusterHeads A vector of the cluster heads
plotNodeLocations <- function(testPath, clusterHeads) {
  print(testPath)
  root <- read_xml(testPath)
  motes <- xml_find_all(root, ".//mote")
  
  
  
  ids <- as.numeric(xml_text(xml_find_all(root, ".//id")))
  xs  <- as.double(xml_text(xml_find_all(root, ".//x")))
  ys  <- as.double(xml_text(xml_find_all(root, ".//y")))
  
  node_colors <- rep("black",length(ids))
  node_colors <- replace(node_colors, clusterHeads, "red")
  node_sizes <- rep(2,length(ids))
  node_sizes <- replace(node_sizes, clusterHeads, 5)
  
  plot(ys~xs, ylim = rev(range(ys)), asp=1, col=node_colors, pch=16, cex=node_sizes, xlab="x", ylab="y")
}

#text(xs, ys, labels=ids, cex= 0.7, offset = 0.6, pos = 1)
