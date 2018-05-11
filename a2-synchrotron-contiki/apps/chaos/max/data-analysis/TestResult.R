TestResult <- setClass(
  "TestResult",
  
  slots = c(
    testName = "character",
    simulationFile = "character",
    testDirectory = "character",
    data = "data.frame",
    max_data = "data.frame",
    location_data = "data.frame"
  )
)

setGeneric(name="calculateSpread", def=function(theObject) {standardGeneric("calculateSpread")})

setMethod(f="calculateSpread", signature = "TestResult", definition = function(theObject) {
  loc <- theObject@location_data
  dist(cbind(loc$x,loc$y))
  coord <- cbind(loc$x, loc$y)
  return(max(as.matrix(dist(coord))))
})