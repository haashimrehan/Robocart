String readString;

void ethernetSetup() {
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
}

void listenRequest() {
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string
          readString += c;
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {


          //stopping client
          client.stop();

          if (readString.indexOf("sendfrompackaging") > 0)
          {
            sendFromPackaging = true;
          }
          if (readString.indexOf("sendfromrepair") > 0)
          {
            sendFromRepair = true;
          }
          if (readString.indexOf("callfrompackaging") > 0)
          {
            callFromPackaging = true;
          }
          if (readString.indexOf("callfromrepair") > 0)
          {
            callFromRepair = true;
          }
          //clearing string for next read
          readString = "";

        }
      }
    }
  }
}
