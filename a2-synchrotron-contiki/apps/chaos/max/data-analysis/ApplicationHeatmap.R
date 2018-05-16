
library(ggplot2)
library(reshape2)

plotHeatmap <- function(testRestult) {
  
  print(testRestult@testName)
  app <- c("association", "sleeping", "cluster", "join", "demote", "chaos_max_app")
  color <- 1:length(app)

  appToColorTable <- data.frame(app, color, stringsAsFactors = F)
  
  node_ids <- sort(unique(testRestult@data$node_id))
  
  heatData <- matrix(ncol=max(node_ids), nrow=max(testRestult@data$round))
  
  dat <- testRestult@data
  for (round in unique(dat$round)) {
    for (node_id in unique(dat$node_id)) {
      
      dataRow <- dat[dat$round==round & dat$node_id == node_id,]
      if(nrow(dataRow) == 1) {
        heatData[round,node_id] = appToColorTable[appToColorTable$app==dataRow$app,]$color
      }
      
    }
  }

  roundData <- testRestult@data
  roundData <- roundData[c("round", "node_id", "app")]
  appToColorTable[appToColorTable$app==dataRow$app,]$color
  roundData <- merge(roundData, appToColorTable, by="app")
  
  ySteps <- 1:max(roundData$node_id)
  xSteps <- 1:max(roundData$round)
  
  p <- ggplot(roundData, aes(round, node_id, fill=app)) +
    geom_raster() +
    geom_tile(colour="white",size=0.25) +
    scale_y_discrete(expand=c(0,0), limits=ySteps, labels=ySteps, breaks=ySteps) +
    scale_x_discrete(expand=c(0,0), limits=xSteps, labels=xSteps ,breaks=xSteps) +
    coord_fixed()+
    labs(x="Round",y="Node ID") +
    scale_color_brewer()
  
  ggsave(file.path(testRestult@testDirectory,"applications.pdf"), plot=p)
  
}