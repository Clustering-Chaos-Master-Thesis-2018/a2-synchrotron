library(functional) # For currying
source("utils.R")

completion_rate <- function(max_data, max_in_CH_round, round, cluster) {
  round_entries <- max_data[max_data$rd == round,]
  
  if(round %% 2 == 0) {
    entries_in_cluster <- round_entries
  } else {
    entries_in_cluster <- round_entries[round_entries$cluster_id == cluster,]  
  }
  
  
  if(empty(entries_in_cluster)) {
    return(NA)
  }
  
  if(round %% 2 == 0) {
    correct_max <- max_in_CH_round
  } else {
    correct_max <- max(entries_in_cluster[["node_id"]])
  }
  
  correct_count = nrow(entries_in_cluster[entries_in_cluster$max == correct_max,])
  total_count = nrow(entries_in_cluster)
  
  return(correct_count==total_count)
}


reliability_of_test <- function(testResult) {
  # All combinations of rounds and cluster ids
  all_rounds <- unique(testResult@max_data$rd)
  all_cluster_ids <- unique(testResult@max_data$cluster_id)
  g <- expand.grid(t(all_rounds), t(all_cluster_ids))
  colnames(g) <- c("round","cluster")
   
  #calculate completion rate for each row
  g <- mdply(g, Curry(completion_rate, testResult@max_data, max(testResult@location_data$node_id)))
  colnames(g)[3] <- "completion_rate"
  g <- g[complete.cases(g),] # remove NA values
  g <- arrange(g,round)
   
  reliability <- mean(t(g["completion_rate"]))
  
  return(reliability)
}
