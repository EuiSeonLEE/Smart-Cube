#include <SoftwareSerial.h>
#include <UTFT.h>
#include <URTouch.h>
#include <Wire.h>

String wifi_id[5];
String wifi_pw[5];

int MPU_Address = 0x68;
int Resister = 0x41;
int16_t AcX, AcY, AcZ;
double angleAcX, angleAcY, angleAcZ;

double FiAcX, FiAcY;
double perangleAcX, perangleAcY;

double speedAcX, speedAcY;
double AcspeedAcX, AcspeedAcY;
double per_speedAcX, per_speedAcY;

unsigned long delaytime = 0;
unsigned long delaytime2 = 0;

const double RADIAN_TO_DEGREE = 180 / 3.14159;

unsigned long now = 0;   // 현재 시간 저장용 변수
unsigned long past = 0;  // 이전 시간 저장용 변수
double dt = 0;           // 한 사이클 동안 걸린 시간 변수

double averAcX, averAcY, averAcZ;

const int BUFFER_SIZE = 50;
char buf[BUFFER_SIZE];

int per_result = 0;
int result;

int covid_touch = 0, per_covid_touch = 1;
int rate_touch = 0, per_rate_touch = 1;

String serString = "";
int serStringIndex = 0;

String covid_day[7] = {"",};
String covid_day_decide = "";

String s = "";
String s1 = "";
String s2 = "";
String s3 = "";
String s4 = "";
char API_DATA[17][60] = {0,};
char WiFi_DATA[2][64] = {0,};
String temp = "";
String am_rate_data[5];// 환율(달러, 엔화, 유로, 위안)
String jp_rate_data[5];
String eu_rate_data[5];
String ch_rate_data[5];
String wea_data[6]; // 날씨(도시, 기온, 습도, 날씨, 풍속, 풍향)
String covid_data[9]; // 6일치 일일확진자,누적검자수,누적검사완료수,누적확진율
String air_data[9]; // 서울, 인천, 대전, 세종, 광주, 부산, 대구, 울산, 제주

String velue = "";
int a = 0;

UTFT myGLCD(ILI9341_16, 38, 39, 40, 41);
URTouch  myTouch( 6, 5, 4, 3, 2);
SoftwareSerial Mega(A8, A9);

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
int x, y, y1 = 0;
char stCurrent[40] = "";
int stCurrentLen = 0;
char stLast[40] = "";

void setup() {
  Serial.begin(9600);
  Mega.begin(9600);

  READwifi();
  wifi_split();
  
  READnodeMCU();

  am_rate_split();
  jp_rate_split();
  eu_rate_split();
  ch_rate_split();
  wea_split();
  covid_split();
  air_split();

  myGLCD.InitLCD();
  myGLCD.clrScr();

  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  myGLCD.setFont(SmallFont);
  myGLCD.setBackColor(0, 0, 0);

  caliSensor();  //  초기 센서 캘리브레이션 함수 호출
  past = millis(); // past에 현재 시간 저장

  Wire.begin();
  Wire.setClock(100000);
  init_MPU6050();
  //String num = "0";
}

int reverseY(int y){
  int temp = 0;
  if ((y >= 180) && (y <= 230))
    temp = y - 170;
  else if ((y >= 120) && (y <= 170))
    temp = y - 50;
  else if ((y >= 60) && (y <= 110))
    temp = y + 70;
  else if ((y >= 0) && (y <= 50))
    temp = y + 190;
  return temp;
}
void take_wifi(){
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.printNumI(x+1, 27+(x*60), 27);
  }
// Draw the center row of buttons
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 70, 60+(x*60), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 70, 60+(x*60), 120);
    if (x<4)
      myGLCD.printNumI(x+6, 27+(x*60), 87);
  }
}
String Rotate(String str) {
  String sub = str;
  if (sub.substring(0, 1) == "(") {
    a = sub.substring(1, 2).toInt();
    //Serial.println(sub.substring(1,2).toInt());
    sub = sub.substring(2);
  }

  return sub;
}

String split(String str, String b) {
  int index1 = 0, index2 = 0, index3 = 0;
  String sub = str;
  index2 = sub.length();

  while (index2 != NULL) //sub문자열이 NULL이 될 때까지
  {
    index1 = sub.indexOf(b); //":"가 있는 문자 갯수
    index3 = sub.indexOf(")"); // ")"가 있는 문자 갯수
    if ((b == ":") && (index1 != -1) && (index3 != -1)) { // ":",")" 둘다 존재 한다면
      temp = sub.substring(0, index1); // 0번째 부터 ":"가 있는 문자 갯수만큼 String복사
      sub = sub.substring(index1 + 1); // ":"다음 문자열부터 문자열 끝까지 String 복사
    }
    else if ((b == ":") && index3 != -1) { //")"만 존재한다면
      temp = sub.substring(0, index3); //0번째 부터 ")"가 있는 문자 갯수만큼 String 복사
      sub = sub.substring(index3 + 1); // NULL이 되게 만듦
    }
    else {
      break;//while문에서 exit
    }
    if ((b == ",") && (index1 != -1)) {
      temp = sub.substring(0, index1);
      sub = sub.substring(index1 + 1);
    }
    else  { //전부 존재하지 않는다면
      //Serial.println(sub);
      return sub;
    }
    //Serial.println(sub);
    return sub;
  }
}
void READwifi() {
  bool c = true;
  int b = 0;
  char ch;
  while (1) {
    if (c) {
      Serial.println((String(b)).c_str());
      Mega.write((String(b)).c_str());
      c = false;
    }
    while (Mega.available() > 0) {
      //Serial.println(Mega.available());
      ch = Mega.read();
      //Serial.print((String)ch);
      if (Mega.overflow()) {
        Serial.println("SoftwareSerial overflow!");
      }
      if (serStringIndex == 0) {
        serString = "";
        serString += ch;
        serStringIndex ++;

      }
      else  {
        serString += ch;
        serStringIndex ++;
      }

      if (serStringIndex != 0 && ch == ')') {
        //화면 초기화
        //tft lcd에 변수 띄우는 함수
        Serial.println(serString);
        strcpy(WiFi_DATA[b], serString.c_str());
        Serial.println(WiFi_DATA[b]);
        serStringIndex = 0;
        c = true;
        b++;
      }
    }

    if (b >= 2) break;
  }
}
void READnodeMCU() {
  bool c = true;
  int b = 0;
  char ch;


  while (1) {
    if (c) {
      Serial.println((String(b)).c_str());
      Mega.write((String(b)).c_str());
      c = false;
    }

    while (Mega.available() > 0) {
      //Serial.println(Mega.available());
      ch = Mega.read();
      //Serial.print((String)ch);
      if (Mega.overflow()) {
        Serial.println("SoftwareSerial overflow!");
      }
      if (serStringIndex == 0) {
        serString = "";
        serString += ch;
        serStringIndex ++;

      }
      else  {
        serString += ch;
        serStringIndex ++;
      }

      if (serStringIndex != 0 && ch == ')') {
        //화면 초기화
        //tft lcd에 변수 띄우는 함수
        Serial.println(serString);

        strcpy(API_DATA[b], serString.c_str());
        Serial.println(API_DATA[b]);
        serStringIndex = 0;
        c = true;
        b++;
      }
    }

    if (b >= 17) break;
  }
}

void loop() {
  //READnodeMCU();

  getData();
  getDT();
  // 삼각함수를 이용한 롤(Roll)의 각도 구하기
  angleAcX = atan(AcY / sqrt(pow(AcX, 2) + pow(AcZ, 2)));
  angleAcX *= RADIAN_TO_DEGREE;
  // 삼각함수를 이용한 피치(Pitch)의 각도 구하기
  angleAcY = atan(-AcX / sqrt(pow(AcY, 2) + pow(AcZ, 2)));
  angleAcY *= RADIAN_TO_DEGREE;

  FiAcX = perangleAcX * (1 - 0.1) + angleAcX * 0.1;
  FiAcY = perangleAcY * (1 - 0.1) + angleAcY * 0.1;
  //각속도
  speedAcX = abs(FiAcX - perangleAcX) / dt;
  speedAcY = abs(FiAcY - perangleAcY) / dt;
  //각가속도
  AcspeedAcX = (abs(speedAcX - per_speedAcX) / dt) / 100;
  AcspeedAcY = (abs(speedAcY - per_speedAcY) / dt) / 100;

  per_speedAcX = speedAcX;
  per_speedAcY = speedAcY;

  perangleAcX = FiAcX;
  perangleAcY = FiAcY;


  //angleFiZ = angleGyZ;    // Z축은 자이로 센서만을 이용하여 구함.
  /*Serial.print("FiAcX:");
    Serial.print(FiAcX);
    Serial.print("\t FiAcY:");
    Serial.print(FiAcY);*/
  /*Serial.print("FilteredX:");
    Serial.print(angleFiX);
    Serial.print("\t FilteredY:");
    Serial.print(angleFiY);*/
  /*Serial.print("\t speedAcX:");
    Serial.print(AcspeedAcX);
    Serial.print("\t speedAcY:");
    Serial.println(AcspeedAcY);*/

  if (-45.0 < FiAcX && FiAcX < 45.0 && FiAcY < -45.0) {
    //Serial.print("DOWN\r\n");
    result = 3;
  }
  else if (FiAcX < -45.0 && -45.0 < FiAcY && FiAcY < 45.0) {
    //Serial.print("RIGHT\r\n");
    result = 4;
  }
  else if (-45.0 < FiAcX && FiAcX < 45.0 && FiAcY > 45.0) {
    //Serial.print("UP\r\n");
    result = 1;
  }
  else if (FiAcX > 45.0 && -45.0 < FiAcY && FiAcY < 45.0)  {
    //Serial.print("LEFT\r\n");
    result = 2;
  }
  else if (-45.0 < FiAcX && FiAcX < 45.0 && -45.0 < FiAcY && FiAcY < 45.0) {
    //Serial.print("BACK\r\n");
    result = 5;
  }
  else {
    //Serial.print("!!!!!\r\n");
    result = result;
  }

  delaytime = now;
  if ((AcspeedAcX < 1000 || AcspeedAcY < 1000) && (delaytime - delaytime2 >= 1000)) {

    if (per_result != result) {
      switch (result) {
        case 1: {
            per_result = result;
            if (per_rate_touch == rate_touch) {
              per_rate_touch = (per_rate_touch + 1) % 4;
            }

            break;
          }
        case 2: {
            per_result = result;
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(0, 0, 319, 239);
            myGLCD.setColor(0, 0, 255);
            myGLCD.setBackColor(255, 0, 0);
            myGLCD.setFont(BigFont);

            velue = String("Weather");
            myGLCD.print(velue, 10, 239, -90);
            myGLCD.setBackColor(0, 0, 0);
            myGLCD.setColor(180, 180, 255);
            velue = String(wea_data[0]);
            myGLCD.print(velue, 30, 239, -90);
            myGLCD.print(wea_data[3], 80, 239, -90);
            myGLCD.print(wea_data[2], 130, 239, -90);
            myGLCD.print(wea_data[4], 180, 239, -90);
            velue = String(wea_data[1]);
            myGLCD.print(velue, 30, 130, -90);
            myGLCD.print(wea_data[5], 230, 239, -90);
            myGLCD.setColor(80, 80, 255);
            myGLCD.setFont(SmallFont);
            myGLCD.print("weather", 60, 239, -90);
            myGLCD.print("humidity", 110, 239, -90);
            myGLCD.print("wind", 160, 239, -90);
            myGLCD.print("wind_deg", 210, 239, -90);
            break;
          }
        case 3: {
            per_result = result;
            if (per_covid_touch == covid_touch) {
              per_covid_touch = (per_covid_touch + 1) % 2;
            }
            break;
          }
        case 4: {
            per_result = result;
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(0, 0, 319, 239);
            myGLCD.setColor(255, 0, 255);
            myGLCD.setBackColor(0, 255, 0);
            myGLCD.setFont(BigFont);

            velue = String("Air Quality");
            myGLCD.print(velue, 310, 1, 90);
            myGLCD.setBackColor(0, 0 , 0);

            velue = String("Seoul: " + air_data[0]);
            air_quality_circle(0, 265, 220);
            myGLCD.print(velue, 270, 1, 90);

            velue = String("Incheon: " + air_data[1]);
            air_quality_circle(1, 235, 220);
            myGLCD.print(velue, 240, 1, 90);

            velue = String("Daejeon: " + air_data[2]);
            air_quality_circle(2, 205, 220);
            myGLCD.print(velue, 210, 1, 90);

            velue = String("Sejong: " + air_data[3]);
            air_quality_circle(3, 175, 220);
            myGLCD.print(velue, 180, 1, 90);

            velue = String("Gwangju: " + air_data[4]);
            air_quality_circle(4, 145, 220);
            myGLCD.print(velue, 150, 1, 90);

            velue = String("Busan: " + air_data[5]);
            air_quality_circle(5, 115, 220);
            myGLCD.print(velue, 120, 1, 90);

            velue = String("Daegu: " + air_data[6]);
            air_quality_circle(6, 85, 220);
            myGLCD.print(velue, 90, 1, 90);

            velue = String("Ulsan: " + air_data[7]);
            air_quality_circle(7, 55, 220);
            myGLCD.print(velue, 60, 1, 90);

            velue = String("Jeju : " + air_data[8]);
            air_quality_circle(8, 25, 220);
            myGLCD.print(velue, 30, 1, 90);

            Serial.println(velue);

            break;
          }
        case 5: {
            per_result = result;
            break;
          }
        default: {
            break;
          }
      }
    }
    if (result == 5) {
      String Clock_time = split(Rotate(API_DATA[16]), ":");
      String Year = temp;
      Clock_time = split(Clock_time, ":");
      String Month = temp;
      Clock_time = split(Clock_time, ":");
      String Day = String((temp.toInt() + (now / 86400000)) % 30);
      Clock_time = split(Clock_time, ":");
      String Hour = String((temp.toInt() + (now / 360000)) % 24);
      Clock_time = split(Clock_time, ":");
      String Min = String((temp.toInt() + (now / 60000)) % 60);
      Clock_time = split(Clock_time, ":");
      String Sec = String((now / 1000) % 60);

      String Clock_velue1 = String(Year + " : " + Month + " : " + Day);
      String Clock_velue2 = String(Hour + " : " + Min + " : " + Sec);
      Serial.println("clock");
      /*  myGLCD.setColor(0, 0, 0);
        myGLCD.fillRect(0, 0, 319, 239);*/
      myGLCD.setBackColor(255, 100, 100);
      myGLCD.setFont(BigFont);
      myGLCD.setColor(255, 100, 100);
      myGLCD.fillRoundRect (20, 20, 300, 220);
      myGLCD.setColor(255, 50, 50);
      myGLCD.drawRoundRect (20, 20, 300, 220);

      myGLCD.setColor(255, 255, 255);
      myGLCD.print("CLOCK" , 120, 30, 0);
      myGLCD.print(Clock_velue1 , 50, 80, 0);
      myGLCD.print(Clock_velue2 , 80, 160, 0);
    }

  }
  else if (!(AcspeedAcX < 1000 || AcspeedAcY < 1000)) {

    delaytime2 = delaytime;
  }
  switch (result) {
    case 1: {
        rate_graph();
        break;
      }
    case 2 : {
        break;
      }
    case 3 : {
        covid_graph();
        break;
      }
    case 4 : {
        break;
      }
    case 5 : {
        break;
      }
    default: {
        break;
      }
  }

  if (myTouch.dataAvailable())
  {
    myTouch.read();
    x = myTouch.getX();
    y = myTouch.getY();
    y1 = reverseY(y);
    if (result == 1) {//(180, 10, 310, 30);
      if ((y1 >= 10) && (y1 <= 40)) { //refresh
        if ((x >= 180) && (x <= 310)) {
          rate_touch = (rate_touch + 1) % 4;
        }
      }
    }
    if (result == 3) {//(10, 10, 130, 60),(10,170 , 130, 230)
      if ((y1 >= 190) && (y1 <= 230)) { //refresh
        if ((x >= 10) && (x <= 130)) {
          covid_touch = (covid_touch + 1) % 2;
        }
      }
    }
  }
}



void init_MPU6050() {
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x6B);//0x6B(PWR_MGMT_1) 레지스터를 선택
  Wire.write(0x00); //0x6B(PWR_MGMT_1) 레지스터의 슬립모드 해제
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1C);//0x6B(PWR_MGMT_1) 인터럽트 사용 ON
  Wire.write(0x00); //0x6B(PWR_MGMT_1)
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_Address);
  Wire.write(0x1A);//0x1A(PWR_MGMT_1) 레지스터를 선택
  Wire.write(0x06); //0x6B(PWR_MGMT_1) 레지스터의 슬립모드 해제
  Wire.endTransmission(true);
}

void getData() {
  Wire.beginTransmission(MPU_Address);
  Wire.write(0x3B);   // AcX 레지스터 위치(주소)를 지칭합니다
  Wire.endTransmission();
  Wire.requestFrom(MPU_Address , 6, true);  // AcX 주소 이후의 14byte의 데이터를 요청
  AcX = Wire.read() << 8 | Wire.read(); //두 개의 나뉘어진 바이트를 하나로 이어 붙여서 각 변수에 저장
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
}
void getDT() {
  now = millis();
  dt = (now - past) / 1000.0;
  past = now;
}

void caliSensor() {
  double sumAcX = 0 , sumAcY = 0, sumAcZ = 0;

  getData();
  for (int i = 0; i < 10; i++) {
    getData();
    sumAcX += AcX;  sumAcY += AcY;  sumAcZ += AcZ;

    //delay(10);
  }
  averAcX = sumAcX / 10;  averAcY = sumAcY / 10;  averAcZ = sumAcZ / 10;

}
void wifi_split() {
  int i = 1;
  String wifi = "";
  wifi = split(Rotate(WiFi_DATA[0]), ":");
  wifi_id[0] = temp;
  while (i < 5) {
    wifi = split(wifi, ":");
    wifi_id[i] = temp;
    i++
  }
  i = 1;
  wifi = split(Rotate(WiFi_DATA[1]), ":");
  while (i < 5) {
    wifi = split(wifi, ":");
    wifi_pw[i] = temp;
    i++
  }
}
void am_rate_split() {
  int i = 1;
  String R = "";
  R = split(Rotate(API_DATA[0]), ":");
  am_rate_data[0] = temp;
  while (i < 5) {
    R = split(R , ":");
    am_rate_data[i] = temp;
    i++;
  }
}

void jp_rate_split() {
  int i = 1;
  String R = "";
  R = split(Rotate(API_DATA[1]), ":");
  jp_rate_data[0] = temp;
  while (i < 5) {
    R = split(R , ":");
    jp_rate_data[i] = temp;
    i++;
  }
}

void eu_rate_split() {
  int i = 1;
  String R = "";
  R = split(Rotate(API_DATA[2]), ":");
  eu_rate_data[0] = temp;
  while (i < 5) {
    R = split(R , ":");
    eu_rate_data[i] = temp;
    i++;
  }
}

void ch_rate_split() {
  int i = 1;
  String R = "";
  R = split(Rotate(API_DATA[3]), ":");
  ch_rate_data[0] = temp;
  while (i < 5) {
    R = split(R , ":");
    ch_rate_data[i] = temp;
    i++;
  }
}

void wea_split() {
  int i = 1;
  String W = "";
  W = split(Rotate(API_DATA[4]), ":");
  wea_data[0] = temp;
  Serial.println(wea_data[0]);

  while (i < 6) {
    W = split(W, ":");
    wea_data[i] = temp;
    Serial.println(wea_data[i]);
    i++;
  }
}

void covid_split() {
  int i = 1;
  String C = "";
  C = split(Rotate(API_DATA[12]), ":"); // 4068
  covid_data[0] = temp;
  Serial.println(covid_data[0]);
  while (i < 6) {
    C = split(C, ":");
    covid_data[i] = temp;
    Serial.println(covid_data[i]);
    i++;
  }

  i = 1;
  C = split(Rotate(API_DATA[13]), ":"); // 17115763
  covid_data[6] = temp;
  Serial.println(covid_data[6]);
  while (i < 3) {
    C = split(C, ":");
    covid_data[i + 6] = temp;
    Serial.println(covid_data[i + 6]);
    i++;
  }
}

void air_split() {
  int i = 1;
  String A1 = split(Rotate(API_DATA[14]), ":"); // Seoul
  air_data[0] = temp;
  Serial.println(air_data[0]);
  while (i < 4) {
    A1 = split(A1, ":");
    air_data[i] = temp;
    Serial.println(air_data[i] + " " + i);
    i++;
  }

  String A2 = split(Rotate(API_DATA[15]), ":"); // Gwangju
  air_data[i] = temp;
  Serial.println(air_data[i] + " " + i);
  i++;
  while (i < 9) {
    A2 = split(A2, ":");
    air_data[i] = temp;
    Serial.println(air_data[i] + " " + i);
    i++;
  }
}
void covid_graph() {
  int i = 0;
  int covid_graph[6];
  String velue = "";
  if (per_covid_touch != covid_touch) {
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(0, 0, 319, 239);
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10, 190 , 130, 230);

    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(0, 0, 255);

    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10, 190 , 130, 230);

    myGLCD.print("NEXT", 100, 220, 180);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);
    if (covid_touch == 0) {
      while (i < 6) {
        covid_graph[i] = (covid_data[i].toInt() / 100) * 3;
        i++;
      }
      myGLCD.drawLine( 20, 17 , 295 , 17);
      myGLCD.drawLine( 290, 15 , 290 , 190);
      myGLCD.setColor(255, 255, 0);
      myGLCD.print(covid_data[5], 280, 40 + covid_graph[5], 180);
      myGLCD.print(covid_data[4], 235, 40 + covid_graph[4], 180);
      myGLCD.print(covid_data[3], 190, 40 + covid_graph[3], 180);
      myGLCD.print(covid_data[2], 145, 40 + covid_graph[2], 180);
      myGLCD.print(covid_data[1], 100, 40 + covid_graph[1], 180);
      myGLCD.print(covid_data[0], 55, 40 + covid_graph[0], 180);
      myGLCD.setColor(0, 255, 0);
      myGLCD.print("-6D", 280, 15, 180);
      myGLCD.print("-5D", 235, 15, 180);
      myGLCD.print("-4D", 190, 15, 180);
      myGLCD.print("-3D", 145, 15, 180);
      myGLCD.print("-2D", 100, 15, 180);
      myGLCD.print("-1D", 55, 15, 180);
      myGLCD.print("Rate", 300, 205, 180);
      myGLCD.setColor(255, 0, 255);
      myGLCD.setBackColor(255, 255, 0);
      myGLCD.print("ConFirm Num", 260, 210, 180);
      myGLCD.setFont(BigFont);
      myGLCD.print("COVID", 260, 230, 180);


      myGLCD.fillRect( 250, 20, 285 , 20 + covid_graph[5]);
      myGLCD.fillRect( 205, 20, 240 , 20 + covid_graph[4]);
      myGLCD.fillRect( 160, 20, 195 , 20 + covid_graph[3]);
      myGLCD.fillRect( 115, 20, 150 , 20 + covid_graph[2]);
      myGLCD.fillRect( 70, 20, 105 , 20 + covid_graph[1]);
      myGLCD.fillRect( 25, 20, 60 , 20 + covid_graph[0]);
    }
    else if (covid_touch == 1) {
      myGLCD.setFont(BigFont);
      myGLCD.setColor(255, 0, 255);
      myGLCD.setBackColor(255, 255, 0);
      velue = String("Covid");
      myGLCD.print(velue, 260, 230, 180);
      myGLCD.setFont(SmallFont);
      velue = String("ConFirm Rate");
      myGLCD.print(velue, 260, 210, 180);
      myGLCD.setFont(BigFont);
      myGLCD.setColor(255, 180, 255);
      myGLCD.setBackColor(0, 0, 0);
      velue = String(covid_data[6]);
      myGLCD.print(velue, 310, 175 - 20, 180);
      velue = String(covid_data[7]);
      myGLCD.print(velue, 310, 130 - 20, 180);
      velue = String(covid_data[8]);
      myGLCD.print(velue, 310, 85 - 20, 180);

      myGLCD.setFont(SmallFont);

      myGLCD.setColor(255, 70, 255);
      velue = String("ACC_EXAM : ");
      myGLCD.print(velue, 310, 190 - 20, 180);

      velue = String("ACC_EXAM_ACC : ");
      myGLCD.print(velue, 310, 145 - 20, 180);

      velue = String("ACC_RATE : ");
      myGLCD.print(velue, 310, 100 - 20, 180);

      myGLCD.setFont(SmallFont);
    }
    per_covid_touch = covid_touch;
  }
}
void rate_graph() {
  int i = 0;
  int rate_graph[5];
  if (per_rate_touch != rate_touch) {
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(0, 0, 319, 239);
    myGLCD.setColor(0, 0, 255);

    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setFont(BigFont);
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (180, 10, 310, 40);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (180, 10, 310, 40);
    myGLCD.print("NEXT", 210, 15, 0);

    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);

    myGLCD.drawLine( 20, 223 , 295 , 223);
    myGLCD.drawLine( 25, 223 , 25 , 50 );
    if (rate_touch == 0) { // 달러
      while (i < 5) {
        rate_graph[i] = (am_rate_data[i].toInt() - 1100) * 2;
        i++;
      }

      myGLCD.setColor(255, 0, 255);
      myGLCD.print("MON", 25 + 27 - 5, 225, 0);
      myGLCD.print("TUE", 25 + 81 - 5, 225, 0);
      myGLCD.print("WED", 25 + 135 - 5, 225, 0);
      myGLCD.print("THU", 25 + 189 - 5, 225, 0);
      myGLCD.print("FRI", 25 + 243 - 5, 225, 0);
      myGLCD.setColor(255, 255, 0);
      myGLCD.print(am_rate_data[0], 25 + 27 - 20, 20 + rate_graph[0], 0);
      myGLCD.print(am_rate_data[1], 25 + 81 - 20, 20 + rate_graph[1], 0);
      myGLCD.print(am_rate_data[2], 25 + 135 - 20, 20 + rate_graph[2], 0);
      myGLCD.print(am_rate_data[3], 25 + 189 - 20, 20 + rate_graph[3], 0);
      myGLCD.print(am_rate_data[4], 25 + 243 - 20, 20 + rate_graph[4], 0);
      myGLCD.setFont(BigFont);
      myGLCD.setBackColor(255, 0, 255);
      myGLCD.print("LAST WEEK", 20, 10, 0);
      myGLCD.print("1 DOLLAR", 20, 30, 0);
      myGLCD.setColor(255, 0, 255);
      myGLCD.fillCircle(25 + 27, rate_graph[0], 3);
      myGLCD.fillCircle(25 + 81, rate_graph[1], 3);
      myGLCD.fillCircle(25 + 135, rate_graph[2], 3);
      myGLCD.fillCircle(25 + 189, rate_graph[3], 3);
      myGLCD.fillCircle(25 + 243, rate_graph[4], 3);
      myGLCD.drawLine(25 + 27, rate_graph[0], 25 + 81, rate_graph[1]);
      myGLCD.drawLine(25 + 81, rate_graph[1], 25 + 135, rate_graph[2]);
      myGLCD.drawLine(25 + 135, rate_graph[2], 25 + 189, rate_graph[3]);
      myGLCD.drawLine(25 + 189, rate_graph[3], 25 + 243, rate_graph[4]);
    }

    else if (rate_touch == 1) { // 엔화
      while (i < 5) {
        rate_graph[i] = (jp_rate_data[i].toInt() - 1000) * 3;
        i++;
      }

      myGLCD.setColor(255, 0, 255);
      myGLCD.print("MON", 25 + 27 - 5, 225, 0);
      myGLCD.print("TUE", 25 + 81 - 5, 225, 0);
      myGLCD.print("WED", 25 + 135 - 5, 225, 0);
      myGLCD.print("THU", 25 + 189 - 5, 225, 0);
      myGLCD.print("FRI", 25 + 243 - 5, 225, 0);
      myGLCD.setColor(255, 255, 0);
      myGLCD.print(jp_rate_data[0], 25 + 27 - 20, 20 + rate_graph[0], 0);
      myGLCD.print(jp_rate_data[1], 25 + 81 - 20, 20 + rate_graph[1], 0);
      myGLCD.print(jp_rate_data[2], 25 + 135 - 20, 20 + rate_graph[2], 0);
      myGLCD.print(jp_rate_data[3], 25 + 189 - 20, 20 + rate_graph[3], 0);
      myGLCD.print(jp_rate_data[4], 25 + 243 - 20, 20 + rate_graph[4], 0);
      myGLCD.setFont(BigFont);
      myGLCD.setBackColor(255, 0, 255);
      myGLCD.print("LAST WEEK", 20, 10, 0);
      myGLCD.print("100 YEN", 20, 30, 0);
      myGLCD.setColor(255, 0, 255);
      myGLCD.setColor(255, 0, 255);
      myGLCD.fillCircle(25 + 27, rate_graph[0], 3);
      myGLCD.fillCircle(25 + 81, rate_graph[1], 3);
      myGLCD.fillCircle(25 + 135, rate_graph[2], 3);
      myGLCD.fillCircle(25 + 189, rate_graph[3], 3);
      myGLCD.fillCircle(25 + 243, rate_graph[4], 3);
      myGLCD.drawLine(25 + 27, rate_graph[0], 25 + 81, rate_graph[1]);
      myGLCD.drawLine(25 + 81, rate_graph[1], 25 + 135, rate_graph[2]);
      myGLCD.drawLine(25 + 135, rate_graph[2], 25 + 189, rate_graph[3]);
      myGLCD.drawLine(25 + 189, rate_graph[3], 25 + 243, rate_graph[4]);
    }

    else if (rate_touch == 2) { // 유로
      while (i < 5) {
        rate_graph[i] = (eu_rate_data[i].toInt() - 1300) * 3;
        i++;
      }
      myGLCD.setColor(255, 0, 255);
      myGLCD.print("MON", 25 + 27 - 5, 225, 0);
      myGLCD.print("TUE", 25 + 81 - 5, 225, 0);
      myGLCD.print("WED", 25 + 135 - 5, 225, 0);
      myGLCD.print("THU", 25 + 189 - 5, 225, 0);
      myGLCD.print("FRI", 25 + 243 - 5, 225, 0);
      myGLCD.setColor(255, 255, 0);
      myGLCD.print(eu_rate_data[0], 25 + 27 - 20, 20 + rate_graph[0], 0);
      myGLCD.print(eu_rate_data[1], 25 + 81 - 20, 20 + rate_graph[1], 0);
      myGLCD.print(eu_rate_data[2], 25 + 135 - 20, 20 + rate_graph[2], 0);
      myGLCD.print(eu_rate_data[3], 25 + 189 - 20, 20 + rate_graph[3], 0);
      myGLCD.print(eu_rate_data[4], 25 + 243 - 20, 20 + rate_graph[4], 0);
      myGLCD.setBackColor(255, 0, 255);
      myGLCD.setFont(BigFont);
      myGLCD.print("LAST WEEK", 20, 10, 0);
      myGLCD.print("1 EURO", 20, 30, 0);
      myGLCD.setColor(255, 0, 255);
      myGLCD.setColor(255, 0, 255);
      myGLCD.fillCircle(25 + 27, rate_graph[0], 3);
      myGLCD.fillCircle(25 + 81, rate_graph[1], 3);
      myGLCD.fillCircle(25 + 135, rate_graph[2], 3);
      myGLCD.fillCircle(25 + 189, rate_graph[3], 3);
      myGLCD.fillCircle(25 + 243, rate_graph[4], 3);
      myGLCD.drawLine(25 + 27, rate_graph[0], 25 + 81, rate_graph[1]);
      myGLCD.drawLine(25 + 81, rate_graph[1], 25 + 135, rate_graph[2]);
      myGLCD.drawLine(25 + 135, rate_graph[2], 25 + 189, rate_graph[3]);
      myGLCD.drawLine(25 + 189, rate_graph[3], 25 + 243, rate_graph[4]);
    }

    else if (rate_touch == 3) { // 위안
      while (i < 5) {
        rate_graph[i] = (ch_rate_data[i].toInt() - 180) * 20;
        i++;
      }
      myGLCD.setColor(255, 0, 255);
      myGLCD.print("MON", 25 + 27 - 5, 225, 0);
      myGLCD.print("TUE", 25 + 81 - 5, 225, 0);
      myGLCD.print("WED", 25 + 135 - 5, 225, 0);
      myGLCD.print("THU", 25 + 189 - 5, 225, 0);
      myGLCD.print("FRI", 25 + 243 - 5, 225, 0);
      myGLCD.setColor(255, 255, 0);
      myGLCD.print(ch_rate_data[0], 25 + 27 - 20, 20 + rate_graph[0], 0);
      myGLCD.print(ch_rate_data[1], 25 + 81 - 20, 20 + rate_graph[1], 0);
      myGLCD.print(ch_rate_data[2], 25 + 135 - 20, 20 + rate_graph[2], 0);
      myGLCD.print(ch_rate_data[3], 25 + 189 - 20, 20 + rate_graph[3], 0);
      myGLCD.print(ch_rate_data[4], 25 + 243 - 20, 20 + rate_graph[4], 0);
      myGLCD.setBackColor(255, 0, 255);
      myGLCD.setFont(BigFont);
      myGLCD.print("LAST WEEK", 20, 10, 0);
      myGLCD.print("1 YUAN", 20, 30, 0);
      myGLCD.setColor(255, 0, 255);
      myGLCD.setColor(255, 0, 255);
      myGLCD.fillCircle(25 + 27, rate_graph[0], 3);
      myGLCD.fillCircle(25 + 81, rate_graph[1], 3);
      myGLCD.fillCircle(25 + 135, rate_graph[2], 3);
      myGLCD.fillCircle(25 + 189, rate_graph[3], 3);
      myGLCD.fillCircle(25 + 243, rate_graph[4], 3);
      myGLCD.drawLine(25 + 27, rate_graph[0], 25 + 81, rate_graph[1]);
      myGLCD.drawLine(25 + 81, rate_graph[1], 25 + 135, rate_graph[2]);
      myGLCD.drawLine(25 + 135, rate_graph[2], 25 + 189, rate_graph[3]);
      myGLCD.drawLine(25 + 189, rate_graph[3], 25 + 243, rate_graph[4]);
    }
    per_rate_touch = rate_touch;
  }
}
void air_quality_circle(int data, int x, int y) {
  if (air_data[data] == "Low") {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillCircle(x, y, 7);
  }

  else if (air_data[data] == "Normal") {
    myGLCD.setColor(255, 255, 0);
    myGLCD.fillCircle(x, y, 7);
  }

  else if (air_data[data] == "High") {
    myGLCD.setColor(255, 0, 0);
    myGLCD.fillCircle(x, y, 7);
  }
}
