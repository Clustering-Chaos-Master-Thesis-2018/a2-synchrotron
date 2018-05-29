
run <- function(test_suites, group_labels, plot_name) {
  the_plot <- plot_reliability(test_suites, group_labels)
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
  browser()
  #transform(stats)
  # Aggregate reliability for rows with same spread. Create mean and sd
  agg <- aggregate(reliability~simulation_name+group+spread, stats, function(a) c(mean=mean(a), sd=sd(a)))
  agg <- do.call(data.frame, agg)
  
  agg$simulation_name <- factor(agg$simulation_name, levels = unique(agg$simulation_name[order(agg$spread)]))
  
  stats <- stats[complete.cases(stats),]
  #order by spread
  stats$simulation_name <- factor(stats$simulation_name, levels = stats$simulation_name[order(unique(stats$spread))])
  
  ggplot(agg) +
    geom_pointrange(
      aes(
        simulation_name,
        reliability.mean,
        ymax=reliability.mean+reliability.sd,
        ymin=reliability.mean-reliability.sd,
        color=group
        ),
      position = position_dodge(width = 0.5)) +
    ylab("Reliability (Mean & Sd)") + 
    xlab("Simulation Setup") +
    labs(color="") +
    theme(axis.text.x=element_text(angle=45, hjust=1), plot.margin=unit(c(1,1,1,2),"cm"))
}
