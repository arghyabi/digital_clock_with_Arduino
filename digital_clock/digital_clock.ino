#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>

#include "RTClib.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
int RECV_PIN = 3;
int led = 2;
RTC_DS1307 rtc;
IRrecv irrecv(RECV_PIN);

decode_results results;

int pos[] = {4, 5, 6, 7, A0, A1, A2};
int gnd[] = {13, 12, 11, 10, 9, 8};
int loop_controller = 128;
byte numbers[10] = {
  B1111110, B0110000, B1101101, B1111001, B0110011, B1011011, B1011111, B1110000, B1111111, B1111011
};
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int ledstate = 0;
int oldtime = 0;
String olddata = "";
int mode = 0;
int setting = 0;
int new_data = 0;
int sp_msg_num = 0;
enum settings_mode {
  default_mode = 0,
  set_hour,
  set_minute,
  set_second,
  set_date,
  set_month,
  set_year,
  special_mode
};

enum data_mode {
  heading_mode = 0,
  setting_mode,
  saving_mode
};
String password = "";
long int real_pass = 291194;
int special = 0;
long int pass_num = 0;
int srolling_text = 1;
void setup()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  irrecv.enableIRIn();

  Serial.begin(57600);
  rtc.begin();
  pinMode(led, OUTPUT);
  //rtc.adjust(DateTime(2019, 11, 17, 17, 23, 00));

  for (int i = 0; i < 7; i++) {
    pinMode(pos[i], OUTPUT);
  }
  for (int i = 0; i < 6; i++) {
    pinMode(gnd[i], OUTPUT);
  }
}

void loop() {
  display.setTextSize(2);
  int led_state = 0;
  String oled_data;
  long int ir_data = 0;
  unsigned long newtime;
  if (irrecv.decode(&results)) {
    ir_data = results.value;
    if (ir_data == 33446055) {//mode button
      led_state = 1;
      setting = 0;
      if (mode == special_mode)
        mode = 0;
      else
        mode++;
    }
    if (ir_data == 33456255) {//select button
      led_state = 1;
      if (setting == saving_mode)
        setting = 0;
      else
        setting++;
    }
    if (ir_data == 33431775) { //come to home
      led_state = 1;
      mode = 0;
      setting = 0;
      special = 0;
    }
    int num = -1;
    if (ir_data == 33444015) {
      led_state = 1;
      num = 1;
    }
    if (ir_data == 33478695) {
      led_state = 1;
      num = 2;
    }
    if (ir_data == 33486855) {
      led_state = 1;
      num = 3;
    }
    if (ir_data == 33435855) {
      led_state = 1;
      num = 4;
    }
    if (ir_data == 33468495) {
      led_state = 1;
      num = 5;
    }
    if (ir_data == 33452175) {
      led_state = 1;
      num = 6;
    }
    if (ir_data == 33423615) {
      led_state = 1;
      num = 7;
    }
    if (ir_data == 33484815) {
      led_state = 1;
      num = 8;
    }
    if (ir_data == 33462375) {
      led_state = 1;
      num = 9;
    }
    if (ir_data == 33480735) {
      led_state = 1;
      num = 0;
    }

    if (num != -1 && special == 0) {
      if (new_data > 9)
        new_data = (new_data % 10) * 10 + num;
      else
        new_data = new_data * 10 + num;
    }
    if (num != -1 && special == 1) {
      pass_num = pass_num * 10 + num;
      password = password + String(num);
      newtime = millis();
    }
    //Serial.println(results.value);
    irrecv.resume();
  }
  digitalWrite(led, led_state);
  DateTime now = rtc.now();

  long num = 0;
  int Thour = now.hour();
  int Tminute = now.minute();
  int Tsecond = now.second();
  String Tday = daysOfTheWeek[now.dayOfTheWeek()];
  int Tdate = now.day();
  int Tmonth = now.month();
  int Tyear = now.year();

  num = Thour;
  num = (num * 100) + Tminute;
  num = (num * 100) + Tsecond;
  if (srolling_text)
    writeFullnumber(num);
  srolling_text = 1;
  /*
    if (Tsecond != oldtime) {
    oldtime = Tsecond;
    digitalWrite(8, ledstate);
    ledstate = ~ledstate;
    }
  */
  switch (mode) {
    case default_mode:
      special = 0;
      oled_data = (String)Tdate + "-" + (String)Tmonth + "-" + (String)Tyear + Tday;
      break;
    case set_hour:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set      Hour";
          new_data = Thour;
          break;
        case setting_mode:
          oled_data = "Old: " + format_data(Thour) + "   New: " + format_data(new_data);
          break;
        case saving_mode:
          if (new_data < 24) {
            rtc.adjust(DateTime(Tyear, Tmonth, Tdate, new_data, Tminute, Tsecond));
            oled_data = "Done";
          }
          else
            oled_data = "Input out of range";
          break;
      }
      break;
    case set_minute:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set     Minute";
          new_data = Tminute;
          break;
        case setting_mode:
          oled_data = "Old: " + format_data(Tminute) + "   New: " + format_data(new_data);
          break;
        case saving_mode:
          if (new_data < 60) {
            rtc.adjust(DateTime(Tyear, Tmonth, Tdate, Thour, new_data, Tsecond));
            oled_data = "Done";
          }
          else
            oled_data = "Input out of range";
          break;
      }
      break;
    case set_second:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set     Second";
          new_data = Tsecond;
          break;
        case setting_mode:
          oled_data = "Old: " + format_data(Tsecond) + "   New: " + format_data(new_data);
          break;
        case saving_mode:
          if (new_data < 60) {
            rtc.adjust(DateTime(Tyear, Tmonth, Tdate, Thour, Tminute, new_data));
            oled_data = "Done";
          }
          else
            oled_data = "Input out of range";
          break;
      }
      break;
    case set_date:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set      Date";
          new_data = Tdate;
          break;
        case setting_mode:
          oled_data = "Old: " + format_data(Tdate) + "   New: " + format_data(new_data);
          break;
        case saving_mode:
          if (new_data < 32 && new_data > 0) {
            rtc.adjust(DateTime(Tyear, Tmonth, new_data, Thour, Tminute, Tsecond));
            oled_data = "Done";
          }
          else if (new_data > 31)
            oled_data = "Input out of range";
          break;
      }
      break;
    case set_month:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set      Month";
          new_data = Tmonth;
          break;
        case setting_mode:
          oled_data = "Old: " + format_data(Tmonth) + "   New: " + format_data(new_data);
          break;
        case saving_mode:
          if (new_data < 13 && new_data > 0) {
            rtc.adjust(DateTime(Tyear, new_data, Tdate, Thour, Tminute, Tsecond));
            oled_data = "Done";
          }
          else if (new_data > 12)
            oled_data = "Input out of range";
          break;
      }
      break;
    case set_year:
      switch (setting) {
        case heading_mode:
          oled_data = "    Set      Year";
          new_data = Tyear % 100;
          break;
        case setting_mode:
          oled_data = "Old: 20" + format_data(Tyear) + " New: 20" + format_data(new_data);
          break;
        case saving_mode:
          rtc.adjust(DateTime(2000 + new_data, Tmonth, Tdate, Thour, Tminute, Tsecond));
          oled_data = "Done";
          break;
      }
      break;
    case special_mode:
      switch (setting) {
        case heading_mode:
          pass_num = 0;
          oled_data = "  Special   Message";
          break;
        case setting_mode:
          special = 1;
          if (millis() - newtime > 500) {
            for (int i = 0; i < password.length(); i++)
              password[i] = '*';
          }
          oled_data = "Password: " + password;
          break;
        case saving_mode:
          if (pass_num == real_pass) {
            srolling_text = 0;
            if (srolling_text == 0)
            {
              PORTB = B11111111;
              PORTD = (PORTD & B00001111);
              PORTC = (PORTC & B11111000);
            }
            
            char *data_msg = "Wish you a very very happy Anniversary";
            if (sp_msg_num == 1)
              data_msg = "Also best wishes & loves for Happy Birthday";

            int x = 0;
            while (data_msg[x] != '\0')
              x++;
            oled_data = "";
            int s = (x * 19) * (-1);
            display.setTextSize(3);
            if (loop_controller > s) {
              display.clearDisplay();
              display.setCursor(loop_controller, 8);
              display.println(data_msg);
              display.display();
              loop_controller--;
            }
            else {
              loop_controller = 128;

              if (sp_msg_num == 1)
                sp_msg_num = 0;
              else
                sp_msg_num++;
            }
            
            password = "";
          }
          else {
            oled_data = "Wrong     Password";
            password = "";
            pass_num = 0;
          }
          break;
      }
      break;
  }
  if (olddata != oled_data) {
    olddata = oled_data;
    writeOLED(oled_data, 0, 0);
  }
}

void writeOLED(String data, int locC, int locR) {
  display.clearDisplay();
  //display.setTextSize(2);
  display.setCursor(locC, locR);
  display.print(data);
  display.display();
}

void writeDigit(int num, int index) {
  byte data = numbers[num];
  PORTB = B11111111 ^ (B00100000 >> index);
  PORTD = (PORTD & B00001111) | ((data << 1) & B11110000);
  PORTC = (PORTC & B11111000) | (data & B00000111);
}

void writeFullnumber(long num) {
  int data[] = {0, 0, 0, 0, 0, 0};
  int i = 5;
  while (num > 0) {
    data[i--] = num % 10;
    num = num / 10;
  }
  for (i = 5; i > -1; i--) {
    writeDigit(data[i], i);
    delayMicroseconds(1500);
  }
}

String format_data(int data) {
  if (data > 99)
    data = data % 100;
  if (data > -1 && data < 10)
    return "0" + (String)data;
  else
    return (String)data;
}
