
#source('../Main.R')
library(memoise)

load_data <- function(rows) {
  lapply(rows, loadResultFromTestInfoRow)
}

load_data_m <- memoise(load_data)

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
      tabPanel("Reliability", "This panel is intentionally left blank")
    )
  ),
  server = function(input, output) {
    output$test_suits_radio_buttons <- renderUI({
      dirs <- list.dirs("/Users/tejp/tests", recursive = F, full.names = F)
      
      test_suites <- dirs[dirs != "Simulations"]
      file_infos <- file.info(test_suites)
      file_infos <- file_infos[order(file_infos$ctime, decreasing = T),]
      
      radioButtons("test_suite_path", label = NULL, rownames(file_infos), selected = NULL)
    })
    
    output$active_test_suite <- renderText({
      dirs <- list.dirs("/Users/tejp/tests", recursive = F, full.names = T)
      ifelse(
        is.null(input$test_suite_path),
        "",
        dirs[endsWith(dirs, input$test_suite_path)]
        )
      
    })
    
    output$application_plot <- renderPlot({
      tests <- testNames(input$test_suite_path)
      if(length(tests) == 0) {
        ouput$error <- "No tests found. Are the simulation files present?"
      } else {
        rows <- lapply(tests, Curry(createTestInfoRow, input$test_suite_path))
        testResults <- load_data_m(rows)
        output$application_plot_name <- renderText(testResults[[input$num]]@testName)
        return(plotHeatmap(testResults[[input$num]], input$application_plot_range))
      }
      
      
    })
    
  }
)