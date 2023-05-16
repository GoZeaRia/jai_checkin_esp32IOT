#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#define SCAN_RX_PIN 16
#define SCAN_TX_PIN 17
// const char* QR_SCAN_ID = "device_ch2_02";
// int count = 0 ;

#define scan_switch_CI 32  // switch check in
#define scan_switch_CO 25  // switch check out
#define switch_3 26        // switch free1
#define switch_4 27        // switch free2

#define statusLed 18
#define Led_2 4             /// free status led now
#define scan_switch_out 33  // triger pin
unsigned long tlast_time_SW_Scan = 0;
unsigned long time_trigger_scan = 0;
unsigned long current_time_ready = 0;
unsigned long time_led_ready = 0;
unsigned long time_trigger_statusLed = 0;
unsigned long mill_holder;

unsigned long period_statchk = 10000; 
unsigned long last_time_statchk = 0; 
 
int SW_Scan_CI = 1;

int led_ready = 0;
int SW_Scan_CO = 1;
int toggle_sw_CO = 1;
int toggle_sw_CI = 1;
int toggle_sw_CS = 1;
int SW_Scan_CS = 1;
int SW_3  =1;
int SW_4 = 1;

int toggle_statusLED = 0;
int reset_Scanfunction = 0;
int scanhold_enable = 1;

String barcodeData = "";
String barcodeDataSave = "";



void setup() {
  pinMode(scan_switch_CI, INPUT_PULLUP);
  pinMode(scan_switch_CO, INPUT_PULLUP);
  pinMode(switch_3, INPUT_PULLUP);
  pinMode(switch_4, INPUT_PULLUP);
  pinMode(statusLed, OUTPUT);
  pinMode(Led_2, OUTPUT);


  pinMode(scan_switch_out, OUTPUT);
  digitalWrite(scan_switch_out, HIGH);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, SCAN_RX_PIN, SCAN_TX_PIN);
  // pinMode(wifi_resetpin, INPUT_PULLUP);
  // pinMode(scan_triger, OUTPUT);
  // tlast_time_SW_Scan = millis();


  WIFI_connect();
  Serial.println("\n loop started version 1.01");
}

void loop() {

  SW_Scan_CI = digitalRead(scan_switch_CI);
  SW_Scan_CO = digitalRead(scan_switch_CO);

  SW_3 = digitalRead(switch_3);
  SW_4 = digitalRead(switch_4);


  // Serial.println(SW_Scan_CO);
  if (!SW_Scan_CI) {  //   trigger switch 1
     SW_Scan_CI_param();
  } else if (!SW_Scan_CO) {
    SW_Scan_CO_param();
  }
    if (!SW_4) {  
    SW_Scan_CS_param();
      
  } 


  if (!toggle_sw_CI || !toggle_sw_CO || !toggle_sw_CS) {  // when trigger switch   enable with 0
    scan_hold();
  }

 

  if (toggle_statusLED) {
    Status_led();

  }
  ready_status();





 if( millis() - last_time_statchk > period_statchk) {
   //10sec
     last_time_statchk = millis(); 

      String statChk = "statChk";
      Http_Post_request_chk(statChk);
     
 }
 
}



// --------------------------------------------------------------- function-------------------------------------------------------------------------------------------------------------------//





void scan_hold() {
  while (reset_Scanfunction) {
    scan_reset_200ms();  // rescan every 2.8sec
    scan_20000ms();      // count times 20sec for scan function
    barcode_read();      //if read barcode  stop scan && send requestAPI
  }
}
void ready_status() {
  if (SW_Scan_CI && SW_Scan_CO && SW_Scan_CS) {

    if ((millis() - current_time_ready) >= 1000) {  //
      if (led_ready == 0) {
        digitalWrite(Led_2, 1);
        led_ready = 1;
      } else {
        digitalWrite(Led_2, 0);
        led_ready = 0;
      }
      current_time_ready = millis();
    }
  }

}
void SW_Scan_CO_param() {
  toggle_sw_CO = 0;
  toggle_sw_CI = 1;
  toggle_sw_CS = 1;
  tlast_time_SW_Scan = millis();
  time_trigger_scan = millis();
  reset_Scanfunction = 1;
  led_ready = 0;
  digitalWrite(Led_2, 0);
  digitalWrite(scan_switch_out, LOW);
   digitalWrite(statusLed, LOW);
  delay(50);
}

void SW_Scan_CI_param() {
  toggle_sw_CI = 0;
  toggle_sw_CO = 1;
  toggle_sw_CS = 1;
  tlast_time_SW_Scan = millis();
  time_trigger_scan = millis();
  reset_Scanfunction = 1;
  led_ready = 0;
  digitalWrite(Led_2, 0);
  digitalWrite(scan_switch_out, LOW);
   digitalWrite(statusLed, LOW);
  delay(50);
}

void SW_Scan_CS_param() {
  toggle_sw_CI = 1;
  toggle_sw_CO = 1;
  toggle_sw_CS = 0;
  tlast_time_SW_Scan = millis();
  time_trigger_scan = millis();
  reset_Scanfunction = 1;
  led_ready = 0;
  digitalWrite(Led_2, 0);
  digitalWrite(scan_switch_out, LOW);
   digitalWrite(statusLed, LOW);
  delay(50);
}


void barcode_read() {
  if (Serial2.available()) {
    // Read the data from the barcode reader
    barcodeData = "";
    delay(500);
    while (Serial2.available()) {
      char c = Serial2.read();
      barcodeData += c;

      // Serial.print("each c");
      // Serial.println(c);

    }
    Serial.println(barcodeData);
    if (barcodeData) {
      barcodeDataSave = barcodeData;
      Serial.print("barcodeData :");
      Serial.println(barcodeData);
      barcodeData = "";


      Http_Post_request(barcodeDataSave);  // send request && recieve status
      Reset_scanFunc();                    //reset_scanfunc

    }
  }
}



void scan_reset_200ms() {
  unsigned long current_time = millis();

  if ((current_time - tlast_time_SW_Scan) >= 3000) {
    digitalWrite(scan_switch_out, LOW);
    tlast_time_SW_Scan = current_time;
  }

  if ((current_time - tlast_time_SW_Scan) == 2800) {
    digitalWrite(scan_switch_out, HIGH);
  }

  // Save the current time and check again after 10 milliseconds have elapsed
  unsigned long new_time = millis();
  if ((new_time - current_time) >= 10) {
    current_time = new_time;
  }
}
void scan_20000ms() {

  if ((millis() - time_trigger_scan) >= 20000) {  // function millis() to turn of a  funtion scan  hold
    digitalWrite(scan_switch_out, HIGH);
    toggle_sw_CI = 1;
    toggle_sw_CO = 1;
    toggle_sw_CS =1;
    reset_Scanfunction = 0;
  }
}
void Reset_scanFunc() {
  digitalWrite(scan_switch_out, HIGH);
  toggle_sw_CI = 1;
  toggle_sw_CO = 1;
  toggle_sw_CS = 1 ;
  reset_Scanfunction = 0;
  digitalWrite(Led_2, LOW);
  
}

void WIFI_connect() {
  digitalWrite(Led_2, HIGH);
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;

  bool res;
  res = wm.autoConnect("barcode_scanner", "adminjai");  // password protected ap
  if (!res) {
    Serial.println("Failed to connect");

    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connect success  ");
    digitalWrite(Led_2, LOW);
    digitalWrite(statusLed, HIGH);
    delay(1000);
    digitalWrite(statusLed, LOW);
  }
}

void Status_led() {

  digitalWrite(Led_2, LOW);
  digitalWrite(statusLed, HIGH);

  if ((millis() - time_trigger_statusLed) >= 5000) {  // function millis()
    digitalWrite(statusLed, LOW);
    toggle_statusLED = 0;
  }
}


void Http_Post_request(String barcode_id) {
  unsigned long current_time = 0;
  int cout_time = 0;
  char data[255];
  String url = "";
  if (toggle_sw_CI == 0) {
    url = "http://34.133.167.107:1880/jai_check_in";

    sprintf(data, "barcode_ID=%s&check=check_in", barcode_id);
  } else if (toggle_sw_CO == 0) {
    url = "http://34.133.167.107:1880/jai_check_out";

    sprintf(data, "barcode_ID=%s&check=check_out", barcode_id);
  }else if (toggle_sw_CS == 0) {
    url = "http://34.133.167.107:1880/jai_check_in_Stime";

    sprintf(data, "barcode_ID=%s&check=jai_check_in_Stime", barcode_id);
  }

   Serial.print("data req:"); Serial.println(data);
  HTTPClient http;
  http.setTimeout(5000);
  http.setConnectTimeout(5000);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(data);


  int payload = httpResponseCode;
 Serial.print("res status : ");   Serial.println(payload);
   http.end();


  if (payload == 200) {
    time_trigger_statusLed = millis();
    toggle_statusLED = 1;
  }


}
void Http_Post_request_chk(String barcode_id) {
  unsigned long current_time = 0;
  int cout_time = 0;
  char data[255];
  String url = "";
 
  if(barcode_id == "statChk"){
     url = "http://34.133.167.107:1880/statusChk";
    sprintf(data, "REQ=1");
  }

  // Serial.println(data);
  HTTPClient http;
  http.setTimeout();
  http.setConnectTimeout(5000);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(data);

  int payload = httpResponseCode;
//  Serial.print("res status : ");   Serial.println(payload);
   http.end();


  

}


void Test() {
  //  if(!SW_3){
  //     Serial.println("SW_3 : 0");
  //     digitalWrite(Led_2,HIGH);
  //   } else{
  //     digitalWrite(Led_2,LOW);
  //   }
  //    if(!SW_4){
  //     Serial.println("SW_4 : 0");

  //      digitalWrite(Led_2,HIGH);
  //      delay(300);
  //      digitalWrite(Led_2,LOW);
  //   }
}
