require(xml2)

#' Plots a tests mote locations and marks the cluster heads
#'
#' @param testPath Absolute path to a single test
#' @param clusterHeads A vector of the cluster heads
plotNodeLocations <- function(simulationFilePath, clusterHeads=c()) {
  root <- read_xml(simulationFilePath)
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
