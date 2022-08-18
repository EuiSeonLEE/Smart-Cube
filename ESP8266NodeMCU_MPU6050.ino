#include <ESP8266WiFi.h> // ESP 8266 와이파이 라이브러리
#include <ESP8266HTTPClient.h> // HTTP 클라이언트
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "time.h"

HTTPClient myClient;  // 날씨
HTTPClient myClient2;
HTTPClient myClient3;
HTTPClient myClient4;
DynamicJsonDocument test1(1024);
DynamicJsonDocument test2(4096);
WiFiClient wificlient; // 와이파이 클라이언트 객체
SoftwareSerial nodemcu(13, 12);

// 시계 관련 변수
const char* ntpServer = "pool.ntp.org"; // NTP 서버
uint8_t timeZone = 9; // 한국 타임존 설정
uint8_t summerTime = 0; // 3600 // 썸머타임 시간

time_t now; // 현재 시간 변수
time_t prevEpoch; // 이전 UTC 시간 변수
struct tm * timeinfo; // 로컬 시간 반영용 포인터 변수 선언

int s_hh = 12;
int s_mm = 59;
uint8_t s_ss = 45;
uint16_t s_yy = 2017;
uint8_t s_MM = 11;
uint8_t s_dd = 19;

int month_days;
//가공 데이터
//날씨
const char* city;
float temp;
float humidity;
float wind;
float wind_deg;
const char* detailwhe;
//환율
String rateapi = "http://ecos.bok.or.kr/api/StatisticSearch/CT2Y3BZYSLV8A17D8ELY/xml/kr/1/10/036Y001/DD/";
String rateadr1 = "";
String rateadr2 = "";
String rateadr3 = "";
String rateadr53 = "";
char rate[4][40] = {0,};

String DOLLAR = "";
String YEN = "";
String EURO = "";
String YUAN = "";
String ratemoney = "";

//코로나
String covidapi = "http://openapi.data.go.kr/openapi/service/rest/Covid19/getCovid19InfStateJson?serviceKey=3%2F4V%2B1ncUo7UPucSMozyvVmsx47Iu%2B3gPXTQQTRD7HPfgAgwysao0LcfwPWBpqgwfEN0Z0viFVO2BpDFPrd00A%3D%3D&pageNo=1&numOfRows=5&startCreateDt=";
String covidadr = "";

char covid_day[9][60] = {0,}; // 일주일 코로나 정보 2차원 배열
int covid_decide[7] = {0,}; // 일일 확진자 변수
String covid_day_decide = "(3"; // 일주일 확진자 수 합산

//미세먼지
String airapi = "http://apis.data.go.kr/B552584/ArpltnInforInqireSvc/getMinuDustWeekFrcstDspth?serviceKey=3%2F4V%2B1ncUo7UPucSMozyvVmsx47Iu%2B3gPXTQQTRD7HPfgAgwysao0LcfwPWBpqgwfEN0Z0viFVO2BpDFPrd00A%3D%3D&returnType=xml&numOfRows=5&pageNo=1&searchDate=";
String airadr = "";

//최종결과값
char API_DATA[17][60] = {0,};

String velue1 = "";
String velue2 = "";
String velue3 = "";
String velue4 = "";

const byte interruptPin = 5;
volatile byte state = LOW;

void setup()
{
  digitalWrite(14, HIGH);
  // 시리얼 세팅
  Serial.begin(9600);
  nodemcu.begin(9600);

  delay(100);

  // 와이파이 접속
  //WiFi.begin("MJU_Wireless", ""); // 공유기 이름과 비밀번호
  WiFi.begin("u.s_1.2", "dmltjs123@");
  //WiFi.begin("euiseon", "12345678");
  //WiFi.begin("AndroidHotspot5339", "qazwsxedc12");
  //WiFi.begin("ATmega128", "qwer1234");
  //WiFi.begin("TWOSOME", "A0313230044");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) // 와이파이 접속하는 동안 "." 출력
  {
    delay(10);
    Serial.print(".");
  }
  Serial.println();

  //Serial.print("Connected, IP address: ");
  //Serial.println(WiFi.localIP()); // 접속된 와이파이 주소 출력
  get_NTP();
  get_today();

  updateWeatherAPI(); delay(100);
  updateAMRateAPI(); delay(100);
  updateJPRateAPI(); delay(100);
  updateEURateAPI();  delay(100);
  updateCHRateAPI();  delay(100);
  updateCovidAPI();  delay(100);
  updateAirAPI();  delay(100);
  updateClockAPI();  delay(100);
  /*pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin),updateWeatherAPI,CHANGE);*/

  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(14, LOW);
  delay(100);
  digitalWrite(14, HIGH);



}

void loop() {
  String ch;
  while (nodemcu.available() > 0) {
    //Serial.println(nodemcu.available());
    ch = nodemcu.readString();
    Serial.println(String(ch).toInt());
    if (String(ch).toInt() >= 0 && String(ch).toInt() <= 16 ) {
      nodemcu.write(API_DATA[String(ch).toInt()]);
      //String velue = velue1 + velue2 + velue3 + velue4;
      //Serial.write(String(ch).toInt());
      Serial.println(API_DATA[String(ch).toInt()]);
      //ch = NULL;
    }

    /*else if ((String)ch == "reflesh") {
      updateWeatherAPI();
      delay(100);
      updateMovieAPI();
      delay(100);
      updateCovidAPI();
      delay(100);
      updateStockAPI();
      delay(100);
      String velue = velue1 + velue2 + velue3 + velue4;
      nodemcu.print(velue);
      }*/


  }
}

void updateWeatherAPI() {
  //날씨
  if (WiFi.status() == WL_CONNECTED) {
    myClient.begin(wificlient, "http://api.openweathermap.org/data/2.5/weather?q=Seoul&appid=a0669da9b085e97d912114e68783ebfa");
    int getResult1 = myClient.GET();
    if (getResult1 == HTTP_CODE_OK) //200
    {
      String receivedData1 = myClient.getString();//이거
      deserializeJson(test1, receivedData1);

      city = test1["name"];
      temp = (float)(test1["main"]["temp"]) - 273.00;
      humidity = (float)(test1["main"]["humidity"]);
      wind = (float)(test1["wind"]["speed"]);
      wind_deg = (float)(test1["wind"]["deg"]);
      detailwhe = test1["weather"][0]["description"];

      Serial.println("<Ready for WeatherAPI>");
      velue1 = String("(1" + (String)city + ":" + (String)temp + ":" + (String)humidity + ":" + (String)detailwhe +  ":" + (String)wind + ":" + (String)wind_deg + ")");
      strcpy(API_DATA[4], velue1.c_str());

      Serial.println(velue1.c_str());
      //nodemcu.write(velue.c_str());
    }
    else
    {
      velue1 = String("(1 WeatherAPI ERROR)");
      //nodemcu.write(error.c_str());
      Serial.printf("WeatherAPI ERROR, code : %d \r\n", getResult1);

    }
    Serial.println();
    myClient.end();

    //delay(100);
  }
}

void updateCovidAPI() {
  String receivedData3 = "";
  if (WiFi.status() == WL_CONNECTED) {
    //코로나
    myClient3.begin(wificlient, covidadr);
    int getResult3 = myClient3.GET();
    if (getResult3 == HTTP_CODE_OK) //200
    {
      receivedData3 = myClient3.getString();//이거
      int a = 111;
      int i = 0;

      while (i <= 6) {
        // 인트로 111, item 사이464
        int statenum = receivedData3.indexOf("<stateDt>", a); // 기준일
        int statenum2 = receivedData3.indexOf("</stateDt>", a);
        int examnum = receivedData3.indexOf("<examCnt>", a); // 검사진행 수
        int examnum2 = receivedData3.indexOf("</examCnt>", a);
        int decidenum = receivedData3.indexOf("<decideCnt>", a); // 확진자 수
        int decidenum2 = receivedData3.indexOf("</decideCnt>", a);
        int clearnum = receivedData3.indexOf("<clearCnt>", a); // 격리해제 수
        int clearnum2 = receivedData3.indexOf("</clearCnt>", a);
        int deathnum = receivedData3.indexOf("<deathCnt>", a); // 사망자 수
        int deathnum2 = receivedData3.indexOf("</deathCnt>", a);

        String STATE_DT = receivedData3.substring(statenum + 9, statenum2);
        String EXAM_CNT = receivedData3.substring(examnum + 9, examnum2);
        String DECIDE_CNT = receivedData3.substring(decidenum + 11, decidenum2);
        String CLEAR_CNT = receivedData3.substring(clearnum + 10, clearnum2);
        String DEATH_CNT = receivedData3.substring(deathnum + 10, deathnum2);

        String todaycovid = String("(3" + STATE_DT + ":" + EXAM_CNT
                                   + ":" + CLEAR_CNT + ":" + DEATH_CNT + ")");

        strcpy(covid_day[i], todaycovid.c_str());
        covid_decide[i] = atoi(DECIDE_CNT.c_str());
        strcpy(API_DATA[i + 5], covid_day[i]);

        if (i >= 1 && i < 6) {
          int result = covid_decide[i - 1] - covid_decide[i];
          covid_day_decide = String(covid_day_decide + String(result) + ":");
        }
        else if (i == 6) {
          int result = covid_decide[i - 1] - covid_decide[i];
          covid_day_decide = String(covid_day_decide + String(result) + ")");
        }

        // Serial.println(covid_decide[i]);
        Serial.println(covid_day[i]);
        a = a + 464;
        i++;
      }
      strcpy(API_DATA[12], covid_day_decide.c_str());
      Serial.println(covid_day_decide);

      int accexam = receivedData3.indexOf("<accExamCnt>"); // 누적 검사 수
      int accexam2 = receivedData3.indexOf("</accExamCnt>");
      int accexamcom = receivedData3.indexOf("<accExamCompCnt>"); // 누적 검사 완료 수
      int accexamcom2 = receivedData3.indexOf("</accExamCompCnt>");
      int accrate = receivedData3.indexOf("<accDefRate>"); // 누적 확진률
      int accrate2 = receivedData3.indexOf("</accDefRate>");

      String ACC_EXAM = receivedData3.substring(accexam + 12, accexam2);
      String ACC_EXAM_COM = receivedData3.substring(accexamcom + 16, accexamcom2);
      String ACC_RATE = receivedData3.substring(accrate + 12, accrate2);

      velue3 = String("(" + (String)3 + ACC_EXAM + ":" + ACC_EXAM_COM + ":" + ACC_RATE + ")");
      strcpy(API_DATA[13], velue3.c_str());
      Serial.println(velue3.c_str());


    }
    else
    {
      velue3 = String("(" + (String)3 + "CovidAPI ERROR)");
      //nodemcu.write(error.c_str());
      Serial.printf("CovidAPI ERROR, code : %d \r\n", getResult3);
    }
    myClient3.end();

    //delay(100);
  }
}

void updateAMRateAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    myClient2.begin(wificlient, rateadr1);
    int getResult21 = myClient2.GET();
    int data_value11;
    int data_value12;

    if (getResult21 == HTTP_CODE_OK) {
      String receivedData21 = myClient2.getString();

      int i = 0;
      int a = 348;
      strcpy(rate[0], "(2");

      while (i <= 3) {
        data_value11 = receivedData21.indexOf("<DATA_VALUE>", a);
        data_value12 = receivedData21.indexOf("</DATA_VALUE>", a);
        DOLLAR = receivedData21.substring(data_value11 + 12, data_value12);
        strcat(rate[0], DOLLAR.c_str());
        strcat(rate[0], ":");
        i++;
        a = a + 348;
      }
      data_value11 = receivedData21.indexOf("<DATA_VALUE>", 1796);
      data_value12 = receivedData21.indexOf("</DATA_VALUE>", 1796);
      DOLLAR = receivedData21.substring(data_value11 + 12, data_value12);
      strcat(rate[0], DOLLAR.c_str());
      strcat(rate[0], ")");
      Serial.println(rate[0]);
      strcpy(API_DATA[0], rate[0]);
    }

    else {
      String error = String("(" + (String)2 + "AM_API ERROR)");
      nodemcu.write(error.c_str());
      Serial.printf("1 AM_RATEAPI ERROR, code : %d \r\n", getResult21);
    }
    myClient2.end();
  }
}

void updateJPRateAPI() {
  myClient2.begin(wificlient, rateadr2);
  int getResult22 = myClient2.GET();
  int data_value11;
  int data_value12;

  if (getResult22 == HTTP_CODE_OK) {
    String receivedData21 = myClient2.getString();

    int i = 0;
    int a = 348;
    strcpy(rate[1], "(2");

    while (i <= 3) {
      data_value11 = receivedData21.indexOf("<DATA_VALUE>", a);
      data_value12 = receivedData21.indexOf("</DATA_VALUE>", a);
      YEN = receivedData21.substring(data_value11 + 12, data_value12);
      strcat(rate[1], YEN.c_str());
      strcat(rate[1], ":");
      i++;
      a = a + 348;
    }
    data_value11 = receivedData21.indexOf("<DATA_VALUE>", 1796);
    data_value12 = receivedData21.indexOf("</DATA_VALUE>", 1796);
    DOLLAR = receivedData21.substring(data_value11 + 12, data_value12);
    strcat(rate[1], YEN.c_str());
    strcat(rate[1], ")");
    Serial.println(rate[1]);
    strcpy(API_DATA[1], rate[1]);
  }

  else {
    String error = String("(" + (String)2 + "JP_RATEAPI ERROR)");
    nodemcu.write(error.c_str());
    Serial.printf("2 JP_RATEAPI ERROR, code : %d \r\n", getResult22);
  }
  myClient2.end();
}

void updateEURateAPI() {
  myClient2.begin(wificlient, rateadr3);
  int getResult23 = myClient2.GET();
  int data_value11;
  int data_value12;

  if (getResult23 == HTTP_CODE_OK) {
    String receivedData21 = myClient2.getString();

    int i = 0;
    int a = 348;
    strcpy(rate[2], "(2");

    while (i <= 3) {
      data_value11 = receivedData21.indexOf("<DATA_VALUE>", a);
      data_value12 = receivedData21.indexOf("</DATA_VALUE>", a);
      EURO = receivedData21.substring(data_value11 + 12, data_value12);
      strcat(rate[2], EURO.c_str());
      strcat(rate[2], ":");
      i++;
      a = a + 348;
    }
    data_value11 = receivedData21.indexOf("<DATA_VALUE>", 1796);
    data_value12 = receivedData21.indexOf("</DATA_VALUE>", 1796);
    EURO = receivedData21.substring(data_value11 + 12, data_value12);
    strcat(rate[2], EURO.c_str());
    strcat(rate[2], ")");
    Serial.println(rate[2]);
    strcpy(API_DATA[2], rate[2]);
  }

  else {
    String error = String("(" + (String)2 + "EU_RATEAPI ERROR)");
    nodemcu.write(error.c_str());
    Serial.printf("2 EU_RATEAPI ERROR, code : %d \r\n", getResult23);
  }
  myClient2.end();
}

void updateCHRateAPI() {
  myClient2.begin(wificlient, rateadr53);
  int getResult253 = myClient2.GET();
  int data_value11;
  int data_value12;

  if (getResult253 == HTTP_CODE_OK) {
    String receivedData21 = myClient2.getString();

    int i = 0;
    int a = 348;
    strcpy(rate[3], "(2");

    while (i <= 3) {
      data_value11 = receivedData21.indexOf("<DATA_VALUE>", a);
      data_value12 = receivedData21.indexOf("</DATA_VALUE>", a);
      YUAN = receivedData21.substring(data_value11 + 12, data_value12);
      strcat(rate[3], YUAN.c_str());
      strcat(rate[3], ":");
      i++;
      a = a + 348;
    }
    data_value11 = receivedData21.indexOf("<DATA_VALUE>", 1796);
    data_value12 = receivedData21.indexOf("</DATA_VALUE>", 1796);
    YUAN = receivedData21.substring(data_value11 + 12, data_value12);
    strcat(rate[3], YUAN.c_str());
    strcat(rate[3], ")");
    Serial.println(rate[3]);
    Serial.println();
    strcpy(API_DATA[3], rate[3]);
  }

  else {
    String error = String("(" + (String)2 + "EU_RATEAPI ERROR)");
    nodemcu.write(error.c_str());
    Serial.printf("2 CH_RATEAPI ERROR, code : %d \r\n", getResult253);
  }
  myClient2.end();
}



void updateAirAPI() {
  myClient4.begin(wificlient, airadr); int getResult4 = myClient4.GET();

  if (getResult4 == HTTP_CODE_OK) {
    String receivedData4 = myClient4.getString();
    int data_value41 = receivedData4.indexOf("<frcstOneCn>");
    int data_value42 = receivedData4.indexOf("</frcstOneCn>");
    String area = receivedData4.substring(data_value41 + 12, data_value42);

    int Seoul1 = area.indexOf("서울");
    String Seoul2 = area.substring(Seoul1 + 9, Seoul1 + 15);
    if (Seoul2 == "낮음") Seoul2 = "Low"; else if (Seoul2 == "보통") Seoul2 = "Normal"; else if (Seoul2 == "높음") Seoul2 = "High";
    int Incheon1 = area.indexOf("인천");
    String Incheon2 = area.substring(Incheon1 + 9, Incheon1 + 15);
    if (Incheon2 == "낮음") Incheon2 = "Low"; else if (Incheon2 == "보통") Incheon2 = "Normal"; else if (Incheon2 == "높음") Incheon2 = "High";
    int Daejeon1 = area.indexOf("대전");
    String Daejeon2 = area.substring(Daejeon1 + 9, Daejeon1 + 15);
    if (Daejeon2 == "낮음") Daejeon2 = "Low"; else if (Daejeon2 == "보통") Daejeon2 = "Normal"; else if (Daejeon2 == "높음") Daejeon2 = "High";
    int Sejong1 = area.indexOf("세종");
    String Sejong2 = area.substring(Sejong1 + 9, Sejong1 + 15);
    if (Sejong2 == "낮음") Sejong2 = "Low"; else if (Sejong2 == "보통") Sejong2 = "Normal"; else if (Sejong2 == "높음") Sejong2 = "High";
    int Gwangju1 = area.indexOf("광주");
    String Gwangju2 = area.substring(Gwangju1 + 9, Gwangju1 + 15);
    if (Gwangju2 == "낮음") Gwangju2 = "Low"; else if (Gwangju2 == "보통") Gwangju2 = "Normal"; else if (Gwangju2 == "높음") Gwangju2 = "High";
    int Busan1 = area.indexOf("부산");
    String Busan2 = area.substring(Busan1 + 9, Busan1 + 15);
    if (Busan2 == "낮음") Busan2 = "Low"; else if (Busan2 == "보통") Busan2 = "Normal"; else if (Busan2 == "높음") Busan2 = "High";
    int Daegu1 = area.indexOf("대구");
    String Daegu2 = area.substring(Daegu1 + 9, Daegu1 + 15);
    if (Daegu2 == "낮음") Daegu2 = "Low"; else if (Daegu2 == "보통") Daegu2 = "Normal"; else if (Daegu2 == "높음") Daegu2 = "High";
    int Ulsan1 = area.indexOf("울산");
    String Ulsan2 = area.substring(Ulsan1 + 9, Ulsan1 + 15);
    if (Ulsan2 == "낮음") Ulsan2 = "Low"; else if (Ulsan2 == "보통") Ulsan2 = "Normal"; else if (Ulsan2 == "높음") Ulsan2 = "High";
    int Jeju1 = area.indexOf("제주");
    String Jeju2 = area.substring(Jeju1 + 9, Jeju1 + 15);
    if (Jeju2 == "낮음") Jeju2 = "Low"; else if (Jeju2 == "보통") Jeju2 = "Normal"; else if (Jeju2 == "높음") Jeju2 = "High";

    String kr_air1 = String("(4" + Seoul2 + ":" + Incheon2 +
                            ":" + Daejeon2 + ":" + Sejong2 + ")");

    String kr_air2 = String("(4" + Gwangju2 + ":" + Busan2 +
                            ":" + Daegu2 + ":" + Ulsan2 + ":" + Jeju2 + ")");

    strcpy(API_DATA[14], kr_air1.c_str());
    strcpy(API_DATA[15], kr_air2.c_str());
    Serial.println("<Ready for AirAPI>");
    Serial.println(kr_air1);
    Serial.println(kr_air2);
    Serial.println();
  }

  else {
    String error = String("(" + (String)4 + "AIRAPI ERROR)");
    nodemcu.write(error.c_str());
    Serial.printf("4 Air ERROR, code : %d \r\n", getResult4);
  }
  myClient4.end();
}

void updateClockAPI() {
  if (WiFi.status() == WL_CONNECTED)
  {
    String temp = Serial.readStringUntil('\n');
    if (temp == "1") set_time(); // set time
    else if (temp == "2") get_NTP(); // NTP Sync
    printLocalTime();
  }
}

void printLocalTime() {
  if (time(&now) != prevEpoch) { // 현재 UTC 시간 값과 이전 UTC 시간 값이 다르면
    // Serial.println(time(&now)); // 현재 UTC 시간 값 출력
    timeinfo = localtime(&now); // 로컬 변경함수이용 UTC 시간값 변경
    int dd = timeinfo->tm_mday; // 구조체 내 해당값 가져오기
    int MM = timeinfo->tm_mon + 1;
    int yy = timeinfo->tm_year + 1900;
    int ss = timeinfo->tm_sec;
    int mm = timeinfo->tm_min;
    int hh = timeinfo->tm_hour;
    // Serial.printf("%d. %d. %d\n%d: %d: %d\n", yy, MM, dd, hh, mm, ss);
    // Serial.print("\n");
    // prevEpoch = time(&now); // 현재 UTC 시간 값을 저장하여 1초마다 실행되도록 함.
    Serial.println("<Ready for Clock>");
    String velue = String("(" + (String)5 + yy + ":" + MM + ":" + dd + ":" + hh + ":" + mm + ":" + ss + ")");
    strcpy(API_DATA[16], velue.c_str());
    Serial.println(velue.c_str());
  }
}

void set_time() {
  struct tm tm_in;
  tm_in.tm_year = s_yy - 1900;
  tm_in.tm_mon = s_MM - 1;
  tm_in.tm_mday = s_dd;
  tm_in.tm_hour = s_hh;
  tm_in.tm_min = s_mm;
  tm_in.tm_sec = s_ss;
  time_t ts = mktime(&tm_in);
  // printf("Setting time: %s", asctime(&tm_in));
  struct timeval now = { .tv_sec = ts };
  settimeofday(&now, NULL);
}

void get_NTP() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer);
  timeinfo = localtime(&now);
  while (timeinfo->tm_year + 1900 == 1970) {
    // Serial.println("Failed to obtain time");
    set_time();
    localtime(&now);
    return;
  }
}

void get_today() { // 날짜에 따른 movie, covid api 주소 최신화
  if (WiFi.status() == WL_CONNECTED)
  {
    String temp = Serial.readStringUntil('\n');
    if (temp == "1") set_time(); // set time
    else if (temp == "2") get_NTP(); // NTP Sync

    if (time(&now) != prevEpoch) { // 현재 UTC 시간 값과 이전 UTC 시간 값이 다르면
      // Serial.println(time(&now)); // 현재 UTC 시간 값 출력
      timeinfo = localtime(&now); // 로컬 변경함수이용 UTC 시간값 변경
      // 구조체 내 해당값 가져오기
      int yy = timeinfo->tm_year + 1900;
      int MM = timeinfo->tm_mon + 1;
      int dd = timeinfo->tm_mday;
      int ss = timeinfo->tm_sec;
      int mm = timeinfo->tm_min;
      int hh = timeinfo->tm_hour;


      String today_str = String(String(yy) + String(MM) + String(dd));

      int dd_1 = (dd - 1) % month_days;
      int dd_7 = (dd - 7) % month_days;
      int dd_3 = (dd - 3) % month_days;
      if (dd_1 <= 0) {
        total_days(timeinfo->tm_mon);
        dd_1 = month_days - dd_1;
      }
      else if (dd_7 <= 0) {
        total_days(timeinfo->tm_mon);
        dd_7 = month_days - dd_7;
      }
      else if (dd_3 <= 0) {
        total_days(timeinfo->tm_mon);
        dd_3 = month_days - dd_3;
      }
      String three_ago_str_line = String(yy);
      String yesterday = String(yy);
      String seven_ago = String(yy);
      if(MM < 10){
        three_ago_str_line += String(yy);
        yesterday = String(yy);
        seven_ago = String(yy);
      }
      else 
      if(dd_1 < 10){
          yesterday = String(String(yy) + "0" + String(MM) + "0" + String(dd_1));
        }
        else yesterday = String(String(yy) + "0" + String(MM) + String(dd_1));
        if(dd_7 < 10){
          
        }
        if(dd_3 < 10){
          
        }
      String three_ago_str_line = String(String(yy) + "-" + String(MM) + "-" + String(dd_3));
      String yesterday = String(String(yy) + String(MM) + String(dd_1));
      String seven_ago = String(String(yy) + String(MM) + String(dd_7));

     
      
      covidadr = String(covidapi + seven_ago + "&endCreateDt=" + yesterday);
      rateadr1 = String(rateapi + "20211122/20211126/0000001");
      rateadr2 = String(rateapi + "20211122/20211126/0000002");
      rateadr3 = String(rateapi + "20211122/20211126/0000003");
      rateadr53 = String(rateapi + "20211122/20211126/0000053");
      /* rateadr1 = String(rateapi + seven_ago + "/" + seven_ago + "/0000001");
        rateadr2 = String(rateapi + seven_ago + "/" + seven_ago + "/0000002");
        rateadr3 = String(rateapi + seven_ago + "/" + seven_ago + "/0000003");
        rateadr53 = String(rateapi + seven_ago + "/" + seven_ago + "/0000053"); */
      airadr = String(airapi + three_ago_str_line);
    }
  }
}
void total_days(int Month) {
  if (Month == 1 || Month == 3 || Month == 5 || Month == 7 || Month == 8 || Month == 10 || Month == 0) {
    month_days = 31;
  }
  else if (Month == 4 || Month == 6 || Month == 9 || Month == 11) {
    month_days = 30;
  }
  else if (Month == 2) {
    month_days = 28;
  }
}
