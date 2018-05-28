
source('Main.R')
library(memoise)

load_data <- function(rows) {
  lapply(rows, loadResultFromTestInfoRow)
}

load_data_m <- memoise(load_data)

tests_path <- "/Users/tejp/tests/comp_rad_tests/"
partial <- function() c(list.dirs(tests_path, recursive = F, full.names = F))
full <- function() c(list.dirs(tests_path, recursive = F, full.names = T))

lookupFullNames <- function(short_names) {
  partial <- partial()
  full <- full()
  lookupTable <- data.frame(partial, full, stringsAsFactors = F)
  lookupTable[lookupTable$partial %in% short_names,]$full
}

shinyApp(
  ui = tagList(
    #shinythemes::themeSelector(),
    navbarPage(
      #theme = "cerulean",  # <--- To use a theme, uncomment this
      "Bringing Order to Chaos",
      tabPanel("Found test data",
               mainPanel(
                 h1("Collected test suits"),
                 p(verbatimTextOutput("active_test_suite")),
                 uiOutput("test_suits_radio_buttons")
               )
      ),
      tabPanel("Application Heatmap",
                 verticalLayout(
                   textOutput("application_plot_name"),
                   plotOutput("application_plot"),
                   numericInput("num", label = h3("Which plot?"), value = 1),
                   sliderInput("application_plot_range", label = h3("Rounds span"), min = 0, 
                               max = 700, value = c(1, 50))
               )
      ),
      tabPanel("Reliability", 
               verticalLayout(
                 plotOutput("reliability_plot"),
                 uiOutput("test_suites_check_boxes")
               )
      ),
      tabPanel("Latency", 
               verticalLayout(
                 plotOutput("latency_plot")
               )
      ),
      tabPanel("Energy", 
               verticalLayout(
                 plotOutput("energy_plot")
               )
      )
    )
  ),
  server = function(input, output) {
    output$test_suits_radio_buttons <- renderUI({
      dirs <- partial()
      
      test_suites <- dirs[dirs != "Simulations"]
      file_infos <- file.info(test_suites)
      file_infos <- file_infos[order(file_infos$ctime, decreasing = T),]
      
      radioButtons("test_suite_path", label = NULL, rownames(file_infos), selected = NULL)
    })
    
    output$test_suites_check_boxes <- renderUI({
      dirs <- partial()
      
      test_suites <- dirs[dirs != "Simulations"]
      file_infos <- file.info(test_suites)
      file_infos <- file_infos[order(file_infos$ctime, decreasing = T),]
      
      checkboxGroupInput("checkedTests",
                         label = h3("Test to use in the reliability plot"),
                         choices = test_suites,
                          selected = NULL)

    })
    
    output$active_test_suite <- renderText({
      dirs <- full()
      ifelse(
        is.null(input$test_suite_path),
        "",
        dirs[endsWith(dirs, input$test_suite_path)]
        )
      
    })
    
    output$application_plot <- renderPlot({
      
      abs_test_suite_path <- lookupFullNames(input$test_suite_path)
      
      tests <- testNames(abs_test_suite_path)
      if(length(tests) == 0) {
        print("NOT working on it")
        #output$error <- "No tests found. Are the simulation files present?"
      } else {
        print("Working on it")
        rows <- lapply(tests, Curry(createTestInfoRow, abs_test_suite_path))
        testResults <- load_data_m(rows)
        output$application_plot_name <- renderText(testResults[[input$num]]@testName)
        return(plotHeatmap(testResults[[input$num]], input$application_plot_range))
      }
      
      
    })
    
    
    output$reliability_plot <- renderPlot({
      
      partialNames <- input$checkedTests
      fullNames <- lookupFullNames(partialNames)
      if (length(fullNames) == 0) {
        return(plot(1,1))
      }
      
      print(input$checkedTests)
      
      testSuites <- lapply(fullNames, loadResultsFromTestSuitePath)
      stats <- do.call("rbind", mapply(function(testSuite, partialName) {
        do.call("rbind", lapply(testSuite, function(result) {
          if(is.na(result)) {
            return(NA)
          }
          data.frame(name=result@testName, reliability=reliability(result), test_suite=partialName, spread=calculateSpread(result))
        }))
      }, testSuites, partialNames, SIMPLIFY = F))
      stats$name <- factor(stats$name, levels = unique(stats$name[order(stats$spread)]))
      
      return(
        ggplot(stats, aes(name, reliability, color=test_suite)) +
          geom_point(size=5) +
          theme(
            axis.text.x=element_text(angle=45, hjust=1),
            plot.margin=unit(c(1,1,1,2),"cm"),
            text = element_text(size=20)
            ) +
          xlab("Test Name") +
          ylab("Reliability")
      )

    })
    
    
    output$latency_plot <- renderPlot({
      
      partialNames <- input$checkedTests
      fullNames <- lookupFullNames(partialNames)
      if (length(fullNames) == 0) {
        return(plot(1,1))
      }
      
      print(input$checkedTests)
      
      
      
      testSuites <- lapply(fullNames, loadResultsFromTestSuitePath)
      stats <- do.call("rbind", mapply(function(testSuite, partialName) {
        do.call("rbind", lapply(testSuite, function(result) {
          if(is.na(result)) {
            return(NA)
          }
          data.frame(name=result@testName, meanOffSlot=meanOffSlot(result), sdOffSlot=sdOffSlot(result), test_suite=partialName, spread=calculateSpread(result))
        }))
      }, testSuites, partialNames, SIMPLIFY = F))
      stats$name <- factor(stats$name, levels = unique(stats$name[order(stats$spread)]))
      
      return(
        ggplot(stats) +
          geom_pointrange(aes(name, meanOffSlot, color=test_suite, ymax=meanOffSlot+sdOffSlot, ymin=meanOffSlot-sdOffSlot), size=1, position=position_dodge(width=0.3)) +
          theme(
            axis.text.x=element_text(angle=45, hjust=1),
            plot.margin=unit(c(1,1,1,2),"cm"),
            text = element_text(size=20)
          ) +
          xlab("Test Name") +
          ylab("Latency")
      )
      
    })
    
    
    output$energy_plot <- renderPlot({
      
      partialNames <- input$checkedTests
      fullNames <- lookupFullNames(partialNames)
      if (length(fullNames) == 0) {
        return(plot(1,1))
      }
      
      print(input$checkedTests)
      
      
      
      testSuites <- lapply(fullNames, loadResultsFromTestSuitePath)
      stats <- do.call("rbind", mapply(function(testSuite, partialName) {
        do.call("rbind", lapply(testSuite, function(result) {
          if(is.na(result)) {
            return(NA)
          }
          data.frame(name=result@testName, totalPowerUsage=totalPowerUsage(result), test_suite=partialName, spread=calculateSpread(result))
        }))
      }, testSuites, partialNames, SIMPLIFY = F))
      stats$name <- factor(stats$name, levels = unique(stats$name[order(stats$spread)]))
      
      return(
        ggplot(stats) +
          geom_point(aes(name, totalPowerUsage, color=test_suite), size=1, position=position_dodge(width=0.3)) +
          theme(
            axis.text.x=element_text(angle=45, hjust=1),
            plot.margin=unit(c(1,1,1,2),"cm"),
            text = element_text(size=20)
          ) +
          xlab("Test Name") +
          ylab("Total power useage")
      )
      
    })
  }
)
