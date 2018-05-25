
roundLogPath = logpath + "round/"
maxLogPath   = logpath + "max/"
errorLogPath = logpath + "error/"
rawLogPath   = logpath + "raw/"
powerLogPath = logpath + "power/"

function csv_format_header_round_log(raw) {
  cells = [];
  var cells_with_label = raw.split(',');
  for (var i = 0; i != cells_with_label.length; i++) {
    parts = cells_with_label[i].split(':');
    cells.push(parts[0].trim());
  }

  return cells.join(" ");
}

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

     while (true) {

      //Has the output files been created.
      if(! outputs[id.toString()]){
        outputs[id.toString()] = new Object();

        outputs[id.toString()].round = new FileWriter(roundLogPath + "log_" + id +".txt");
        outputs[id.toString()].isFirstRoundPrint = true;

        outputs[id.toString()].max = new FileWriter(maxLogPath   + "log_" + id +".txt");
        outputs[id.toString()].isFirstMaxPrint = true;

        outputs[id.toString()].error = new FileWriter(errorLogPath + "log_" + id +".txt");

        outputs[id.toString()].raw = new FileWriter(rawLogPath + "log_" + id +".txt");

        outputs[id.toString()].power = new FileWriter(powerLogPath + "log_" + id +".txt");
        outputs[id.toString()].isFirstPowerPrint = true;
      }

      outputs[id.toString()].raw.write(msg + "\n");

      var topic = msg.substring(0, msg.indexOf(' '));
      var raw = msg.substring(msg.indexOf(' ')+1);
      if(msg.startsWith("DEBUG: power:")) {
        topic = "power:";
        raw = raw.substring(raw.indexOf(' ') + 1);
      }

      if (topic == "cluster_res:") {
        if (outputs[id.toString()].isFirstMaxPrint) {
          outputs[id.toString()].isFirstMaxPrint = false;
          formatted_header = csv_format_header_round_log(raw);
          outputs[id.toString()].max.write(formatted_header + "\n");
        }

        formatted_row = csv_format_round_log(raw);
        outputs[id.toString()].max.write(formatted_row + "\n");
        outputs[id.toString()].max.flush();
      } else if (topic == "chaos_round_report:") {
        if (outputs[id.toString()].isFirstRoundPrint) {
          outputs[id.toString()].isFirstRoundPrint = false;
          formatted_header = csv_format_header_round_log(raw);
          outputs[id.toString()].round.write(formatted_header + "\n");
        }

        formatted_row = csv_format_round_log(raw);
        outputs[id.toString()].round.write(formatted_row + "\n");
        outputs[id.toString()].round.flush();
      } else if(topic == "power:") {
        if (outputs[id.toString()].isFirstPowerPrint) {
          outputs[id.toString()].isFirstPowerPrint = false;
          formatted_header = csv_format_header_round_log(raw);
          outputs[id.toString()].power.write(formatted_header + "\n");
        }

        formatted_row = csv_format_round_log(raw);
        outputs[id.toString()].power.write(formatted_row + "\n");
        outputs[id.toString()].power.flush();
      }

      try{
        //This is the tricky part. The Script is terminated using
        // an exception. This needs to be caught.
        //YIELD_THEN_WAIT_UNTIL(msg.startsWith("round:") || msg.startsWith("cluster_res:"));
        YIELD(); // Always yield to write raw message.
      } catch (e) {
        //Close files.

        for (var ids in outputs){
          outputs[ids].max.close();
          outputs[ids].round.close();
          outputs[ids].error.close();
          outputs[ids].raw.close();
          outputs[ids].power.close();
        }
        //Rethrow exception again, to end the script.
        throw('test script killed');
      }
     }

}
