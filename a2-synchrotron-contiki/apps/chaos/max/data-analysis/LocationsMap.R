require(xml2)
library(RColorBrewer) # For unique colors
library(plyr)


#' Plots a tests mote locations and marks the cluster heads
#'
#' @param testPath Absolute path to a single test
#' @param clusterHeads A vector of the cluster heads
plotNodeLocations <- function(simulationFilePath, clusterHeads=c(), node_cluster_map) {
  root <- read_xml(simulationFilePath)
  motes <- xml_find_all(root, ".//mote")
  
  ids <- as.numeric(xml_text(xml_find_all(root, ".//id")))
  xs  <- as.double(xml_text(xml_find_all(root, ".//x")))
  ys  <- as.double(xml_text(xml_find_all(root, ".//y")))
  
  # Get distinguishable colors
  qual_col_pals = brewer.pal.info[brewer.pal.info$category == 'qual',]
  col_vector = unlist(mapply(brewer.pal, qual_col_pals$maxcolors, rownames(qual_col_pals)))
  
  # Cluster ids need to be in the order of the node_ids when plotting.
  sortedByNodeId <- node_cluster_map[order(node_cluster_map["node_id"]),]

  # Mapping from cluster_id to color index
  clusters <- unique(sortedByNodeId[["cluster_id"]])
  minColors <- order(clusters)
  node_color_indexes <- mapvalues(sortedByNodeId[["cluster_id"]], from=clusters, to=minColors)
  
  # Get the colors by index.
  node_colors <- col_vector[node_color_indexes]

  node_sizes <- rep(2,length(ids))
  node_sizes <- replace(node_sizes, clusterHeads, 5)
  
  plot(ys~xs, ylim = rev(range(ys)), asp=1, col=node_colors, pch=16, cex=node_sizes, xlab="x", ylab="y")
  #text(ys~xs, labels=ids, col="white") # Write node_id on top of the nodes
}
