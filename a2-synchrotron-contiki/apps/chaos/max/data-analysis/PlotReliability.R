
run <- function(test_suites, group_labels, plot_name) {
  the_plot <- plot(loaded_test_suites, group_labels)
  ggsave(file.path(evaluation_directory, plot_name), plot=the_plot)
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

plot_reliability <- function(test_suite_groups, group_labels) {
  
  if(length(test_suite_groups) != length(group_labels)) {
    stop("Requires same length on number of test suite groups and labels")
  }

  stats <- label_and_flatten_data(test_suite_groups, group_labels)
  stats <- stats[complete.cases(stats),]
  #order by spread
  stats$simulation_name <- factor(stats$simulation_name, levels = stats$simulation_name[order(unique(stats$spread))])
  
  
  
  ggplot(stats, aes(simulation_name, reliability, color=group)) +
    geom_point() +
    theme(axis.text.x=element_text(angle=45, hjust=1), plot.margin=unit(c(1,1,1,2),"cm"))
}
