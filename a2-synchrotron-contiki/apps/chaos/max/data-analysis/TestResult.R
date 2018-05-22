library(memoise)

TestResult <- setClass(
  "TestResult",
  
  slots = c(
    testName = "character",
    simulationFile = "character",
    testDirectory = "character",
    reliability = "numeric",
    data = "data.frame",
    max_data = "data.frame",
    location_data = "data.frame"
  )
)

setGeneric(name="calculateSpread", def=function(theObject) {standardGeneric("calculateSpread")})
setGeneric(name="totalPowerUsage", def=function(theObject) {standardGeneric("totalPowerUsage")})
setGeneric(name="calculateReliability", def=function(theObject) {standardGeneric("calculateReliability")})

setMethod(f="calculateSpread", signature = "TestResult", definition = function(theObject) {
  loc <- theObject@location_data
  dist(cbind(loc$x,loc$y))
  coord <- cbind(loc$x, loc$y)
  return(max(as.matrix(dist(coord))))
})

setMethod(f="totalPowerUsage", signature = "TestResult", definition = function(theObject) {
  #browser()
  roundData <- theObject@data
  
  dat <- as.data.table(roundData)
  lastRoundForEachNode <- dat[,.SD[which.max(round)],by=node_id]
  return(sum(c(lastRoundForEachNode$all_cpu, lastRoundForEachNode$all_listen, lastRoundForEachNode$all_lpm, lastRoundForEachNode$all_transmit)))
})

setMethod(f="calculateReliability", signature = "TestResult", definition = function(theObject) {
  networkwide_max <- max(theObject@location_data$node_id)
  all_rounds <- unique(theObject@max_data$rd)

  roundData <- theObject@data
  maxData <- theObject@max_data

  round_result <- sapply(all_rounds, function(round) {
    #browser()
    cluster_heads <- roundData[roundData$round == round & roundData$node_id == roundData$cluster_id,]
    if(round %% 2 == 0) {
      cluster_head_done_max <- maxData[maxData$rd == round & maxData$node_id %in% cluster_heads$node_id,]
      return(all(cluster_head_done_max$max == networkwide_max, nrow(cluster_head_done_max) == nrow(cluster_heads)))
    } else {
      a <- all(apply(cluster_heads, 1, function(cluster_head) {
        #browser()
        nodes_in_cluster <- roundData[roundData$round == round & roundData$cluster_id == cluster_head[["cluster_id"]],]
        clusterwide_max <- max(nodes_in_cluster$node_id)
        #nodes_in_cluster$max ==
        nodes_done_max <- maxData[maxData$rd == round & maxData$cluster_id == cluster_head[["cluster_id"]],]
        all(nodes_done_max$max == clusterwide_max, nrow(nodes_done_max) == nrow(nodes_in_cluster))
      }))
      return(a)
    }

  })
  
  return(mean(round_result))
  
  # counts cluster failures as partial failures for a round. e.g. 0.333 for a round where 1 cluster succeeds and 2 fails.
  # 
  # all_cluster_ids <- unique(theObject@max_data$cluster_id)
  # g <- expand.grid(t(all_rounds), t(all_cluster_ids))
  # colnames(g) <- c("round","cluster")
  # 
  # #calculate completion rate for each row
  # g <- mdply(g, Curry(completion_rate, theObject@max_data, max(theObject@location_data$node_id)))
  # colnames(g)[3] <- "completion_rate"
  # g <- g[complete.cases(g),] # remove NA values
  # g <- arrange(g,round)
  # 
  # reliability <- mean(t(g["completion_rate"]))
  # 
  # return(reliability)
})

reliability <- memoise(calculateReliability)

max_count <- function(testResult) {
  testResult@
  return(5)
}
