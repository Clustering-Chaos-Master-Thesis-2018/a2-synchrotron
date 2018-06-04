source("PlotReliability.R")

min_node_count_0 <- c(
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_1_2018-05-26_17.06.05/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_2_2018-05-27_01.17.50/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_3_2018-05-27_09.31.22/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_4_2018-05-27_18.30.30/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_5_2018-05-28_03.34.29/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-0_6_2018-05-28_11.53.32/")
)

min_node_count_2 <- c(
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_1_2018-05-26_20.08.45/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_2_2018-05-27_04.16.54/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_3_2018-05-27_12.25.57/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_4_2018-05-27_21.40.01/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_5_2018-05-28_06.42.08/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-2_6_2018-05-28_14.47.24/")
)

min_node_count_4 <- c(
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_1_2018-05-26_22.48.45/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_2_2018-05-27_06.55.47/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_3_2018-05-27_15.22.12/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_4_2018-05-28_01.02.36/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_5_2018-05-28_09.05.34/"),
  file.path(evaluation_directory, "/min-node-count/min-node-count-4_6_2018-05-28_17.29.10/")
)



min_node_count_labels <- c("0  ", "2  ", "4  ")


min_node_count_test_suites <- list(min_node_count_0, min_node_count_2, min_node_count_4)
min_node_count_loaded <- lapply(min_node_count_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


run(min_node_count_loaded, min_node_count_labels, "MinNodeCount.pdf", reliability, "Min Node Count", c(0.005, 0.005), 12, 6.4, 2)
run(min_node_count_loaded, min_node_count_labels, "MinNodeCount_WeakReliability.pdf", weakReliability, "Min Node Count", c(0.005, 0.005), 12, 6.4, 2)
