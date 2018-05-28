
library(ggplot2)
library(reshape2)

plotHeatmap <- function(testResult, round_interval=range(testResult@data$round)) {
  
  
  print(testResult@testName)
  app <- c("association", "sleeping", "cluster", "join", "demote", "chaos_max_app")
  color <- 1:length(app)

  appToColorTable <- data.frame(app, color, stringsAsFactors = F)
  
  node_ids <- sort(unique(testResult@data$node_id))

  roundData <- testResult@data
  roundData <- roundData[roundData$round %in% seq(round_interval[[1]],round_interval[[2]]),]
  roundData <- roundData[c("round", "node_id", "app")]
  roundData <- merge(roundData, appToColorTable, by="app")
  
  points <-  9
  step <-  max(roundData$round)/points
  
  
  
  ySteps <- c(1,seq(5,max(roundData$node_id),5), max(roundData$node_id))
  xSteps <- floor(c(1, seq(step, max(roundData$round)-step, step), max(roundData$round)))
  #browser()
  p <- ggplot(roundData, aes(round, node_id, fill=app)) +
    geom_raster() +
    geom_tile(colour="white",size=0.25) +
    scale_y_discrete(expand=c(0,0), limits=ySteps, labels=ySteps, breaks=ySteps) +
    scale_x_discrete(expand=c(0,0), limits=xSteps, labels=xSteps ,breaks=xSteps) +
    coord_fixed()+
    labs(x="Round",y="Node ID") +
    scale_color_brewer()
  
  return(p)
}