if(!logpath) {
  logpath = "log/"
}

if(!errorpath) {
  errorpath = "error/"
}

var imports = new JavaImporter(java.io, java.lang);
with (imports) {

     // Use JavaScript object as an associative array
     outputs = new Object();
     errorOutputs = new Object();
     isFirstPrintToFile = new Object();
     
     while (true) {

      //Has the output file been created.
      if(! outputs[id.toString()]){
        
        // BTW: FileWriter seems to be buffered.
        outputs[id.toString()]= new FileWriter(logpath + "log_" + id +".txt");
        errorOutputs[id.toString()]= new FileWriter(errorpath + "log_" + id +".txt");
        outputs[id.toString()].write("[" + "\n");
        isFirstPrintToFile[id.toString()] = true;
      }
      //Write to file.
      //outputs[id.toString()].write(time + " " + msg + "\n");
       

      /* energy level readings
         0 clock time
         2 rime address
         3 sequence number
         4 accumulated CPU energy consumption
         5 accumulated Low Power Mode energy consumption
         6 accumulated transmission energy consumption
         7 accumulated listen energy consumption
         8 accumulated idle transmission energy consumption
         9 accumulated idle listen energy consumption
         10 CPU energy consumption for this cycle
         11 LPM energy consumption for this cycle
         12 transmission energy consumption for this cycle
         13 listen energy consumption for this cycle
         14 idle transmission energy consumption for this cycle
         15 idle listen energy consumption for this cycle 
       */
      var rawJson = msg.substring(msg.indexOf(' ')+1);
      var parsed = {status: "error"};
      //outputs[id.toString()].write(msg + "\n");
      //outputs[id.toString()].write(rawJson + "\n");
      try {
        parsed = JSON.parse(rawJson);
        parsed.nodeid = id
        
        var s = " ";
        
        if (isFirstPrintToFile[id.toString()]) {
          outputs[id.toString()].write(JSON.stringify(parsed) + "\n");
          isFirstPrintToFile[id.toString()] = false;
        }
        outputs[id.toString()].write("," + JSON.stringify(parsed) + "\n");
        
        outputs[id.toString()].flush();
      } catch (e) {
        errorOutputs[id.toString()].write(e + "\n");          
      }
        

      // var p = msg.trim().split(/\s+/) //.split(/\\s+/);
      // var formatted = time + " " + p[0] + " " + p[1] + " " + p[2] + " " + p[3];
      // outputs[id.toString()].write(msg + "\n");
      // outputs[id.toString()].write(formatted + "\n");
      
      // outputs[id.toString()].write(time + " " + msg + "\n");
        
      try{
        //This is the tricky part. The Script is terminated using
        // an exception. This needs to be caught.
        YIELD_THEN_WAIT_UNTIL(msg.startsWith("power"));
        //YIELD();
      } catch (e) {
        //Close files.

        for (var ids in errorOutputs){
          errorOutputs[ids].close();
        }

        for (var ids in outputs){
          outputs[ids].write("]" + "\n");
          outputs[ids].close();
        }
        //Rethrow exception again, to end the script.
        throw('test script killed');
      }
     }
    
}

