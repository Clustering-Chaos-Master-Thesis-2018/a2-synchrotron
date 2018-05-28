source("Main.R")

output_path <- "~/evaluation_plots"

p <- plotReliabilityForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-1_2018-05-18_14:38:24/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-second-run_2018-05-19_17:56:34/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-third-run_2018-05-20_22:29:12")
)
ggsave(file.path(output_path,"clustering_comp_1_reliability.pdf"), plot=p)


p <- plotPowerUsageForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-1_2018-05-18_14:38:24/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-second-run_2018-05-19_17:56:34/",
  "/Users/tejp/tests/evaluation-clustering-comp-1-third-run_2018-05-20_22:29:12")
)
ggsave(file.path(output_path,"clustering_comp_1_power.pdf"), plot=p)






p <- plotReliabilityForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-2_2018-05-18_23:33:03/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-second-run_2018-05-20_01:46:22/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-third-run_2018-05-21_07:09:08/")
)
ggsave(file.path(output_path,"clustering_comp_2_reliability.pdf"), plot=p)


p <- plotPowerUsageForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-2_2018-05-18_23:33:03/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-second-run_2018-05-20_01:46:22/",
  "/Users/tejp/tests/evaluation-clustering-comp-2-third-run_2018-05-21_07:09:08/")
)
ggsave(file.path(output_path,"clustering_comp_2_power.pdf"), plot=p)




p <- plotReliabilityForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-3_2018-05-19_08:04:40//",
  "/Users/tejp/tests/evaluation-clustering-comp-3-second-run_2018-05-20_10:38:49/",
  "/Users/tejp/tests/evaluation-clustering-comp-3-third-run_2018-05-21_15:19:49/")
)
ggsave(file.path(output_path,"clustering_comp_3_reliability.pdf"), plot=p)


p <- plotPowerUsageForTestSuites(c(
  "/Users/tejp/tests/evaluation-clustering-comp-3_2018-05-19_08:04:40//",
  "/Users/tejp/tests/evaluation-clustering-comp-3-second-run_2018-05-20_10:38:49/",
  "/Users/tejp/tests/evaluation-clustering-comp-3-third-run_2018-05-21_15:19:49/")
)
ggsave(file.path(output_path,"clustering_comp_3_power.pdf"), plot=p)