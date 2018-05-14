library(xml2)
library(RColorBrewer) # For unique colors
library(plyr)


prepareAndPlotNodeLocations <- function(testResult) {
  #print(row["testName"])
  
  #roundData <- load_all_nodes_round_data(row["testDirectory"])
  roundData <- testResult@max_data
  max_round <- max(roundData$rd, na.rm = TRUE)
  pdf(file = file.path(testResult@testDirectory, "locations.pdf"))
  for (round in 1:max_round){
    filteredRoundData <- roundData[roundData$rd > round,]
    clusters <- clusterHeadIds(filteredRoundData)
    
    # Create node to cluster map
    a <- !duplicated(filteredRoundData[c("node_id")])
    roundDataSub <- subset(filteredRoundData, a)
    node_cluster_map <- roundDataSub[c("node_id","cluster_id")]
    plotNodeLocations(testResult@simulationFile, clusters, node_cluster_map)
    
  }
  dev.off()
}


#' Plots a tests mote locations and marks the cluster heads
#'
#' @param testPath Absolute path to a single test
#' @param clusterHeads A vector of the cluster heads
plotNodeLocations <- function(simulationFilePath, clusterHeads=c(), node_cluster_map) {
  root <- read_xml(simulationFilePath)
  motes <- xml_find_all(root, ".//mote")
  
  # Fetch data from xml file
  node_id <- as.numeric(xml_text(xml_find_all(root, ".//id")))
  x  <- as.double(xml_text(xml_find_all(root, ".//x")))
  y  <- as.double(xml_text(xml_find_all(root, ".//y")))
  nodes <- data.frame(node_id,x,y)
  
  # Merge data from log files with data from xml file
  nodes <- merge(node_cluster_map, nodes, by = "node_id", all = TRUE)
  
  # Mapping from cluster_id to color index
  clusters <- unique(nodes[["cluster_id"]])
  minColors <- order(clusters)
  
  # Add color column
  # Get distinguishable colors
  qual_col_pals = brewer.pal.info[brewer.pal.info$category == 'qual',]
  col_vector = unlist(mapply(brewer.pal, qual_col_pals$maxcolors, rownames(qual_col_pals)))
  nodes$node_color_indexes <- mapvalues(nodes[["cluster_id"]], from=clusters, to=minColors)
  nodes$color <- with(nodes, col_vector[node_color_indexes])

  # Add size column
  nodes$node_sizes <- rep(2,length(nodes[["node_id"]]))
  nodes$node_sizes <- replace(nodes$node_sizes, clusterHeads, 5)
  
  plot(nodes$y~nodes$x, ylim = rev(range(nodes$y)), asp=1, col=nodes$color, pch=16, cex=nodes$node_sizes, xlab="x", ylab="y")
  text(nodes$y~nodes$x, labels=nodes$node_id, col="black") # Write node_id on top of the nodes
}
