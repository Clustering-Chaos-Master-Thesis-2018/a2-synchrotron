library(memoise)

db <- cache_filesystem("~/.rcache")

TestResult <- setClass(
  "TestResult",
  
  slots = c(
    testName = "character",
    simulationFile = "character",
    testDirectory = "character",
    reliability = "numeric",
    data = "data.frame",
    max_data = "data.frame",
    energy_data = "data.frame",
    location_data = "data.frame"
  )
)

setGeneric(name="calculateSpread", def=function(theObject) {standardGeneric("calculateSpread")})
setGeneric(name="totalPowerUsage", def=function(theObject) {standardGeneric("totalPowerUsage")})
setGeneric(name="calculateReliability", def=function(theObject) {standardGeneric("calculateReliability")})
setGeneric(name="calculateWeakReliability", def=function(theObject) {standardGeneric("calculateWeakReliability")})
setGeneric(name="meanOffSlot", def=function(theObject) {standardGeneric("meanOffSlot")})
setGeneric(name="sdOffSlot", def=function(theObject) {standardGeneric("sdOffSlot")})


setMethod(f="calculateSpread", signature = "TestResult", definition = function(theObject) {
  loc <- theObject@location_data
  dist(cbind(loc$x,loc$y))
  coord <- cbind(loc$x, loc$y)
  return(max(as.matrix(dist(coord))))
})

setMethod(f="totalPowerUsage", signature = "TestResult", definition = function(theObject) {
  #browser()
  energyData <- theObject@energy_data
  
  return(
    sum(energyData$cpu) + sum(energyData$lpm) + sum(energyData$transmit) + sum(energyData$listen) + sum(energyData$idle_transmit) + sum(energyData$idle_listen)
  )
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
      if(nrow(cluster_heads) == 0){
        return(FALSE)
      }
      a <- all(apply(cluster_heads, 1, function(cluster_head) {
        nodes_in_cluster <- roundData[roundData$round == round & roundData$cluster_id == cluster_head[["cluster_id"]],]
        clusterwide_max <- max(nodes_in_cluster$node_id)
        #nodes_in_cluster$max ==
        nodes_done_max <- maxData[maxData$rd == round & maxData$cluster_id == cluster_head[["cluster_id"]],]
        all(nodes_done_max$max == clusterwide_max, nrow(nodes_done_max) == nrow(nodes_in_cluster))
      }))
      return(a)
    }

  })

  expectedMaxRounds <- 599-105
  if(expectedMaxRounds - length(round_result) > 0) {
    round_result <- c(round_result,  rep(F, expectedMaxRounds - length(round_result)))
  }

  return(mean(round_result))
})

reliability <- memoise(calculateReliability, cache=db)

setMethod(f="calculateWeakReliability", signature = "TestResult", definition = function(theObject) {
  # counts cluster failures as partial failures for a round. e.g. 0.333 for a round where 1 cluster succeeds and 2 fails.
  
  all_rounds <- unique(theObject@max_data$rd)
  if (is.null(all_rounds) ) {
    warning(paste("All rounds is null. There is no max data for ", theObject@testDirectory))
    return(0)
  }
  
  all_cluster_ids <- unique(theObject@max_data$cluster_id)
  g <- expand.grid(t(all_rounds), t(all_cluster_ids))
  colnames(g) <- c("round","cluster")
  
  #calculate completion rate for each row
  g <- mdply(g, Curry(completion_rate, theObject@max_data, max(theObject@location_data$node_id)))
  colnames(g)[3] <- "completion_rate"
  g <- g[complete.cases(g),] # remove NA values
  g <- arrange(g,round)
  
  reliability <- mean(t(g["completion_rate"]))
  
  return(reliability)
})

weakReliability <- memoise(calculateWeakReliability, cache=db)

setMethod(f="meanOffSlot", signature = "TestResult", definition = function(theObject) {
  return(mean(theObject@max_data$off_slot))
})

setMethod(f="sdOffSlot", signature = "TestResult", definition = function(theObject) {
  return(sd(theObject@max_data$off_slot))
})

max_count <- function(testResult) {
  testResult@
  return(5)
}
