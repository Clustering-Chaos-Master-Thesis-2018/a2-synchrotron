
plotLatency <- function(testResults) {
  
  # Extract plot data
  testNames <- sapply(testResults, function(res) {res@testName})
  testDataFields <- lapply(testResults, function(res) {res@max_data})
  testMeans <- sapply(testDataFields, function(df) {mean(df$complete_slot)})
  testSds <- sapply(testDataFields, function(df) {sd(df$complete_slot)})
  testOffSlotMeans <- sapply(testDataFields, function(df) {mean(df$off_slot)})
  testOffSlotSds <- sapply(testDataFields, function(df) {sd(df$off_slot)})
  
  size <- length(testResults)
  
  par(mar=c(9,4,4,4))
  plot(1:size, testMeans, pch=19,xlab="" ,ylab="" ,xaxt="n" ,xlim=c(0.5,size+0.5), ylim=c(0,120))#c(min(testMeans-testSds), max((testMeans+testSds))) )
  lines(rbind(1:size,1:size,NA), rbind(testMeans-testSds,testMeans+testSds, NA))
  
  points(1:size+0.1, testOffSlotMeans, pch=19,xlab="" ,ylab="" ,xaxt="n" ,xlim=c(0.5,size+0.5), ylim=c(min(testOffSlotMeans-testOffSlotSds), max((testOffSlotMeans+testOffSlotSds))) )
  lines(rbind(1:size,1:size,NA)+0.1, rbind(testOffSlotMeans-testOffSlotSds,testOffSlotMeans+testOffSlotSds, NA))
  
  axis(side=1, at=1:size, labels=FALSE)
  text(seq(0.5, size-0.5, by=1), par("usr")[3] - 8.0, labels = testNames, srt = 45, pos = 1, xpd = TRUE)
}
