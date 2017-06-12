/*
 * 結果：
 * 安定した！
 * 
 * 作業：
 * client_waitの反映
 * dtostrf解消
 */

/*

随時重複するとまずそうな宣言を整理している

#include "Arduino.h"

#define USE_SERIAL Serial

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <Hash.h>

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <ESP8266WebServer.h>
#include <FS.h>

char path[] = "/";  // add
char host[] = "________";  // add
int client_wait = 200;

int wsport = 1880;
int EXE_MODE = 1; // 1:server 0:normal
const char* espid = "ESP-00001";

// モード切り替えピン
const int MODE_PIN = 0; // GPIO0

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";

// HTML設定保存ファイル
const char* cssPath = "/framework.css";
const char* jsPath = "/framework.js";

// サーバモードでのパスワード
const String pass = "boxboxbox";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[256];
int value = 0;

///////////////////////////////////////////////////////////////////////////////////////

/**
 * センサー初期設定
 */

// #include "Arduino.h"

class Ultrasonic
{
    public:
    Ultrasonic(int pin);
    void DistanceMeasure(void);
    long microsecondsToCentimeters(void);
    long microsecondsToInches(void);
    private:
    int _pin;//pin number of Arduino that is connected with SIG pin of Ultrasonic Ranger.
    long duration;// the Pulse time received;
};
Ultrasonic::Ultrasonic(int pin)
{
    _pin = pin;
}
/*Begin the detection and get the pulse back signal*/
void Ultrasonic::DistanceMeasure(void)
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(_pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(_pin,LOW);
    pinMode(_pin,INPUT);
    duration = pulseIn(_pin,HIGH);
}
/*The measured distance from the range 0 to 400 Centimeters*/
long Ultrasonic::microsecondsToCentimeters(void)
{
    return duration/29/2;
}
/*The measured distance from the range 0 to 157 Inches*/
long Ultrasonic::microsecondsToInches(void)
{
    return duration/74/2;
}


///////////////////////////////////////////////////////////////////////////////////////

ESP8266WebServer server(80);

/**
 * WiFi設定
 */
void handleRootGet() {

    File f = SPIFFS.open(settings, "r");
    String ssid_fs = f.readStringUntil('\n');
    String pass_fs = f.readStringUntil('\n');
    String host_fs = f.readStringUntil('\n');
    String wait_fs = f.readStringUntil('\n');
    f.close();

    ssid_fs.trim();
    pass_fs.trim();
    host_fs.trim();
    wait_fs.trim();

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");

    String html = "";
    html += "<html>";
    html += "<head>";
    html += "<meta name=\"viewport\" content=\"width=360,initial-scale=1\">";
    html += "<style type='text/css'>";

    // Serial.println(html);
    server.sendContent(html);
    
    /*
    delay(200);

    //html = "";
    File fr = SPIFFS.open(cssPath, "r");
    // Serial.println();
    while (fr.available()) {
        String src = fr.readStringUntil('\n');
        // html += src;
        server.sendContent(src);
    }
    fr.close();
    //html += "</style>";
    //html += "<script>";
    Serial.println("</style>");
    Serial.println("<script>");
    */

    html = "";
    html += "*,*:after,*:before{box-sizing:inherit}html{box-sizing:border-box;font-size:62.5%}body{color:#606c76;font-family:'Roboto', 'Helvetica Neue', 'Helvetica', 'Arial', sans-serif;font-size:1.6em;font-weight:300;letter-spacing:.01em;line-height:1.6}blockquote{border-left:0.3rem solid #d1d1d1;margin-left:0;margin-right:0;padding:1rem 1.5rem}blockquote *:last-child{margin-bottom:0}.button,button,input[type='button'],input[type='reset'],input[type='submit']{background-color:#9b4dca;border:0.1rem solid #9b4dca;border-radius:.4rem;color:#fff;cursor:pointer;display:inline-block;font-size:1.1rem;font-weight:700;height:3.8rem;letter-spacing:.1rem;line-height:3.8rem;padding:0 3.0rem;text-align:center;text-decoration:none;text-transform:uppercase;white-space:nowrap}.button:focus,.button:hover,button:focus,button:hover,input[type='button']:focus,input[type='button']:hover,input[type='reset']:focus,input[type='reset']:hover,input[type='submit']:focus,input[type='submit']:hover{background-color:#606c76;border-color:#606c76;color:#fff;outline:0}.button[disabled],button[disabled],input[type='button'][disabled],input[type='reset'][disabled],input[type='submit'][disabled]{cursor:default;opacity:.5}.button[disabled]:focus,.button[disabled]:hover,button[disabled]:focus,button[disabled]:hover,input[type='button'][disabled]:focus,input[type='button'][disabled]:hover,input[type='reset'][disabled]:focus,input[type='reset'][disabled]:hover,input[type='submit'][disabled]:focus,input[type='submit'][disabled]:hover{background-color:#9b4dca;border-color:#9b4dca}.button.button-outline,button.button-outline,input[type='button'].button-outline,input[type='reset'].button-outline,input[type='submit'].button-outline{background-color:transparent;color:#9b4dca}.button.button-outline:focus,.button.button-outline:hover,button.button-outline:focus,button.button-outline:hover,input[type='button'].button-outline:focus,input[type='button'].button-outline:hover,input[type='reset'].button-outline:focus,input[type='reset'].button-outline:hover,input[type='submit'].button-outline:focus,input[type='submit'].button-outline:hover{background-color:transparent;border-color:#606c76;color:#606c76}.button.button-outline[disabled]:focus,.button.button-outline[disabled]:hover,button.button-outline[disabled]:focus,button.button-outline[disabled]:hover,input[type='button'].button-outline[disabled]:focus,input[type='button'].button-outline[disabled]:hover,input[type='reset'].button-outline[disabled]:focus,input[type='reset'].button-outline[disabled]:hover,input[type='submit'].button-outline[disabled]:focus,input[type='submit'].button-outline[disabled]:hover{border-color:inherit;color:#9b4dca}.button.button-clear,button.button-clear,input[type='button'].button-clear,input[type='reset'].button-clear,input[type='submit'].button-clear{background-color:transparent;border-color:transparent;color:#9b4dca}.button.button-clear:focus,.button.button-clear:hover,button.button-clear:focus,button.button-clear:hover,input[type='button'].button-clear:focus,input[type='button'].button-clear:hover,input[type='reset'].button-clear:focus,input[type='reset'].button-clear:hover,input[type='submit'].button-clear:focus,input[type='submit'].button-clear:hover{background-color:transparent;border-color:transparent;color:#606c76}.button.button-clear[disabled]:focus,.button.button-clear[disabled]:hover,button.button-clear[disabled]:focus,button.button-clear[disabled]:hover,input[type='button'].button-clear[disabled]:focus,input[type='button'].button-clear[disabled]:hover,input[type='reset'].button-clear[disabled]:focus,input[type='reset'].button-clear[disabled]:hover,input[type='submit'].button-clear[disabled]:focus,input[type='submit'].button-clear[disabled]:hover{color:#9b4dca}code{background:#f4f5f6;border-radius:.4rem;font-size:86%;margin:0 .2rem;padding:.2rem .5rem;white-space:nowrap}pre{background:#f4f5f6;border-left:0.3rem solid #9b4dca;overflow-y:hidden}pre>code{border-radius:0;display:block;padding:1rem 1.5rem;white-space:pre}hr{border:0;border-top:0.1rem solid #f4f5f6;margin:3.0rem 0}input[type='email'],input[type='number'],input[type='password'],input[type='search'],input[type='tel'],input[type='text'],input[type='url'],textarea,select{-webkit-appearance:none;-moz-appearance:none;appearance:none;background-color:transparent;border:0.1rem solid #d1d1d1;border-radius:.4rem;box-shadow:none;box-sizing:inherit;height:3.8rem;padding:.6rem 1.0rem;width:100%}input[type='email']:focus,input[type='number']:focus,input[type='password']:focus,input[type='search']:focus,input[type='tel']:focus,input[type='text']:focus,input[type='url']:focus,textarea:focus,select:focus{border-color:#9b4dca;outline:0}select{background:url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"14\" viewBox=\"0 0 29 14\" width=\"29\"><path fill=\"#d1d1d1\" d=\"M9.37727 3.625l5.08154 6.93523L19.54036 3.625\"/></svg>') center right no-repeat;padding-right:3.0rem}select:focus{background-image:url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"14\" viewBox=\"0 0 29 14\" width=\"29\"><path fill=\"#9b4dca\" d=\"M9.37727 3.625l5.08154 6.93523L19.54036 3.625\"/></svg>')}textarea{min-height:6.5rem}label,legend{display:block;font-size:1.6rem;font-weight:700;margin-bottom:.5rem}fieldset{border-width:0;padding:0}input[type='checkbox'],input[type='radio']{display:inline}.label-inline{display:inline-block;font-weight:normal;margin-left:.5rem}.container{margin:0 auto;max-width:112.0rem;padding:0 2.0rem;position:relative;width:100%}.row{display:flex;flex-direction:column;padding:0;width:100%}.row.row-no-padding{padding:0}.row.row-no-padding>.column{padding:0}.row.row-wrap{flex-wrap:wrap}.row.row-top{align-items:flex-start}.row.row-bottom{align-items:flex-end}.row.row-center{align-items:center}.row.row-stretch{align-items:stretch}.row.row-baseline{align-items:baseline}.row .column{display:block;flex:1 1 auto;margin-left:0;max-width:100%;width:100%}.row .column.column-offset-10{margin-left:10%}.row .column.column-offset-20{margin-left:20%}.row .column.column-offset-25{margin-left:25%}.row .column.column-offset-33,.row .column.column-offset-34{margin-left:33.3333%}.row .column.column-offset-50{margin-left:50%}.row .column.column-offset-66,.row .column.column-offset-67{margin-left:66.6666%}.row .column.column-offset-75{margin-left:75%}.row .column.column-offset-80{margin-left:80%}.row .column.column-offset-90{margin-left:90%}.row .column.column-10{flex:0 0 10%;max-width:10%}.row .column.column-20{flex:0 0 20%;max-width:20%}.row .column.column-25{flex:0 0 25%;max-width:25%}.row .column.column-33,.row .column.column-34{flex:0 0 33.3333%;max-width:33.3333%}.row .column.column-40{flex:0 0 40%;max-width:40%}.row .column.column-50{flex:0 0 50%;max-width:50%}.row .column.column-60{flex:0 0 60%;max-width:60%}.row .column.column-66,.row .column.column-67{flex:0 0 66.6666%;max-width:66.6666%}.row .column.column-75{flex:0 0 75%;max-width:75%}.row .column.column-80{flex:0 0 80%;max-width:80%}.row .column.column-90{flex:0 0 90%;max-width:90%}.row .column .column-top{align-self:flex-start}.row .column .column-bottom{align-self:flex-end}.row .column .column-center{-ms-grid-row-align:center;align-self:center}@media (min-width: 40rem){.row{flex-direction:row;margin-left:-1.0rem;width:calc(100% + 2.0rem)}.row .column{margin-bottom:inherit;padding:0 1.0rem}}a{color:#9b4dca;text-decoration:none}a:focus,a:hover{color:#606c76}dl,ol,ul{list-style:none;margin-top:0;padding-left:0}dl dl,dl ol,dl ul,ol dl,ol ol,ol ul,ul dl,ul ol,ul ul{font-size:90%;margin:1.5rem 0 1.5rem 3.0rem}ol{list-style:decimal inside}ul{list-style:circle inside}.button,button,dd,dt,li{margin-bottom:1.0rem}fieldset,input,select,textarea{margin-bottom:1.5rem}blockquote,dl,figure,form,ol,p,pre,table,ul{margin-bottom:2.5rem}table{border-spacing:0;width:100%}td,th{border-bottom:0.1rem solid #e1e1e1;padding:1.2rem 1.5rem;text-align:left}td:first-child,th:first-child{padding-left:0}td:last-child,th:last-child{padding-right:0}b,strong{font-weight:bold}p{margin-top:0}h1,h2,h3,h4,h5,h6{font-weight:300;letter-spacing:-.1rem;margin-bottom:2.0rem;margin-top:0}h1{font-size:4.6rem;line-height:1.2}h2{font-size:3.6rem;line-height:1.25}h3{font-size:2.8rem;line-height:1.3}h4{font-size:2.2rem;letter-spacing:-.08rem;line-height:1.35}h5{font-size:1.8rem;letter-spacing:-.05rem;line-height:1.5}h6{font-size:1.6rem;letter-spacing:0;line-height:1.4}img{max-width:100%}.clearfix:after{clear:both;content:' ';display:table}.float-left{float:left}.float-right{float:right}";
    server.sendContent(html);
    
    server.sendContent("</style>");

    /*
    server.sendContent("<script>");
    // html = "";
    delay(200);
    File fr2 = SPIFFS.open(jsPath, "r");
    // Serial.println();
    while (fr2.available()) {
        String src2 = fr2.readStringUntil('\n');
        src2.trim();
        Serial.println(src2);
        server.sendContent(html);
    }
    fr2.close();
    
    html = "";
    html += "</script>";
    */

    html = "";
    html += "</head>";
    html += "<body>";
    html += "<h1>WiFi Settings</h1>";
    html += "<form method='post'>";
    html += "  <input type='text' name='ssid' placeholder='ssid' value='" + ssid_fs + "'><br>";
    html += "  <input type='text' name='pass' placeholder='pass' type='password' value='" + pass_fs + "'><br>";
    html += "  <input type='text' name='host' placeholder='host' value='" + host_fs + "'><br>";
    html += "  <input type='text' name='wait' placeholder='wait' value='" + wait_fs + "'><br>";
    html += "  <input type='submit'><br>";
    html += "</form>";
    html += "</body>";
    html += "</html>";
    // Serial.println(html);
    server.sendContent(html);

}

void handleRootPost() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    String host = server.arg("host");
    String wait = server.arg("wait");

    File f = SPIFFS.open(settings, "w");
    f.println(ssid);
    f.println(pass);
    f.println(host);
    f.println(wait);
    f.close();

    String html = "";
    html += "<html>";
    html += "<body>";
    html += "<h1>WiFi Settings</h1>";
    html += ssid + "<br>";
    html += pass + "<br>";
    html += "<h1>Config</h1>";
    html += host + "<br>";
    html += wait + "<br>";
    html += "</body>";
    html += "</html>";

    server.send(200, "text/html", html);
}

/**
 * 初期化(クライアントモード)
 */
void setup_client() {

    File f = SPIFFS.open(settings, "r");
    String ssid_fs = f.readStringUntil('\n');
    String pass_fs = f.readStringUntil('\n');
    String host_fs = f.readStringUntil('\n');
    String wait_fs = f.readStringUntil('\n');
    f.close();

    ssid_fs.trim();
    pass_fs.trim();
    host_fs.trim();
    wait_fs.trim();

    Serial.println("SSID_FS: " + ssid_fs);
    Serial.println("PASS_FS: " + pass_fs);
    Serial.println("HOST_FS: " + host_fs);
    Serial.println("WAIT_FS: " + wait_fs);

    client_wait = atoi(wait_fs.c_str());
    strcpy(host,host_fs.c_str());
    // strcpy(ssid,ssid_fs.c_str());
    // strcpy(password,pass_fs.c_str());
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid_fs);
  
    WiFi.begin(ssid_fs.c_str(), pass_fs.c_str());

    Serial.print("connecting host : ");
    Serial.println(host);
    Serial.print("client_wait : ");
    Serial.println(client_wait);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.println("");
    Serial.print("My localIP address: ");
    Serial.println(WiFi.localIP());
}

/**
 * 初期化(サーバモード)
 */
void setup_server() {
    byte mac[6];
    WiFi.macAddress(mac);
    String ssid = "";
    for (int i = 0; i < 6; i++) {
        ssid += String(mac[i], HEX);
    }
    Serial.println("SSID: " + ssid);
    // Serial.println("PASS: " + pass);
    Serial.println("PASS: ************** ");

    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid.c_str(), pass.c_str());
    WiFi.mode(WIFI_AP); // 明示的な行き先指定

    server.on("/", HTTP_GET, handleRootGet);
    server.on("/", HTTP_POST, handleRootPost);
    server.begin();
    Serial.println("HTTP server started.");
}

///////////////////////////////////////////////////////////////////////////////////////

// センサー取得用初期設定

// Ultrasonic
Ultrasonic ultrasonic(14);

// ADXL345
#include <Wire.h>
#include <ADXL345.h>

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

// ITG3200
#include <Wire.h>
#include "ITG3200.h"
ITG3200 gyro;

//////////////////////////////////////////////////////////////////////////////////////

void setup() {

  // 元のプログラム用
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  /////////////////////
 
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  // BOOT 4秒以内にMODEを切り替える
  //  0 : Server
  //  1 : Client

  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }

  // ファイルシステム初期化
  bool res = SPIFFS.begin();
  if (!res) {
      Serial.println("SPIFFS.begin fail");
      return;
  } else {
      Serial.println("SPIFFS.begin ok!!");
  }

  Serial.println("[START...]");

  if (digitalRead(MODE_PIN) == 0) {
      // サーバモード初期化
      Serial.println("[MODE]SERVER");
      EXE_MODE = 1;
      setup_server();
  } else {
      // クライアントモード初期化
      Serial.println("[MODE]CLIENT");
      EXE_MODE = 0;
      setup_client();
      setup_sensor();
      // setup_wifi();
      //
      client.setServer(host, 1883);
      client.setCallback(callback);
  }
  
}

/*
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
*/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long sound_sum;
long RangeInCentimeters;
int x,y,z;
double xyz[3];
double ax,ay,az;
float gx,gy,gz;
float temp_c;

void loop() {

  if (EXE_MODE == 1) {
    // サーバモード
    server.handleClient();
  } else {
      // クライアントモード初期化
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      
      long now = millis();
      if (now - lastMsg > client_wait) {
        lastMsg = now;
        ++value;

        get_sensor();
    
        // 初期化コード
        int msg_length = strlen( msg );
        memset( msg , '\0' , msg_length );
    
        if(value == 1){
    
          strcat(msg, "{ \"message\": \"FirstAccess!!!!!!\" ");
          strcat(msg, " }");
          
          Serial.print("Publish message: ");
          Serial.println(msg);
          client.publish("outTopic", msg);
          
        } else {
          
          char sum_str[5];
          sprintf(sum_str, "%d", sound_sum);
          char range_str[5];
          sprintf(range_str, "%d", RangeInCentimeters);
          char count_int[10];
          sprintf(count_int, "%d", value);
          strcat(msg, "{");
          strcat(msg, "\"s\":");
          strcat(msg, sum_str);
          strcat(msg, ",\"r\":");
          strcat(msg, range_str);
          
          char ax_int[10];
          char ay_int[10];
          char az_int[10];
          dtostrf(ax,-8,2,ax_int);
          dtostrf(ay,-8,2,ay_int);
          dtostrf(az,-8,2,az_int);
          
          strcat(msg, ",\"ax\":");
          strcat(msg, ax_int);
          strcat(msg, ",\"ay\":");
          strcat(msg, ay_int);
          strcat(msg, ",\"az\":");
          strcat(msg, az_int);
          
          char gx_int[10];
          char gy_int[10];
          char gz_int[10];
          dtostrf(gx,-8,2,gx_int);
          dtostrf(gy,-8,2,gy_int);
          dtostrf(gz,-8,2,gz_int);

          strcat(msg, ",\"gx\":");
          strcat(msg, gx_int);
          strcat(msg, ",\"gy\":");
          strcat(msg, gy_int);
          strcat(msg, ",\"gz\":");
          strcat(msg, gz_int);
            
          Serial.print(" gx_int ");
          Serial.print( gx_int );
          Serial.print( " " );
          Serial.print( strlen(gx_int) );
          Serial.print(" gy_int ");
          Serial.print( gy_int );
          Serial.print( " " );
          Serial.print( strlen(gy_int) );
          Serial.print(" gz_int ");
          Serial.println( gz_int );
          Serial.print( " " );
          Serial.println( strlen(gz_int) );
          
          char temp_c_int[10];
          dtostrf(temp_c,-10,2,temp_c_int); //変換する値 ，変換後の総文字数，小数点以下の桁数，変換後格納する変数
          
          strcat(msg, ",\"t\":");
          strcat(msg, temp_c_int);
          
          strcat(msg, ",\"c\":");
          strcat(msg, count_int);
          strcat(msg, "}");
          
          Serial.print("Publish message: ");
          Serial.println(msg);
          
          client.publish("outTopic", msg);
        }
      }
  }
}

void setup_sensor() {

  // 加速度  /////////////////////////////////////////////////////////////
  adxl.powerOn();
  
  //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75); //62.5mg per increment
  adxl.setInactivityThreshold(75); //62.5mg per increment
  adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?
  
  //look of activity movement on this axes - 1 == on; 0 == off
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
  
  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);
  
  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);
  
  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50); //62.5mg per increment
  adxl.setTapDuration(15); //625us per increment
  adxl.setDoubleTapLatency(80); //1.25ms per increment
  adxl.setDoubleTapWindow(200); //1.25ms per increment
  
  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment
  
  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );
  
  //register interrupt actions - 1 == on; 0 == off
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  1);
  adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 1);
  
  // ジャイロ /////////////////////////////////////////////////////////////
  gyro.init();
  gyro.zeroCalibrate(200,10);//sample 200 times to calibrate and it will take 200*10ms
  
}

void get_sensor() {

  // 音圧センサー
  sound_sum = 0;
  for(int i=0; i<32; i++)
  {
      sound_sum += analogRead(A0);
  }
  sound_sum >>= 5;
  
  // 距離センサー
  RangeInCentimeters;
  ultrasonic.DistanceMeasure();// get the current signal time;
  RangeInCentimeters = ultrasonic.microsecondsToCentimeters();//convert the time to centimeters
  
  // adxl
  adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  adxl.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];
  
  // gyro
  temp_c = gyro.getTemperature();
  gyro.getAngularVelocity(&gx,&gy,&gz);
  
  
}
