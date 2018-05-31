source("PlotReliability.R")

max_node_count_0 <- c(
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_1_2018-05-26_19.03.59/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_2_2018-05-27_02.29.50/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_3_2018-05-27_09.24.41/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_4_2018-05-27_20.28.24/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_5_2018-05-28_03.48.05/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-off_6_2018-05-28_11.11.07/")
)

max_node_count_5 <- c(
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_1_2018-05-26_20.48.13/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_2_2018-05-27_04.06.08/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_3_2018-05-27_11.03.59/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_4_2018-05-27_22.08.00/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_5_2018-05-28_05.29.39/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-5_6_2018-05-28_12.52.05/")
)

max_node_count_10 <- c(
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_1_2018-05-26_22.53.28/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_2_2018-05-27_06.06.21/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_3_2018-05-27_12.57.00/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_4_2018-05-28_00.20.53/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_5_2018-05-28_07.34.33/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-10_6_2018-05-28_14.49.00/")
)

max_node_count_15 <- c(
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_1_2018-05-27_00.43.35/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_2_2018-05-27_07.47.08/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_3_2018-05-27_14.39.52/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_4_2018-05-28_02.05.19/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_5_2018-05-28_09.22.47/"),
  file.path(evaluation_directory, "/max-node-count/max-node-count-15_6_2018-05-28_16.24.24/")
)


max_node_count_labels <- c("Off  ", "5  ", "10  ", "15  ")


max_node_count_test_suites <- list(max_node_count_0, max_node_count_5, max_node_count_10, max_node_count_15)
max_node_count_loaded <- lapply(max_node_count_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


run(max_node_count_loaded, max_node_count_labels, "MaxNodeCount.pdf", "Max Node Count", c(0.005, 0.005), 12, 6.4, 2)
