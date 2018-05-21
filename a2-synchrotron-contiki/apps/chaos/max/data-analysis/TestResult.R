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