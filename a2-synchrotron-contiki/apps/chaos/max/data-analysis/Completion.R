working_directory <- "repos/a2-synchrotron/a2-synchrotron-contiki/apps/chaos/max/log/"
#setwd(working_directory)

log_files <- list.files(path="round", pattern="*.txt", full.names=T, recursive=FALSE)
info <- file.info(log_files)
log_files <- rownames(info[info$size != 0, ])


tables <- lapply(log_files, function(path) read.table(path))

#colnames(tables[[1]]) <- c("rd", "max", "fin", "node_id", "n", "cluster_id")
#newNames <- c("rd", "max", "fin", "node_id", "n", "cluster_id")
#tables <- lapply(tables, setNames,  newNames)


table = Reduce(function(x,y) merge(x,y, all=T), tables)
colnames(table) <- c("rd", "max", "fin", "node_id", "n", "cluster_id")


completion_rate <- function(round, cluster) {
  round_entries <- table[table$rd == round,]
  entries_in_cluster <- round_entries[round_entries$cluster_id == cluster,]
  
  correct_max <- max(entries_in_cluster[["node_id"]])
  correct_count = nrow(entries_in_cluster[entries_in_cluster$max == correct_max,])
  total_count = nrow(entries_in_cluster)
  
  return(correct_count/total_count)
}


all_rounds <- unique(table["rd"])
all_cluster_ids <- unique(table["cluster_id"])
g <- expand.grid(t(all_rounds),t(all_cluster_ids))
colnames(g) <- c("round","cluster")
g <- mdply(g, completion_rate)
colnames(g)[3] <- "completion_rate"
g <- g[complete.cases(g),]
g <- arrange(g,round)

reliability <- mean(t(g["completion_rate"]))
print(reliability)