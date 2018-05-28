library(plotly)
library(jsonlite)
library(plyr)

working_directory <- "repos/a2-synchrotron/a2-synchrotron-contiki/apps/chaos/max/tests/givemedata_2018-03-22_14:28:35/30-motes-1000x1000-spread/"
log_files <- list.files(path="log", pattern="*.txt", full.names=T, recursive=FALSE)

# list of data frames
dfs <- lapply(log_files, function(x) fromJSON(x, flatten=TRUE))
dfs_order <- lapply(dfs, function(x) x[["nodeid"]][[1]]) # extract nodeid from each data frame
dfs_internally_sorted <- dfs[order(unlist(dfs_order))] # sort using the nodeid

dfs_ <- lapply(dfs_internally_sorted, function(x) t(data.matrix(x[,c("cpu")]))) # only keep the app_cpu column in each df

heat_matrix = rbind.fill.matrix(dfs_)
plot_ly(z = heat_matrix, type="heatmap")
#heat_matrix = rbind.fill.matrix(dfs[,c("all_cpu")])

# total <- Reduce(function(x,y) {
#   merge(x,y, all=TRUE)
# }, dfs)
# 
# lapply(log_files, function(x) {
#   
#   total <- merge.data.frame(total, stuff)
# })
# 
# colnames(stuff)
# stuff[,c("all_cpu","all_transmit")]
# 
# #p <- plot_ly(midwest, x = stuff[,c("clock_time")], y = stuff[,c("nodeid"), z = stuff[,c("all_cpu")]], color = ~state, type = "heatmap")
# # p <- plot_ly(midwest,
# #              x = stuff,
# #              color = ~ state,
# #              type = "heatmap")
# # p
# qplot(stuff$clock_time, stuff$all_cpu)
