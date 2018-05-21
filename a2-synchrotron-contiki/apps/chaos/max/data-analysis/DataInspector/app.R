
#source('../Main.R')

shinyApp(
  ui = tagList(
    #shinythemes::themeSelector(),
    navbarPage(
      #theme = "cerulean",  # <--- To use a theme, uncomment this
      "shinythemes",
      tabPanel("Found test data",
               mainPanel(
                 h1("Collected test suits"),
                 p(verbatimTextOutput("active_test_suite")),
                 uiOutput("test_suits_radio_buttons")
               )
      ),
      tabPanel("Navbar 2",
                 verticalLayout(
                   plotOutput("application_plot"),
                   sliderInput("application_plot_range", label = h3("Rounds span"), min = 0, 
                               max = 700, value = c(1, 50))
               )
      ),
      tabPanel("Navbar 3", "This panel is intentionally left blank")
    )
  ),
  server = function(input, output) {
    output$test_suits_radio_buttons <- renderUI({
      dirs <- list.dirs("/Users/tejp/tests", recursive = F, full.names = T)
      
      test_suites <- dirs[dirs != "Simulations"]
      file_infos <- file.info(test_suites)
      file_infos <- file_infos[order(file_infos$ctime, decreasing = T),]
      
      radioButtons("test_suite_path", label = NULL, rownames(file_infos), selected = NULL)
    })
    
    output$active_test_suite <- renderText(
      input$test_suite_path
    )
    
    output$application_plot <- renderPlot({
      tests <- testNames(input$test_suite_path)
      if(length(tests) == 0) {
        ouput$error <- "No tests found. Are the simulation files present?"
      } else {
        rows <- lapply(tests, Curry(createTestInfoRow, input$test_suite_path))
        testResults <- lapply(rows, loadResultFromTestInfoRow)
        
        return(plotHeatmap(testResults[[1]], input$application_plot_range))
      }
      
      
    })
    
  }
)