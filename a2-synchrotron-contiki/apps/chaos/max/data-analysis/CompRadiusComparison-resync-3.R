comp_1_test_suites <- c(
  "/Users/tejp/tests/evaluation-clustering-comp-1-resync-3_2018-05-22_21:52:44/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-resync-3-second-run_2018-05-23_14:14:06/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-resync-3-third-run_2018-05-24_13:33:44/"
)


comp_2_test_suites <- c(
  "/Users/tejp/tests/evaluation-clustering-comp-2-resync-3_2018-05-23_04:36:22/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-resync-3-second-run_2018-05-23_20:47:09/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-resync-3-third-run_2018-05-24_18:36:05/"
)


comp_3_test_suites <- c(
  "/Users/tejp/tests/evaluation-clustering-comp-3-resync-3_2018-05-23_07:27:05/",
  "/Users/tejp/tests/evaluation-clustering-comp-3-resync-3-second-run_2018-05-24_07:00:52/",
  "/Users/tejp/tests/evaluation-clustering-comp-3-resync-3-third-run_2018-05-24_21:44:33/"
)

a <- list(comp_1_test_suites, comp_2_test_suites, comp_3_test_suites)

helo <- lapply(a, loadTestSuitesFromPaths)


comp_1_resync_2_test_suites <- c(
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius1Resync2_2018-05-24_1/",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius1Resync2_2018_05_24_2//",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius1-Resync2_2018-05-24_3/"
)


comp_2_resync_2_test_suites <- c(
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius2Resync2_2018-05-24_1/",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius2Resync2_2018-05-24_2/",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius2-Resync2_2018-05-24_3/"
)


comp_3_resync_2_test_suites <- c(
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius3Resync2_2018-05-24_1/",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius3Resync2_2018-05-24_2/",
  "/Users/tejp/tests/CompetitionRadiusTest Resync2/CompRadius3-Resync2_2018-05-24_3/"
)

b <- list(comp_1_resync_2_test_suites, comp_2_resync_2_test_suites, comp_3_resync_2_test_suites)

helo2 <- lapply(b, loadTestSuitesFromPaths)

calculate <- function(helo, label) {
  a <- lapply(helo, function(list_of_test_suite) {
    do.call("rbind", mapply(function(ts1, ts2, ts3) {
      #browser()
      if(!(ts1@testName == ts2@testName && ts2@testName == ts3@testName)) {
        warning("Tests doesn't match, skipping result", ts1@testName, ts2@testName, ts3@testName)
        return(NA)
      }
      mean_reliability <- mean(c(reliability(ts1), reliability(ts2), reliability(ts3)))
      sd_reliability <- sd(c(reliability(ts1), reliability(ts2), reliability(ts3)))
      data.frame(testName=ts1@testName, mean_reliability=mean_reliability, sd_reliability=sd_reliability, group=label)
    }, list_of_test_suite[[1]], list_of_test_suite[[2]], list_of_test_suite[[3]], SIMPLIFY = F))

  })
  browser()
}



label_and_flattern_data <- function(helo, group_labels) {
  
  if(length(helo) != length(group_labels)) {
    stop("Requires same length on helo and labels")
  }
  a <- mapply(function(list_of_test_suite_with_same_comp_radius, group_label) {
    b <- do.call("rbind", lapply(list_of_test_suite_with_same_comp_radius, function(test_suite) {
      c <- do.call("rbind", lapply(test_suite, function(test) {
        if(is.na(test)) {
          return(NA)
        }
        data.frame(simulation_name=test@testName, reliability=reliability(test), group=group_label, spread=calculateSpread(test))
      }))
      c
    }))
    b
  }, helo, group_labels, SIMPLIFY = F)
  
  stats <- do.call("rbind", a)
  
  browser()
  stats <- stats[complete.cases(stats),]
  #order by spread
  stats$simulation_name <- factor(stats$simulation_name, levels = stats$simulation_name[order(unique(stats$spread))])
  
  
  
  p <- ggplot(stats, aes(simulation_name, reliability, color=group)) +
    geom_point() +
    theme(axis.text.x=element_text(angle=45, hjust=1), plot.margin=unit(c(1,1,1,2),"cm"))
  print(p)
  browser()
}