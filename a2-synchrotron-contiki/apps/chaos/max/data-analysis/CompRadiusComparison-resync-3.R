evaluation_directory <- "/home/kerp/Exjobb/Evaluation/"
comp_1_test_suites <- c(
  file.path(evaluation_directory, "competition-radius/competition-radius-1_1_2018-05-26_17.05.14/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-1_2_2018-05-27_03.07.25/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-1_3_2018-05-27_20.42.28/")
)

comp_2_test_suites <- c(
  file.path(evaluation_directory, "competition-radius/competition-radius-2_1_2018-05-26_21.12.48/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-2_2_2018-05-27_07.23.09/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-2_3_2018-05-28_00.54.29/")
)

comp_3_test_suites <- c(
  file.path(evaluation_directory, "competition-radius/competition-radius-3_1_2018-05-27_00.05.08/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-3_2_2018-05-27_11.19.51/"),
  file.path(evaluation_directory, "competition-radius/competition-radius-3_3_2018-05-28_03.40.21/")
)

group_labels <- c("Competition Radius 1", "Competition Radius 2", "Competition Radius 3")

competition_radius_test_suites <- list(comp_1_test_suites, comp_2_test_suites, comp_3_test_suites)
loaded_test_suites <- lapply(competition_radius_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


run <- function(test_suites, group_labels) {
  plot(loaded_test_suites, group_labels)
}
  


label_and_flatten_data <- function(test_suite_groups, group_labels) {
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
  }, test_suite_groups, group_labels, SIMPLIFY = F)
  
  do.call("rbind", a)
}

plot <- function(test_suite_groups, group_labels) {
  
  if(length(test_suite_groups) != length(group_labels)) {
    stop("Requires same length on helo and labels")
  }

  stats <- label_and_flatten_data(test_suite_groups, group_labels)
  stats <- stats[complete.cases(stats),]
  #order by spread
  stats$simulation_name <- factor(stats$simulation_name, levels = stats$simulation_name[order(unique(stats$spread))])
  
  
  
  ggplot(stats, aes(simulation_name, reliability, color=group)) +
    geom_point() +
    theme(axis.text.x=element_text(angle=45, hjust=1), plot.margin=unit(c(1,1,1,2),"cm"))
}
