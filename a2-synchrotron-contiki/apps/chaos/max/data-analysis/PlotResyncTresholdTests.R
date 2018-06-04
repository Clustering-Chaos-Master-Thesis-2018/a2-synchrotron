source("PlotReliability.R")


comp_1_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-1_resync-1_2018-05-18_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-1_resync-1_2018-05-19_2/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-1_resync-1_2018-05-20_3/")
)

comp_2_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-2_resync-1_2018-05-18_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-2_resync-1_2018-05-18_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-2_resync-1_2018-05-21_3/")
)

comp_3_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-3_resync-1_2018-05-19_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-3_resync-1_2018-05-20_2/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-1/competition-radius-3_resync-1_2018-05-21_3/")
)

competition_radius_labels <- c("1  ", "2  ", "3  ")

competition_radius_test_suites <- list(comp_1_test_suites, comp_2_test_suites, comp_3_test_suites)
competition_radius_loaded <- lapply(competition_radius_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


#pdf(file = file.path(evaluation_directory, "/plot.pdf"))

run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold1.pdf", reliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)
run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold1_WeakReliability.pdf", weakReliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)

comp_1_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-1_resync-2_2018-05-24_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-1_resync-2_2018-05-24_2/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-1_resync-2_2018-05-24_3/")
)

comp_2_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-2_resync-2_2018-05-24_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-2_resync-2_2018-05-24_2/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-2_resync-2_2018-05-24_3/")
)

comp_3_test_suites <- c(
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-3_resync-2_2018-05-24_1/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-3_resync-2_2018-05-24_2/"),
  file.path(evaluation_directory, "/resync-treshold/resync-treshold-2/competition-radius-3_resync-2_2018-05-24_3/")
)

competition_radius_labels <- c("1  ", "2  ", "3  ")

competition_radius_test_suites <- list(comp_1_test_suites, comp_2_test_suites, comp_3_test_suites)
competition_radius_loaded <- lapply(competition_radius_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold2.pdf", reliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)
run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold2_WeakReliability.pdf", weakReliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)


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



competition_radius_labels <- c("1  ", "2  ", "3  ")

competition_radius_test_suites <- list(comp_1_test_suites, comp_2_test_suites, comp_3_test_suites)
competition_radius_loaded <- lapply(competition_radius_test_suites, function(test_suite_vector) lapply(test_suite_vector, loadResultsFromTestSuitePath))


run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold3.pdf", reliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)
run(competition_radius_loaded, competition_radius_labels, "ResyncThreshold3_WeakReliability.pdf", weakReliability, "Competition Radius", c(0.735, 0.80), 13, 6, 3)

#dev.off()
