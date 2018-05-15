
library(ggplot2)
library(reshape2)

plotHeatmap <- function(testRestult) {
  
  print(testRestult@testName)
  app <- c("association", "sleeping", "cluster", "chaos_max_app", "join", "demote")
  color <- 1:length(app)

  appToColorTable <- data.frame(app, color, stringsAsFactors = F)
  
  node_ids <- sort(unique(testRestult@data$node_id))
  
  heatData <- matrix(ncol=length(node_ids), nrow=max(testRestult@data$round))
  
  dat <- testRestult@data
  for (round in unique(dat$round)) {
    for (node_id in unique(dat$node_id)) {
      
      dataRow <- dat[dat$round==round & dat$node_id == node_id,]
      if(nrow(dataRow) == 1) {
        heatData[round,node_id] = appToColorTable[appToColorTable$app==dataRow$app,]$color
      }
      
    }
  }
  print("helo")
  #browser()
  p <- ggplot(melt(heatData), aes(Var1,Var2, fill=value)) +
    geom_raster() +
    geom_tile(colour="white",size=0.25) +
    scale_y_discrete(expand=c(0,0), breaks=1:50) +
    scale_x_discrete(expand=c(0,0), breaks=1:50) +
    coord_fixed()+
    labs(x="Round",y="Node ID")
  print(p)
  browser()
  #ggplot(nba.m, aes(variable, Name)) + geom_tile(aes(fill = rescale), colour = "white") + scale_fill_gradient(low = "white", high = "steelblue")
  
}