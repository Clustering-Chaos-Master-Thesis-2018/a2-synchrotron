library(jsonlite)

load_all_nodes_round_data <- function(test_path) {
  round_files_path <- paste(c(test_path,"log/round"), collapse = "/")
  log_files <- list.files(path=round_files_path, pattern="*.txt", full.names=T, recursive=FALSE)
  info <- file.info(log_files)
  log_files <- rownames(info[info$size != 0, ])
  
  if (length(log_files) == 0) {
    stop("No log files")
  }
  
  tables <- lapply(log_files, function(path) read.table(path, header = T))
  table = Reduce(function(x,y) merge(x,y, all=T), tables)
  #colnames(table) <- c("round", "max", "complete_slot", "off_slot", "node_id", "perceived_cluster_size", "cluster_id")
  return(table)
  #dfs <- lapply(log_files, function(x) fromJSON(x, flatten=TRUE))
  #dfs_order <- lapply(dfs, function(x) x[["nodeid"]][[1]]) # extract nodeid from each data frame
  #dfs_internally_sorted <- dfs[order(unlist(dfs_order))] # sort using the nodeid
}

clusterHeadIds <- function(roundData) {
  as.vector(unique(roundData[["cluster_id"]]))
}

load_location_data <- function(simulationFilePath) {
  root <- read_xml(simulationFilePath)
  motes <- xml_find_all(root, ".//mote")
  
  # Fetch data from xml file
  node_id <- as.numeric(xml_text(xml_find_all(root, ".//id")))
  x  <- as.double(xml_text(xml_find_all(root, ".//x")))
  y  <- as.double(xml_text(xml_find_all(root, ".//y")))
  nodes <- data.frame(node_id,x,y)
  return(nodes)
}