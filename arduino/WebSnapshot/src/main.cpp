#include <WiFi.h>
#include "M5Lite.h"
#include <HTTPClient.h>

const char* wifi_ssid = "【WiFiアクセスポイントのSSID】";
const char* wifi_password = "【WiFiアクセスポイントのパスワード】";

const char* screenshot_url = "【Node.jsサーバのURL】/screenshot";
const char* target_url = "【スクリーンショット対象のWebページのURL】";

#define SCREENSHOT_INTERVAL   (10 * 60 * 1000) //スクリーンショット取得の間隔
#define DISPLAY_WIDTH   320  //LCDの横解像度
#define DISPLAY_HEIGHT  240 //LCDの縦解像度
#define SCREENSHOT_SCALE  1.0 //スクリーンショットの表示倍率
#define BUFFER_SIZE   20000 //画像受信のバッファサイズ

unsigned char buffer[BUFFER_SIZE];

void wifi_connect(const char *ssid, const char *password);
String urlencode(String str);
long doHttpGet(String url, uint8_t *p_buffer, unsigned long *p_len, unsigned short timeout);

void setup() {
  M5Lite.begin();
  Serial.begin(9600);
  Serial.println("setup");

  wifi_connect(wifi_ssid, wifi_password);
  Serial.println("connected");
}

void loop() {
  M5Lite.update();

  String url = screenshot_url;
  url += "?type=jpeg&width=" + String((int)(DISPLAY_WIDTH / SCREENSHOT_SCALE)) + "&height=" + String((int)(DISPLAY_HEIGHT / SCREENSHOT_SCALE)) + "&scale=" + String(SCREENSHOT_SCALE);
  url += "&waitfor=true";
//  url += "&wait=5000"; 
  url += "&url=" + urlencode(target_url);
  Serial.println(url);
  unsigned long length = sizeof(buffer);
  long ret = doHttpGet(url, buffer, &length, 5000);
  if( ret == 0 ){
    M5Lite.Lcd.drawJpg(buffer, length);
  }

  delay(SCREENSHOT_INTERVAL);
}

void wifi_connect(const char *ssid, const char *password){
  Serial.println("");
  Serial.print("WiFi Connenting");
  M5Lite.Lcd.println("WiFi Connectiong");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    M5Lite.Lcd.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected : ");
  Serial.println(WiFi.localIP());
  M5Lite.Lcd.println("");
  M5Lite.Lcd.print("Connected : ");
  M5Lite.Lcd.println(WiFi.localIP());
}

String urlencode(String str){
    String encodedString = "";
    char c;
    char code0;
    char code1;
//    char code2;
    for (int i = 0 ; i < str.length() ; i++){
      c = str.charAt(i);
      if (c == ' '){
        encodedString += '+';
      } else if (isalnum(c)){
        encodedString += c;
      } else{
        code1 = (c & 0xf) + '0';
        if ((c & 0xf) > 9){
            code1 = (c & 0xf) - 10 + 'A';
        }
        c = (c >> 4) & 0xf;
        code0 = c + '0';
        if (c > 9){
            code0 = c - 10 + 'A';
        }
//        code2 = '\0';
        encodedString += '%';
        encodedString += code0;
        encodedString += code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

long doHttpGet(String url, uint8_t *p_buffer, unsigned long *p_len, unsigned short timeout){
  HTTPClient http;

  http.setTimeout(timeout + 5000);

  Serial.print("[HTTP] GET begin...\n");
  // configure traged server and url
  http.begin(url);

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  unsigned long index = 0;

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // get lenght of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        Serial.printf("[HTTP] Content-Length=%d\n", len);
        if( len != -1 && len > *p_len ){
          Serial.printf("[HTTP] buffer size over\n");
          http.end();
          return -1;
        }

        // read all data from server
        while(http.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();

            if(size > 0) {
                // read up to 128 byte
                if( (index + size ) > *p_len){
                  Serial.printf("[HTTP] buffer size over\n");
                  http.end();
                  return -1;
                }
                int c = stream->readBytes(&p_buffer[index], size);

                index += c;
                if(len > 0) {
                    len -= c;
                }
            }
            delay(1);
        }
      }else{
        http.end();
        return -1;
      }
  } else {
    http.end();
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    return -1;
  }

  http.end();
  *p_len = index;

  return 0;
}
