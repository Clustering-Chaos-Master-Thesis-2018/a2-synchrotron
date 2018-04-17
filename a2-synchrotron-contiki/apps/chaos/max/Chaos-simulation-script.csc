<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <project EXPORT="discard">[APPS_DIR]/cooja-gdbstub</project>
  <simulation>
    <title>My simulation</title>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      org.contikios.mrm.MRM
      <obstacles />
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #sky1</description>
      <firmware EXPORT="copy">[CONTIKI_DIR]/apps/chaos/max/max-app.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>55.287674068559554</x>
        <y>94.44626369243171</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>80.43444137387763</x>
        <y>40.031555625599765</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>96.94895757387012</x>
        <y>17.838246283308546</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>27.700159199350505</x>
        <y>25.423237694540767</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>19.690366063866026</x>
        <y>17.475976076339062</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>55.30309544025167</x>
        <y>96.44435958734984</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>58.520826367656284</x>
        <y>88.85259313190129</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>41.584800488098736</x>
        <y>64.29402818719298</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>8</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>28.129880214017298</x>
        <y>8.1673558189298</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>50.6685094627781</x>
        <y>0.24735778636463257</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspClock
        <deviation>1.0</deviation>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>280</width>
    <z>5</z>
    <height>160</height>
    <location_x>400</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <moterelations>true</moterelations>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>org.contikios.mrm.MRMVisualizerSkin</skin>
      <viewport>3.2698051774648254 0.0 0.0 3.2698051774648254 3.3060678366674803 14.918460956731415</viewport>
    </plugin_config>
    <width>400</width>
    <z>4</z>
    <height>400</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>2152</width>
    <z>1</z>
    <height>240</height>
    <location_x>404</location_x>
    <location_y>160</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <mote>2</mote>
      <mote>3</mote>
      <mote>4</mote>
      <mote>5</mote>
      <mote>6</mote>
      <mote>7</mote>
      <mote>8</mote>
      <mote>9</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <zoomfactor>500.0</zoomfactor>
    </plugin_config>
    <width>2556</width>
    <z>2</z>
    <height>375</height>
    <location_x>0</location_x>
    <location_y>978</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Notes
    <plugin_config>
      <notes>Enter notes here</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>1876</width>
    <z>3</z>
    <height>160</height>
    <location_x>680</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>


TIMEOUT(120000);
// if (!logpath) {
  logpath = "log/"
// }

powerLogPath = logpath + "power/"
roundLogPath = logpath + "round/"
errorLogPath = logpath + "error/"
rawLogPath   = logpath + "raw/"

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

        outputs[id.toString()].power = new FileWriter(powerLogPath + "log_" + id +".txt");
        outputs[id.toString()].power.write("[" + "\n");
        outputs[id.toString()].isFirstPowerPrint = true;

        outputs[id.toString()].round = new FileWriter(roundLogPath + "log_" + id +".txt");
        outputs[id.toString()].isFirstRoundPrint = true;

        outputs[id.toString()].error = new FileWriter(errorLogPath + "log_" + id +".txt");

        outputs[id.toString()].raw = new FileWriter(rawLogPath + "log_" + id +".txt");
      }

      outputs[id.toString()].raw.write(msg + "\n");

      var topic = msg.substring(0, msg.indexOf(' '));
      var raw = msg.substring(msg.indexOf(' ')+1);

      if (topic == "cluster_res:") {
        if (outputs[id.toString()].isFirstRoundPrint) {
          outputs[id.toString()].isFirstRoundPrint = false;
          formatted_header = csv_format_header_round_log(raw);
          outputs[id.toString()].round.write(formatted_header + "\n");
        }

        formatted_row = csv_format_round_log(raw);
        outputs[id.toString()].round.write(formatted_row + "\n");
        outputs[id.toString()].round.flush();
      } else if (topic == "power:") {

        try {
          var parsed = JSON.parse(raw);
          parsed.nodeid = id

          if (outputs[id.toString()].isFirstPowerPrint) {
            outputs[id.toString()].power.write(JSON.stringify(parsed) + "\n");
            outputs[id.toString()].isFirstPowerPrint = false;
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
        //YIELD_THEN_WAIT_UNTIL(msg.startsWith("power:") || msg.startsWith("cluster_res:"));
        YIELD(); // Always yield to write raw message.
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






</script>
      <active>false</active>
    </plugin_config>
    <width>1027</width>
    <z>0</z>
    <height>700</height>
    <location_x>998</location_x>
    <location_y>294</location_y>
  </plugin>
</simconf>

