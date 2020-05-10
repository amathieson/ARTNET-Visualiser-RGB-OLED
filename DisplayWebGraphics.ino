#include <ArtnetWifi.h>
#include <qrcode.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <base64.hpp>
#include <ArduinoJson.h>




//------- CONFIG ----
#define LISTEN_ADDRESS 512

#define SCLK_PIN 18
#define MOSI_PIN 23
#define DC_PIN   17
#define CS_PIN   5
#define RST_PIN  16

const char* ssid = ".......";
const char* password = ".......";
//------- END CONFIG ----








const int numberOfChannels = 1;

ArtnetWifi artnet;
const int startUniverse = 0;

bool sendFrame = 1;
int previousDataLength = 0;


WebServer server(80);
// Screen dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.

// You can use any (4 or) 5 pins 


Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}
void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  PROGMEM String index = "<!doctype html><!-- Material Design Lite Copyright 2015 Google Inc. All rights reserved. Licensed under the Apache License, Version 2.0 (the \"License\"); you may not use this file except in compliance with the License. You may obtain a copy of the License at https://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License--><html lang=\"en\"><head> <meta charset=\"utf-8\"> <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> <meta name=\"description\" content=\"An editor for ESP32 connected RGB OLED modules using the Adafruit GFX library.\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, minimum-scale=1.0\"> <title>ESP OLED EDITOR</title> <meta name=\"mobile-web-app-capable\" content=\"yes\"> <link rel=\"icon\" sizes=\"192x192\" href=\"data:image/png;base64,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\"> <meta name=\"apple-mobile-web-app-capable\" content=\"yes\"> <meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\"> <meta name=\"apple-mobile-web-app-title\" content=\"Material Design Lite\"> <link rel=\"apple-touch-icon-precomposed\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAHgAAAB4CAMAAAAOusbgAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAMAUExURagtLas0NK05ObBAQLFDQ7FERLJFRbJGRrJHR7NISLRKSrRLS7RMTLVNTbVOTrVPT7ZQULdRUbdSUrdTU7dUVLhVVbhWVrlXV7lYWLpZWbpaWrpbW7tcXLtdXbxeXrxfX7xgYL1hYb1iYr5jY8+NjdSYmNefn9ihodijo9qnp9uoqNupqdyrq92srN2trd6vr96wsN6xsd+zs+Czs+K6uvXo6Pbq6vbr6/fs7Pft7fju7vjv7/jw8Pnx8fny8vrz8/r09Pv19fv29vz39/z4+Pz5+f36+v37+/7+/v///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAK70hasAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAKaSURBVGhD7dlXc9wgGIVhUhzb6c1Or07vvdmJ0xP9/z8UlXdXEiCki+9oJzM8N7sLDGdYhEC7LsuyLMsyuTd7u894O6eNorbOx9lcbnKLYo2CmZwktrRJ0SwOElqbccwXiMRsY75C4NIGFWJHieuYJfksYT0zfNs3ifLIr7ADBAXkYz5EUEA+5sMEBeRjXiMoQL3OJkG+t9Rb2+bVuSN/ifJQbWyneM0759ZJ8lBr60bZ8SveO3e8Ser7Q6WpE3XXL/kUn+e71Fm6Tt/tWecYJa2v1Fjaoe+ieEFJuFd8otxSd/99TplzpyhpfKbU0nn6bjyitJz3zqr6RZml2/S98ITy7pjfU2Ip3AefUuPcGUoU96xmHfU9ps650/VnxfV8re7Z94Ba586VnxTXVbuO+rpjfsc7S5fICbXJW7xaip7rcI82Cv466mvn2dotEoY8pJ21/h0x5j4tbQXPKRF3aGtpaB11Ke6TU8ar2AfH57coftPW0tj1XFGMN71+G4r5Td2vFvZpa2n4/tz6RltLq1pH8f237yNtLcXOGz7FOhr4naFnl7aWVrWO+ufnOMX5yvu9Luo7bS1NWUcfaGtp8TyYosidso5+0tZS9bw/Zo+2lqbMr+Lcvk3fKYrxbtF3imJ+V7WOplxXq1pHivvVlP13zuffLsV+dJG+UxT7b/U7whjFeWPKvq84X0055yjmt/O/5CDF+fkqfaconhdWtY6W/zsnKJ4Hm18C0xTryO3TeYJivFOCFfNbGg1WXM+VsWDF78C1kWDR91xKByvuz0gGS9YRUsGK/XcpEayb38pw8BdaiAwGK/bBrqFg7fdcGggWriPEgxX/D3qiwYrnQV8sWD6/lUiw4jklFAb/oCbLsizLsuw/5tw/8prQmVAa56AAAAAASUVORK5CYII=\"> <meta name=\"msapplication-TileImage\" content=\"images/touch/ms-touch-icon-144x144-precomposed.png\"> <meta name=\"msapplication-TileColor\" content=\"#3372DF\"> <link rel=\"shortcut icon\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAIxSURBVFhHxZfLMgNBFIYTxJ1iqbBwqVIWyevwiLwOG1bsrCiXuAv+70yfqRGTTE9mwld1nJk25v/7dOucNBvl8OfJ2b/9CgGeo4g1MKGYPGq335LbwRyenk4r9RSfNlBAkQGEWxJ+SW7jkZE5JQwPNTLIAONTMTMuIlTkQ5G7NMywH8Rn6hCH8J4ZRe5k+w24+HNyWw/hfbOKXyb6B6b18Gu4rh0tByaoSLocWQOs+Xu4Hhsy0VJiTxi+BBhh1/4F84p04m6A2d+H65E5ODkJV4ORzp3SVHKXGMANa1MJF48xIai2VYEfnHDpmlQhK37c6YSrfLQXqEKPCkzayIgg6sIuWiQeMF0MpOtRluyM+01EgG5z5ApkxUfEJo4BohR54iVm7pjuf4mnYCC6gahZ3HQxQPNQSN0zF6aLgcIzYAziYD2CVUCHwroNRVJVXHobSjZxX4InbmKoYeaAnrVq/qlEx7KmI/kiuR0fmv220pXC+kwqAPQBlT8NI0EnbffcAOXohrUZG3r/plJXkXbKbgCowm0oUe3ovTtKt4ofzW7WAAcDzeO1Ht61kZoI77tWsPl+HHxpa5QBU4uKVW3KSxupgMS3lG4Uj4pfh16eAcAE/xkrimUZOWOwDBLeV6L9Itjxud+QBhkAfsdHJu3TkmJBRs6VhyLhPSVK/RCynXiKXIYZcNwIFaF35KsWQR/B73g5pWUT852C2ZKHCjsxBhyeJVgeF3cDboIy+30EjcY32BazdELfO88AAAAASUVORK5CYII=\"><!-- <link rel=\"canonical\" href=\"http://www.example.com/\"> --> <link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css?family=Roboto:regular,bold,italic,thin,light,bolditalic,black,medium&amp;lang=en\"> <link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\"> <link rel=\"stylesheet\" href=\"https://code.getmdl.io/1.3.0/material.cyan-light_blue.min.css\"> <script src=\"https://code.jquery.com/jquery-3.5.0.min.js\" crossorigin=\"anonymous\"></script> <style>#view-source{position: fixed; display: block; right: 0; bottom: 0; margin-right: 40px; margin-bottom: 40px; z-index: 900;}/** * Copyright 2015 Google Inc. All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the \"License\"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * * http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an \"AS IS\" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */ html, body{font-family: 'Roboto', 'Helvetica', sans-serif;}.demo-avatar{width: 48px; height: 48px; border-radius: 24px;}.demo-layout .mdl-layout__header .mdl-layout__drawer-button{color: rgba(0, 0, 0, 0.54);}.mdl-layout__drawer .avatar{margin-bottom: 16px;}.drawer{border: none;}/* iOS Safari specific workaround */ .drawer .mdl-menu__container{z-index: -1;}.drawer .navigation{z-index: -2;}/* END iOS Safari specific workaround */ .drawer .mdl-menu .mdl-menu__item{display: -webkit-flex; display: -ms-flexbox; display: flex; -webkit-align-items: center; -ms-flex-align: center; align-items: center;}.drawer-header{box-sizing: border-box; display: -webkit-flex; display: -ms-flexbox; display: flex; -webkit-flex-direction: column; -ms-flex-direction: column; flex-direction: column; -webkit-justify-content: flex-end; -ms-flex-pack: end; justify-content: flex-end; padding: 16px; height: 151px;}.demo-layout .navigation .mdl-navigation__link{display: -webkit-flex !important; display: -ms-flexbox !important; display: flex !important; -webkit-flex-direction: row; -ms-flex-direction: row; flex-direction: row; -webkit-align-items: center; -ms-flex-align: center; align-items: center; color: rgba(255, 255, 255, 0.56); font-weight: 500;}.demo-layout .navigation .mdl-navigation__link:hover{background-color: #00BCD4; color: #37474F;}.navigation .mdl-navigation__link .material-icons{font-size: 24px; color: rgba(255, 255, 255, 0.56); margin-right: 32px;}.content{max-width: 1080px;}.demo-cards{-webkit-align-items: flex-start; -ms-flex-align: start; align-items: flex-start; -webkit-align-content: flex-start; -ms-flex-line-pack: start; align-content: flex-start;}.demo-cards .demo-separator{height: 32px;}.demo-cards .mdl-card__title.mdl-card__title{color: white; font-size: 24px; font-weight: 400;}.demo-cards ul{padding: 0;}.demo-cards h3{font-size: 1em;}.demo-updates .mdl-card__title{min-height: 200px; background-image: url('images/dog.png'); background-position: 90% 100%; background-repeat: no-repeat;}.demo-cards .mdl-card__actions a{color: #00BCD4; text-decoration: none;}.demo-options h3{margin: 0;}.demo-options .mdl-checkbox__box-outline{border-color: rgba(255, 255, 255, 0.89);}.demo-options ul{margin: 0; list-style-type: none;}.demo-options li{margin: 4px 0;}.demo-options .material-icons{color: rgba(255, 255, 255, 0.89);}.demo-options .mdl-card__actions{height: 64px; display: -webkit-flex; display: -ms-flexbox; display: flex; box-sizing: border-box; -webkit-align-items: center; -ms-flex-align: center; align-items: center;}#canvas{margin: auto; width: 128px; height: 128px;}</style></head><body><div class=\"layout mdl-layout mdl-js-layout mdl-layout--fixed-drawer mdl-layout--fixed-header\"> <header class=\"header mdl-layout__header mdl-color--grey-100 mdl-color-text--grey-600\"> <div class=\"mdl-layout__header-row\"> <span class=\"mdl-layout-title\">ESP OLED EDITOR</span> <div class=\"mdl-layout-spacer\"></div><button onclick=\"downloadJSON(event)\" class=\"mdl-button mdl-js-button mdl-js-ripple-effect mdl-button--icon\"> <i class=\"material-icons\">get_app</i> </button> <button onclick=\"sendToDevice(event)\" class=\"mdl-button mdl-js-button mdl-js-ripple-effect mdl-button--icon\"> <i class=\"material-icons\">system_update</i> </button> </div></header> <div class=\"drawer mdl-layout__drawer mdl-color--blue-grey-900 mdl-color-text--blue-grey-50\"> <header class=\"drawer-header\"> <span>Tools</span> </header> <nav class=\"navigation mdl-navigation mdl-color--blue-grey-800\"> <a class=\"mdl-navigation__link\" href=\"#\" onclick=\"modeSelect(event, 'RECT')\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">crop_portrait</i>Rectangle</a> <a class=\"mdl-navigation__link\" href=\"#\" onclick=\"modeSelect(event, 'CIRC')\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">radio_button_unchecked</i>Circle</a> <a class=\"mdl-navigation__link\" href=\"#\" onclick=\"modeSelect(event, 'PIXL')\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">create</i>Pixel</a> <a class=\"mdl-navigation__link\" href=\"#\" onclick=\"modeSelect(event, 'TEXT')\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">format_size</i>Text</a> <div class=\"mdl-layout-spacer\"></div><a class=\"mdl-navigation__link\" href=\"#\" onclick=\"downloadJSON(event)\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">get_app</i>Download JSON</a> <a class=\"mdl-navigation__link\" href=\"#\" onclick=\"sendToDevice(event)\"><i class=\"mdl-color-text--blue-grey-400 material-icons\" role=\"presentation\">system_update</i>Send To Device</a> </nav> </div><main class=\"mdl-layout__content mdl-color--grey-100\"> <div class=\"mdl-grid content\"> <div class=\"mdl-color--white mdl-shadow--2dp mdl-cell mdl-cell--12-col mdl-grid\"> <canvas id=\"canvas\"></canvas> </div><div class=\"mdl-color--white mdl-shadow--2dp mdl-cell mdl-cell--12-col mdl-grid\"> <form action=\"#\" style=\"width: 100%\"> <div style=\"width: 100%\" class=\"mdl-textfield mdl-js-textfield\"> <textarea id=\"JSON\" class=\"mdl-textfield__input\" rows=\"3\" cols=\"25\"></textarea> <label class=\"mdl-textfield__label\" for=\"JSON\">JSON Layout Description</label> </div></form> <button onclick=\"elements=JSON.parse(document.getElementById('JSON').value)\" class=\"mdl-button mdl-js-ripple-effect mdl-js-button mdl-button--raised mdl-button--colored\"> Update From JSON </button> </div><div class=\"mdl-color--white mdl-shadow--2dp mdl-cell mdl-cell--12-col mdl-grid\"> <form action=\"#\"> <div class=\"mdl-textfield mdl-js-textfield mdl-textfield--floating-label\"> <input id=\"IP\" class=\"mdl-textfield__input\" value=\"" + IpAddress2String(WiFi.localIP()) + "\" type=\"text\" pattern=\"^((\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$\"> <label class=\"mdl-textfield__label\" for=\"IP\">IP ADDRESS</label> </div></form> <input type=\"color\" id=\"c\" tabindex=-1 class=\"hidden\"><br></div></div></main></div><script src=\"https://code.getmdl.io/1.3.0/material.min.js\"></script><script>var ctx=document.getElementById(\"canvas\").getContext(\"2d\"),canvas=document.getElementById(\"canvas\"),Globalmode=\"RECT\",elements={elements:[],elementCount:0};function hexToRgb(e){var t=/^#?([a-f\\d]{2})([a-f\\d]{2})([a-f\\d]{2})$/i.exec(e);return t?{r:parseInt(t[1],16),g:parseInt(t[2],16),b:parseInt(t[3],16)}:null}function hex24Tohex16(e){var t=hexToRgb(e),n=Math.round(t.r/255*31).toString(2).padStart(5,\"0\"),l=Math.round(t.g/255*63).toString(2).padStart(6,\"0\"),o=Math.round(t.b/255*31).toString(2).padStart(5,\"0\");return binaryToHex(n.toString()+l.toString()+o.toString())}function binaryToHex(e){var t,n,l,o,a=\"\";for(t=e.length-1;t>=3;t-=4){for(l=e.substr(t+1-4,4),o=0,n=0;n<4;n+=1){if(\"0\"!==l[n]&&\"1\"!==l[n])return{valid:!1};o=2*o+parseInt(l[n],10)}a=o>=10?String.fromCharCode(o-10+\"A\".charCodeAt(0))+a:String(o)+a}if(t>=0){for(o=0,n=0;n<=t;n+=1){if(\"0\"!==e[n]&&\"1\"!==e[n])return{valid:!1};o=2*o+parseInt(e[n],10)}a=String(o)+a}return a}function modeSelect(e,t){e.preventDefault(),Globalmode=t}function sendToDevice(e){e.preventDefault(),$.ajax({type:\"POST\",url:\"http://\"+document.getElementById(\"IP\").value+\"/post\",data:{JSON:JSON.stringify(elements)},success:function(e){console.log(e)}})}function sendReq(e){var t=elements.elements[e];$.get(\"http://\"+document.getElementById(\"IP\").value+\"/?type=\"+t.type+\"&x1=\"+t.X1+\"&y1=\"+t.Y1+\"&x2=\"+t.X2+\"&y2=\"+t.Y2+\"&color=\"+t.color,function(t){console.log(t),e<elements.elements.length&&sendReq(e+1)})}ctx.fillStyle=\"#000000\",ctx.scale(1,1),canvas.width=128,canvas.height=128,canvas.style.width=\"128px\",canvas.style.height=\"128px\",ctx.fillRect(0,0,canvas.width,canvas.height);var globalX=0,globalY=0,GlobalClickState=!1;function getMousePos(e,t){var n=e.getBoundingClientRect();return{x:t.clientX-n.left,y:t.clientY-n.top}}function downloadObjectAsJson(e,t){var n=\"data:text/json;charset=utf-8,\"+encodeURIComponent(JSON.stringify(e)),l=document.createElement(\"a\");l.setAttribute(\"href\",n),l.setAttribute(\"download\",t+\".json\"),document.body.appendChild(l),l.click(),l.remove()}function downloadJSON(e){e.preventDefault(),downloadObjectAsJson(elements,\"elements\")}canvas.onclick=function(e){var t=getMousePos(canvas,e);if(GlobalClickState)return GlobalClickState=!1,document.getElementById(\"c\").click(),void(document.getElementById(\"c\").oninput=function(e){switch(Globalmode){case\"RECT\":var n=document.getElementById(\"c\").value,l=globalX,o=globalX,a=t.x,c=t.y,s=hex24Tohex16(n);ctx.fillStyle=n,ctx.fillRect(l,o,a-l,c-o),elements.elements.push({type:Globalmode,color:parseInt(s,16),X1:l,Y1:o,X2:a,Y2:c}),elements.elementCount=elements.elements.length;break;case\"CIRC\":n=document.getElementById(\"c\").value;var i=globalX,m=globalY,r=Math.sqrt((t.x-globalX)*(t.x-globalX)+(t.y-globalY)*(t.y-globalY));s=hex24Tohex16(n);ctx.beginPath(),ctx.arc(i,m,r,0,2*Math.PI,!1),ctx.fillStyle=n,ctx.fill(),elements.elements.push({type:Globalmode,color:parseInt(s,16),X:i,Y:m,R:r}),elements.elementCount=elements.elements.length}document.getElementById(\"JSON\").value=JSON.stringify(elements,void 0,2),$.ajax({type:\"POST\",url:\"http://\"+document.getElementById(\"IP\").value+\"/post\",data:{JSON:JSON.stringify(elements)},success:function(e){console.log(e)}})});if(GlobalClickState);else switch(Globalmode){case\"TEXT\":document.getElementById(\"c\").click(),document.getElementById(\"c\").oninput=function(e){var n=document.getElementById(\"c\").value,l=t.x,o=t.y,a=prompt(\"TEXT?\"),c=hex24Tohex16(n);ctx.fillStyle=n,ctx.font=\"9px Arial\",ctx.fillText(a,l,o),elements.elements.push({type:Globalmode,color:parseInt(c,16),X:l,Y:o,text:a}),elements.elementCount=elements.elements.length,document.getElementById(\"JSON\").value=JSON.stringify(elements,void 0,2),$.ajax({type:\"POST\",url:\"http://\"+document.getElementById(\"IP\").value+\"/post\",data:{JSON:JSON.stringify(elements)},success:function(e){console.log(e)}})};break;case\"PIXL\":document.getElementById(\"c\").click(),document.getElementById(\"c\").oninput=function(e){var n=document.getElementById(\"c\").value,l=t.x,o=t.y,a=hex24Tohex16(n);ctx.fillStyle=n,ctx.fillRect(l,o,1,1),elements.elements.push({type:Globalmode,color:parseInt(a,16),X:l,Y:o}),elements.elementCount=elements.elements.length,document.getElementById(\"JSON\").value=JSON.stringify(elements,void 0,2),$.ajax({type:\"POST\",url:\"http://\"+document.getElementById(\"IP\").value+\"/post\",data:{JSON:JSON.stringify(elements)},success:function(e){console.log(e)}})};break;case\"RECT\":case\"CIRC\":globalX=t.x,globalY=t.y,GlobalClickState=!0,ctx.fillStyle=\"#FFFFFF\",ctx.fillRect(globalX,globalY,1,1)}};</script></body></html>";
  server.send(200, "text/html", index);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handlePost() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String message = "Number of args received:";
  message += server.args();
  for (int i = 0; i < server.args(); i++) {
  
    message += "Arg nº" + (String)i + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
  
  } 

  if (server.hasArg("JSON")){
    char elements[1024];
    server.arg("JSON").toCharArray(elements, 1024);
    
    unsigned char elements_decoded[decode_base64_length((unsigned char *)elements)];
    decode_base64((unsigned char *)elements,elements_decoded);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, server.arg("JSON"));
    Serial.println(server.arg("JSON"));
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    drawFromJSON(doc);
  }

  server.send(200, "text/plain", message);


}

void drawFromJSON(DynamicJsonDocument doc) {
  Serial.println((int)doc["elementCount"]);
    Serial.println((int)(sizeof doc["elements"])/(sizeof doc["elements"][0]));
    int x1 = (int)doc["elements"][0]["X1"];
    int x2 = (int)doc["elements"][0]["X2"];
    int y1 = (int)doc["elements"][0]["Y1"];
    int y2 = (int)doc["elements"][0]["Y2"];
    int color = (int)doc["elements"][0]["color"];
    Serial.print (x1);
    Serial.print (",");
    Serial.print (y1);
    Serial.print (",");
    Serial.print (x2);
    Serial.print (",");
    Serial.print (y2);
    Serial.print (",");
    Serial.print (color);
    
    

    tft.fillRect(0, 0, 128, 128, 0x0000);
    for (int el = 0; el < (int)doc["elementCount"]; el++) {
      if (doc["elements"][el]["type"] == "RECT") {
        int x1 = (int)doc["elements"][el]["X1"];
        int x2 = (int)doc["elements"][el]["X2"];
        int y1 = (int)doc["elements"][el]["Y1"];
        int y2 = (int)doc["elements"][el]["Y2"];
        int color = (int)doc["elements"][el]["color"];
        Serial.println ("EL");
        tft.fillRect(x1, y1, x2-x1, y2-y1, color);
      }
      if (doc["elements"][el]["type"] == "CIRC") {
        int x = (int)doc["elements"][el]["X"];
        int y = (int)doc["elements"][el]["Y"];
        int r = (int)doc["elements"][el]["R"];
        int color = (int)doc["elements"][el]["color"];
        Serial.println ("EL");
        tft.fillCircle(x, y, r, color);
      }
      if (doc["elements"][el]["type"] == "TEXT") {
        int x = (int)doc["elements"][el]["X"];
        int y = (int)doc["elements"][el]["Y"];
        int color = (int)doc["elements"][el]["color"];
        String text = doc["elements"][el]["text"];
        Serial.println ("EL");
        tft.setTextColor(color);
        tft.setFont(&FreeSans9pt7b);
        tft.setCursor(x,y);
        tft.print(text);
      }
      if (doc["elements"][el]["type"] == "PIXL") {
        int x = (int)doc["elements"][el]["X"];
        int y = (int)doc["elements"][el]["Y"];
        int color = (int)doc["elements"][el]["color"];
        Serial.println ("EL");
        tft.drawPixel(x,y,color);
      }
    }
    int16_t  TxTx1, TxTy1;
    uint16_t w, h;
    char string[50];
    
    sprintf(string, "ADDR: %03d", (LISTEN_ADDRESS));
    tft.setTextColor(64512);
    tft.setFont(&FreeSans9pt7b);
    tft.getTextBounds(string, 1, 32, &TxTx1, &TxTy1, &w, &h);
    tft.setCursor((64-(w/2)),32);
    tft.fillRect((64-(w/2)), TxTy1, w, h, 0);
    tft.print(string);
    
/*int rowCount = 0;
int colCount = 0;
    for (int y = 24; y < 105; y++){
      if (y%4 == 0) {
        tft.fillRect(0,y,125,1,48631);
        rowCount++;
      }
    }
    
    for (int x = 0; x < 126; x++){
      if (x%5 == 0) {
        tft.fillRect(x,24,1,80,48631); 
        colCount++;
      }
    }
    tft.fillRect(12*5+1,101,13*5,4,0);
    Serial.println("DMX Values: ");
    Serial.print(rowCount*colCount);*/
}


void setup() {
  // put your setup code here, to run once:
  tft.begin();
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  tft.fillRect(0, 0, 128, 128, 0x0000);
  tft.fillScreen(0xffff);
  tft.setTextColor(0x0000);
  tft.setFont();
  tft.setCursor(0,121);
  tft.print(WiFi.localIP());
  tft.print(" on : ");
  tft.print(ssid);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  server.handleClient();
  server.on("/post",HTTP_POST,handlePost);

    // The structure to manage the QR code
  QRCode qrcode;
  
  // Allocate a chunk of memory to store the QR code
  uint8_t qrcodeBytes[qrcode_getBufferSize(3)];
  
  qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, ("http://" + IpAddress2String(WiFi.localIP()) + "/").c_str());
  for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            tft.fillRect((x*4)+2,(y*4)+2,4,4,qrcode_getModule(&qrcode, x, y) ? 0x0000: 0xffff);
        }
    }
  artnet.begin();

  artnet.setArtDmxCallback(onDmxFrame);

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, "{\r\n  \"elements\": [\r\n    {\r\n      \"type\": \"RECT\",\r\n      \"color\": 64512,\r\n      \"X1\": 0,\r\n      \"Y1\": 0,\r\n      \"X2\": 128,\r\n      \"Y2\": 16\r\n    },\r\n    {\r\n      \"type\": \"RECT\",\r\n      \"color\": 0,\r\n      \"X1\": 1,\r\n      \"Y1\": 1,\r\n      \"X2\": 127,\r\n      \"Y2\": 15\r\n    },\r\n    {\r\n      \"type\": \"TEXT\",\r\n      \"color\": 64512,\r\n      \"X\": 3,\r\n      \"Y\": 13,\r\n      \"text\": \"   ArtNet  U: 1\"\r\n    },\r\n    {\r\n      \"type\": \"RECT\",\r\n      \"color\": 2016,\r\n      \"X1\": 0,\r\n      \"Y1\": 112,\r\n      \"X2\": 128,\r\n      \"Y2\": 128\r\n    },\r\n    {\r\n      \"type\": \"TEXT\",\r\n      \"color\": 65535,\r\n      \"X\": 3,\r\n      \"Y\": 125,\r\n      \"text\": \"     " + IpAddress2String(WiFi.localIP()) + "\"\r\n    },\r\n    {\r\n      \"type\": \"PIXL\",\r\n      \"color\": 2016,\r\n      \"X\": 85,\r\n      \"Y\": 118\r\n    },\r\n    {\r\n      \"type\": \"PIXL\",\r\n      \"color\": 63488,\r\n      \"X\": 106,\r\n      \"Y\": 118\r\n    }\r\n  ],\r\n  \"elementCount\": 6\r\n}");
  Serial.println(server.arg("JSON"));
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  drawFromJSON(doc);
}
int lastVal = 0;
char str[10];
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  sendFrame = 1;
  // set brightness of the whole strip 
  if (universe == 1 && lastVal !=data[LISTEN_ADDRESS-1])
  {
    lastVal = data[LISTEN_ADDRESS-1];
    int16_t  TxTx1, TxTy1;
    uint16_t w, h;
    tft.setTextColor(0);
    tft.setFont(&FreeSansBold24pt7b);
    
    tft.setTextWrap(false);
    tft.getTextBounds(str, 1, 90, &TxTx1, &TxTy1, &w, &h);
    tft.setCursor((64-(w/2)),90);
    tft.print(str);
    
    tft.setTextColor(65535);
    sprintf(str, "%03d", (data[LISTEN_ADDRESS-1]));
    tft.setTextWrap(false);
    tft.getTextBounds(str, 1, 90, &TxTx1, &TxTy1, &w, &h);
    tft.setCursor((64-(w/2)),90);
    tft.print(str);
  }
  previousDataLength = length;     

  
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  artnet.read();

}
