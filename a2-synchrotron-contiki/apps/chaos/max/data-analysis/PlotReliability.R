
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
        #Only include the network spread in the plot.
        testName <- sub(".+?-motes-(.+?)-random", "\\1", test@testName)
        data.frame(simulation_name=testName, reliability=reliability(test), group=group_label, spread=calculateSpread(test))
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
      size=1.5,
      aes(
        simulation_name,
        reliability.mean,
        ymax=reliability.mean+reliability.sd,
        ymin=reliability.mean-reliability.sd,
        color=group
      ),
      position = position_dodge(width = 0.5)) +
    ylab("Reliability (Mean & Sd)") + 
    xlab("Network Size (Meters)") +
    labs(color="Competition Radius") +
    coord_cartesian(ylim = c(0,1)) +
    coord_fixed(3) +
    theme(
      text = element_text(size=24),
      axis.text.x=element_text(angle=45, hjust=1),
      legend.justification = c(1, 1),
      legend.position = c(0.995, 0.99),
      plot.margin=grid::unit(c(0,0,0,0), "mm")
    )

}
