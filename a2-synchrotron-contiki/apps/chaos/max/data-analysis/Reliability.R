library(plyr)       # For mdplyr, applying a table row as args to a function.
library(functional) # For currying
library(magrittr)   # For piping %>%
source("utils.R")

#working_directory <- "~/repos/a2-synchrotron/a2-synchrotron-contiki/apps/chaos/max/"
working_directory <- "~/tests" # the tests folder
#working_directory <- "~/tests/50_nodes-comp_radius_1_2018-03-27_14:57:13/50-motes-1000x1000-spread/log"
#working_directory <- "~/tests/50_nodes-comp_radius_1_2018-03-27_14:57:13/50-motes-100x100-spread/log"
#setwd(working_directory)

#test_suite_name <- "50_nodes-comp_radius_1_2018-03-28_11:28:53"
#test_suite_name <- "50_nodes-comp_radius_1_2018-03-29_11:41:46"
test_suite_path <- paste(working_directory, "50_nodes-comp_radius_2_2018-03-29_11:42:11", sep="/")
#test_suite_name <- "50_nodes-comp_radius_3_2018-03-29_11:42:29"

#test_suite_name <-  "50_nodes-comp_radius_1_2018-03-27_14:57:13"

#table100x100 <- load_all_nodes_round_data(test_path = paste(test_suite_path,"50-motes-100x100-spread", sep = "/"))
#table500x500 <- load_all_nodes_round_data(test_path = paste(test_suite_path,"50-motes-500x500-spread", sep = "/"))
#table1000x1000 <- load_all_nodes_round_data(test_path = paste(test_suite_path,"50-motes-1000x1000-spread", sep = "/"))

#tables <- list(table100x100, table500x500, table1000x1000)

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
  
  if (total_count == 0) {
    return(NA)
  } else {
    return(correct_count==total_count)  
  }
}

load_test_suite  <- function(test_suite_path) {
  static_files <- c("information.txt", "simulation_files")
  files <- list.files(test_suite_path)
  test_names <- setdiff(files,static_files)
  test_names <- c(test_names[2],test_names[3],test_names[1]) #temporary order fix
  
  
  test_names %>%
    paste(test_suite_path, ., sep = "/") %>%   # dot '.' is for each item in the list
    lapply(., load_all_nodes_round_data)
  
  
  
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

cluster_count <- function(table) {
  table[["cluster_id"]] %>%
  unique() %>%
  length()
}

create_status_row <- function(table) {
  c(reliability_of_test(table),cluster_count(table))  
}

red <- rgb(0.7,0.1,0.1,0.8)
green <- rgb(0.1,0.7,0.1,0.8)
blue <- rgb(0.1,0.1,0.7,0.8)

plot_reliability_result <- function(result, yMax, yMin) {
  range <- yMax - yMin
  plot(result[1,], ylim=c(yMin-range*0.1,yMax+range*0.1), col=green, lwd=3, type="b", pch=20, cex=4, bty="l", ylab = "Reliability", xlab = "Node spread", xaxt='n')
  
  axis(1, at=1:3, labels=c("100", "500", "1000"))
  #lines(result[1,]-0.00001, ylim=c(mi-range*0.1,ma+range*0.1), col=rgb(0.1,0.7,0.1,0.8), lwd=3, type="b", pch=20, cex=4, bty="l", ylab = "reliability", xlab = "competition radius")
  #legend(x=1,y=0.5669, c("1","2","3"), col=c(green,red,blue), pch=16, title = "Competition radius")
  legend("bottomleft",c("1","2","3"),, col=c(green,red,blue), pch=16, title = "Competition radius")
  #text(result[1,])
  
}

line_reliability_result <- function(result, color) {
  ma <- max(result[1,])
  mi <- min(result[1,])
  range <- ma - mi
  
  #plot(result[1,], ylim=c(mi-range*0.1,ma+range*0.1), col=rgb(0.1,0.7,0.1,0.8), lwd=3, type="b", pch=20, cex=4, bty="l", ylab = "reliability", xlab = "node spread")
  lines(result[1,], ylim=c(mi-range*0.1,ma+range*0.1), col=color, lwd=3, type="b", pch=20, cex=4, bty="l", ylab = "reliability", xlab = "competition radius")
  
  #text(result[1,])
  
}

plot_test_suite_results <- function(tables) {
  tables %>% reliability_of_test
}


# # test1 <- "50_nodes-comp_radius_1_2018-03-29_11:41:46"
# # test2 <- "50_nodes-comp_radius_2_2018-03-29_11:42:11"
# # test3 <- "50_nodes-comp_radius_3_2018-03-29_11:42:29"
# 
# test1 <- "50_nodes-comp_radius_1_2018-04-03_22:03:08"
# test2 <- "50_nodes-comp_radius_2_2018-03-29_11:42:11"
# test3 <- "50_nodes-comp_radius_3_2018-03-29_11:42:29"
# 
# test_suite_path <- paste(working_directory, test1, sep="/")
# tables1 <- load_test_suite(test_suite_path)
# result1 <- sapply(tables1, create_status_row)
# 
# 
# test_suite_path <- paste(working_directory, test2, sep="/")
# tables2 <- load_test_suite(test_suite_path)
# result2 <- sapply(tables2, create_status_row)
# 
# 
# test_suite_path <- paste(working_directory, test3, sep="/")
# tables3 <- load_test_suite(test_suite_path)
# result3 <- sapply(tables3, create_status_row)
# 
# # to find y bounds
# results <- c(result1[1,],result2[1,],result3[1,])
# 
# plot_reliability_result(result1, max(results), min(results))
# line_reliability_result(result2, red)
# line_reliability_result(result3, blue)
# 
# #plot_reliability_result(result)
# #print(result)
# 
# #print(reliability_of_test(table100x100))
# #print(reliability_of_test(table500x500))
# #print(reliability_of_test(table1000x1000))