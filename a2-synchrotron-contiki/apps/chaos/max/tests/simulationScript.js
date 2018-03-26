
if (!logpath) {
  logpath = "log/"
}

powerLogPath = logpath + "power/"
roundLogPath = logpath + "round/"
errorLogPath = logpath + "error/"

function csv_format_round_log(raw) {
  cells = [];
  var cells_with_label = raw.split(',');
  for (var i = 0; i != cells_with_label.length; i++) {
    parts = cells_with_label[i].split(':');
    cells.push(parts[1].trim());
  }

  return cells.join(" ");
}

var imports = new JavaImporter(java.io, java.lang);
with (imports) {

     // Use JavaScript object as an associative array
     outputs = new Object();
     isFirstPrintToFile = new Object();

     while (true) {

      //Has the output files been created.
      if(! outputs[id.toString()]){
        outputs[id.toString()] = new Object();

        outputs[id.toString()].power = new FileWriter(powerLogPath + "log_" + id +".txt");
        outputs[id.toString()].round = new FileWriter(roundLogPath + "log_" + id +".txt");
        outputs[id.toString()].error = new FileWriter(errorLogPath + "log_" + id +".txt");

        outputs[id.toString()].power.write("[" + "\n");
        isFirstPrintToFile[id.toString()] = true;
      }

      var topic = msg.substring(0, msg.indexOf(' '))
      var raw = msg.substring(msg.indexOf(' ')+1);
      var parsed = {status: "error"};
      //outputs[id.toString()].write(msg + "\n");
      //outputs[id.toString()].write(raw + "\n");

      if (topic == "cluster_res:") {
        formatted_row = csv_format_round_log(raw);
        outputs[id.toString()].round.write(formatted_row + "\n");
        outputs[id.toString()].round.flush();
      } else if (topic == "power:") {
        try {
          parsed = JSON.parse(raw);
          parsed.nodeid = id

          var s = " ";

          if (isFirstPrintToFile[id.toString()]) {
            outputs[id.toString()].power.write(JSON.stringify(parsed) + "\n");
            isFirstPrintToFile[id.toString()] = false;
          }
          outputs[id.toString()].power.write("," + JSON.stringify(parsed) + "\n");

          outputs[id.toString()].power.flush();
        } catch (e) {
          outputs[id.toString()].error.write(e + "\n");
        }

      }

      try{
        //This is the tricky part. The Script is terminated using
        // an exception. This needs to be caught.
        YIELD_THEN_WAIT_UNTIL(msg.startsWith("power:") || msg.startsWith("cluster_res:"));
        //YIELD();
      } catch (e) {
        //Close files.

        for (var ids in outputs){
          outputs[ids].power.write("]" + "\n");
          outputs[ids].round.close();
          outputs[ids].power.close();
          outputs[ids].error.close();
        }
        //Rethrow exception again, to end the script.
        throw('test script killed');
      }
     }
}
