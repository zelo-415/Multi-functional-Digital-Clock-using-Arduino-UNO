/************包含的头文件**************/
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <DS1302.h>
#include <stdio.h>
/* * 
 * LCD RS pin to digital pin 12 
 * LCD Enable pin to digital pin 11 
 * LCD D4 pin to digital pin 5 
 * LCD D5 pin to digital pin 4 
 * LCD D6 pin to digital pin 3 
 * LCD D7 pin to digital pin 2 
 * LCD R/W pin to ground 
 * LCD VSS pin to ground 
 * LCD VCC pin to 5V 
 * */ 
 
 /*指定LCD1602和Arduino之间的引脚关系*/
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);    

/*指定按键和蜂鸣器与Arduino之间的引脚关系*/
#define choose A0   //选择端口
#define add A1      //加
#define minus A2    //减
#define Tone 13     //蜂鸣器端口
#define DELAY 50

/*指定DS1302和Arduino之间的引脚关系*/
uint8_t CE_PIN = 8;           //DS1302 RST 端口
uint8_t IO_PIN = 9;           //DS1302 DAT 端口
uint8_t SCLK_PIN = 10;        //DS1302 CLK 端口
uint16_t Time_Flag = DELAY;

DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN); //创建 DS1302 对象

/*时间数据*/
unsigned long seconds; 
int s = 0, m = 0, h = 0, d = 0, mon = 0, y = 0;                       //时间进位
int second = 0, minute = 0, hour = 0, day = 0, month = 0, year = 0;   //当前时间
int SECOND = 0, MINUTE = 0, HOUR = 0, DAY = 0, MONTH = 0, YEAR = 0;   //初始时间
int chose = 0, alarm_choose = 0 ,ButtonDelay = 10, frequence = 2093;  //消抖时间，蜂鸣器频率
int frequence1 = 2000;
int LongButtonDelay = 1000;
int alarm_hour = 7, alarm_minute = 30, alarm_second = 0;              //闹钟时间
float Temperatures, Temp_Alarm = 30 ;                                 //当前温度、报警温度：30摄氏度
bool LongPress = false;
bool Clock_Arrive  = false;                                              //到达闹钟时间后此变量置true，直至用户按下按钮才可以变回false
bool set_flag = false;                                                                      //当Clock_Arrive为true时，蜂鸣器一直响
bool set_end_flag = false;
/*超过指定温度报警*/
void Temperatures_Alarm()
{
  if (Temperatures >= Temp_Alarm)
  {
    tone(Tone,frequence);           // 语法：tone(pin, frequency, duration)；注：产生50%占空比的特定频率的方波信号
    delay(500);
    noTone(Tone);  
  }  
}

/** 获取 LM35 温度 并且 在LCD1602上显示*/ 
void GetTemperatures()
{ 
 if (Time_Flag == DELAY)                  //每执行50次，测一次温度
 {
   long a = analogRead(A3);               // Get temperatures ，返回值在0-1024之间
   // Temperatures = 500.0*a/1023.0; 
   Temperatures = 0.0048828125*a*100; 
   Time_Flag = 0;
 }
 Time_Flag ++;
 lcd.setCursor(9, 1) ; 
 lcd.print(Temperatures); //获取温度
 lcd.setCursor(14, 1); 
 lcd.print((char)223); //显示 o 符号
 lcd.setCursor(15, 1); 
 lcd.print("C"); //显示字母 C 
}

/** 闹钟 指定时间蜂鸣 */ 
/*improvement:采用现实闹钟的设计，一直蜂鸣到用户按下按钮*/
void Clock_Alarm()
{ 
 if(hour == alarm_hour && minute == alarm_minute && second == alarm_second)
 { 
   tone(Tone,frequence); 
   delay(50); 
   noTone(Tone); 
 } 
}

/** 正点蜂鸣 */ 
void Point_Time_Alarm()
{ 
 if(minute == 0 && second == 0)
 { 
   tone(Tone,frequence1); 
   delay(1000); 
   noTone(Tone); 
 } 
}

/** 进入报警设置 */ 
/*improment:长按退出*/
void Set_Alarm()
{ 
 if(digitalRead(add) == LOW && digitalRead(minus) == LOW)  /*同时按下+-键*/
 { 
   alarm_hour = hour; 
   alarm_minute = minute; 
   //alarm_choose = 1; 
   lcd.setCursor(0, 0); 
   lcd.print("set alarm "); 
   lcd.setCursor(6, 1); 
   lcd.print("00"); //闹钟秒数 
   while(1)
   { 
     if(digitalRead(choose) == LOW)       /*软件消抖*/
     { 
       delay(ButtonDelay); 
       if(digitalRead(choose) == LOW)
       { 
         alarm_choose++; 
       } 
     } 
     /*显示报警温度*/
     lcd.setCursor(9, 1); 
     lcd.print(Temp_Alarm); 
     lcd.setCursor(14, 1); 
     lcd.print((char)223); //显示 o 符号
     lcd.setCursor(15, 1); 
     lcd.print("C"); //显示字母 C 
     if(alarm_choose == 1)
     { 
       Set_Alarm_Hour(); 
     }
     else if(alarm_choose == 2)
     { 
       Set_Alarm_Minute(); 
     }
     else if(alarm_choose == 3)
     {
       Set_Alarm_Temp(); 
     }
     else if(alarm_choose >= 4)
     { 
       alarm_choose = 0; 
       break; 
     } 
   } 
 } 
}

/*设置报警温度*/
void Set_Alarm_Temp()
{
  DisplayCursor(10, 1); 
  if(digitalRead(add) == LOW) 
  { 
    delay(ButtonDelay); 
    if(digitalRead(add) == LOW)
    { 
      Temp_Alarm ++; 
    } 
  } 
  if(digitalRead(minus) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(minus) == LOW)
    { 
      Temp_Alarm --; 
    } 
  }
}

/** 设置闹钟分钟 */ 
void Set_Alarm_Minute()
{ 
  DisplayCursor(4, 1); 
  if(digitalRead(add) == LOW) 
  { 
    delay(ButtonDelay); 
    if(digitalRead(add) == LOW)
    { 
      alarm_minute ++; 
      if(alarm_minute == 60)
      { 
        alarm_minute = 0; 
      } 
      FormatDisplay(3,1,alarm_minute); 
    } 
    FormatDisplay(3,1,alarm_minute); 
  } 
  if(digitalRead(minus) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(minus) == LOW)
    { 
      alarm_minute --; 
      if(alarm_minute == -1)
      { 
        alarm_minute = 59; 
      } 
      FormatDisplay(3,1,alarm_minute); 
    } 
  } 
}

/** 设置闹钟小时 */ 
void Set_Alarm_Hour()
{ 
  DisplayCursor(1, 1); 
  if(digitalRead(add) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(add) == LOW)
    { 
      alarm_hour ++; 
      if(alarm_hour == 24)
      {
        alarm_hour = 0; 
      } 
    FormatDisplay(0,1,alarm_hour); 
    } 
  } 
  if(digitalRead(minus) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(minus) == LOW)
    { 
      alarm_hour --; 
      if(alarm_hour == -1)
      { 
        alarm_hour = 23; 
      } 
      FormatDisplay(0,1,alarm_hour); 
    } 
  } 
}

/** 按键选择 */ 
void Set_Clock()
{ 
  if(digitalRead(choose)==LOW)
  { 
    set_end_flag = false;
    delay(200);
    /*
    lcd.setCursor(9, 1); 
    lcd.print("SetTime"); 
    while(1)
    { 
      if(digitalRead(choose) == LOW)
      { 
        delay(ButtonDelay); 
        { 
          chose++; 
        }
      } 
      seconds = millis()/1000; 
      Display(); 
      if(chose == 1)
      { 
        Set_Time(1, 1, HOUR); //SetHour 
      }
      else if(chose == 2)
      { 
        Set_Time(4, 1, MINUTE); //SetMinute 
      }
      else if(chose == 3)
      { 
        Set_Time(7, 1, SECOND); //SetSecond 
      }
      else if(chose == 4)
      { 
        Set_Time(9, 0, DAY); //SetDay 
      }
      else if(chose == 5)
      { 
        Set_Time(6, 0, MONTH); // SetMonth 
      }
      else if(chose == 6)
      { 
        Set_Time(3, 0, YEAR); //SetYear 
      }
      else if(chose >= 7) 
      { 
        chose = 0; 
        delay(100);
        break; 
      } 
    } 
    */
    while(1 && !set_end_flag)
    {
      if (! set_flag)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Please select");
        lcd.setCursor(0,1);
        lcd.print("-:time");
        lcd.setCursor(9,1);
        lcd.print("+:alarm"); 
        delay(100);
      }
      if (digitalRead(choose) == LOW)
      {
        lcd.clear();
        delay(400);
        break;
      }
      if (digitalRead(minus) == LOW || digitalRead(add) == LOW) set_flag = true;
      if(digitalRead(minus) == LOW && set_flag)
      { 
        while (1)
        {
          if(digitalRead(choose) == LOW)
          { 
            delay(ButtonDelay); 
              chose++; 
          } 
          lcd.clear();
          lcd.setCursor(9, 1); 
          lcd.print("SetTime");
          seconds = millis()/1000; 
          Display(); 
          if(chose == 1)
          { 
            Set_Time(1, 1, HOUR); //SetHour 
          }
          else if(chose == 2)
          { 
            Set_Time(4, 1, MINUTE); //SetMinute 
          }
          else if(chose == 3)
          { 
            Set_Time(7, 1, SECOND); //SetSecond 
          }
          else if(chose == 4)
          { 
            Set_Time(9, 0, DAY); //SetDay 
          }
          else if(chose == 5)
          { 
            Set_Time(6, 0, MONTH); // SetMonth 
          }
          else if(chose == 6)
          { 
            Set_Time(3, 0, YEAR); //SetYear 
          }
          else if(chose >= 7) 
          { 
            chose = 0; 
            
            set_flag = false;
            set_end_flag = true;
            delay(100);
            break; 
          }
          delay(100); 
        }
      } 
      if(digitalRead(add) == LOW && set_flag)
      {
        alarm_hour = hour; 
        alarm_minute = minute;
        while (1)
        {
           lcd.clear(); 
           //alarm_choose = 1; 
           lcd.setCursor(0, 0); 
           lcd.print("set alarm "); 
           FormatDisplay(0,1,alarm_hour);
           lcd.setCursor(2,1);
           lcd.print(":");
           FormatDisplay(3,1,alarm_minute);
           lcd.setCursor(5,1);
           lcd.print(":");
           lcd.setCursor(6, 1); 
           lcd.print("00"); //闹钟秒数 
           if(digitalRead(choose) == LOW)       /*软件消抖*/
           { 
             delay(ButtonDelay); 
             if(digitalRead(choose) == LOW)
             { 
              Serial.println(alarm_choose);
               alarm_choose++; 
             } 
           } 
           /*显示报警温度*/
           lcd.setCursor(9, 1); 
           lcd.print(Temp_Alarm); 
           lcd.setCursor(14, 1); 
           lcd.print((char)223); //显示 o 符号
           lcd.setCursor(15, 1); 
           lcd.print("C"); //显示字母 C 
           if(alarm_choose == 1)
           { 
             Set_Alarm_Hour(); 
           }
           else if(alarm_choose == 2)
           { 
             Set_Alarm_Minute(); 
           }
           else if(alarm_choose == 3)
           {
             Set_Alarm_Temp(); 
           }
           else if(alarm_choose >= 4)
           { 
             alarm_choose = 0; 
             set_flag = false;
             set_end_flag = true;
             delay(100);
             break; 
           }
           delay(100);
        }
      }
    }
  } 
}

/** 通过按键设置时间 */ 
void Set_Time(int rol, int row, int &Time)
{ 
  DisplayCursor(rol, row); 
  if(digitalRead(add) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(add) == LOW)
    { 
      Time ++; 
    } 
    Display(); 
  } 
  if(digitalRead(minus) == LOW)
  { 
    delay(ButtonDelay); 
    if(digitalRead(minus) == LOW)
    { 
      Time --; 
    } 
    Display(); 
  }  
}

/** 显示时间、日期、星期 */ 
void Display() 
{ 
  time(); 
  Day(); 
  Month(); 
  Year(); 
  Week(year,month,day); 
} 
/** 显示光标 */ 
void DisplayCursor(int rol, int row) 
{ 
  lcd.setCursor(rol, row); 
  /*光标闪烁*/
  lcd.cursor(); 
  delay(100); 
  lcd.noCursor(); 
  delay(100); 
} 
/** 设置初始时间 */ 
void set(int y, int mon, int d, int h, int m, int s)
{ 
  YEAR = y; 
  MONTH = mon; 
  DAY = d; 
  HOUR = h; 
  MINUTE = m; 
  SECOND = s; 
}

/** 根据年月日计算星期几 */ 
void Week(int y,int m, int d)
{ 
  if(m == 1)
  { 
    m = 13; 
  } 
  if(m == 2)
  { 
    m = 14; 
  } 
  int week = (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7+1; 
  String weekstr = ""; 
  switch(week)
  { 
    case 1: weekstr = "Mon. "; break; 
    case 2: weekstr = "Tues. "; break; 
    case 3: weekstr = "Wed. "; break; 
    case 4: weekstr = "Thur. "; break; 
    case 5: weekstr = "Fri. "; break; 
    case 6: weekstr = "Sat. "; break; 
    case 7: weekstr = "Sun. "; break; 
  } 
  lcd.setCursor(11, 0); 
  lcd.print(weekstr); 
}

/** 计算年份 */ 
void Year()
{ 
  year = ( YEAR + y ) % 9999; 
  if(year == 0)
  { 
    year = 9999; 
  } 
  lcd.setCursor(0, 0); 
  if(year < 1000)
  { 
    lcd.print("0"); 
  } 
  if(year < 100)
  { 
    lcd.print("0"); 
  } 
  if(year < 10)
  { 
    lcd.print("0"); 
  } 
  lcd.print(year); 
  lcd.setCursor(4, 0); 
  lcd.print('-'); 
}

/** 计算月份 */ 
void Month()
{ 
  month = (MONTH + mon) % 12; 
  if(month == 0)
  { 
    month = 12; 
  } 
  y = (MONTH + mon - 1) / 12; 
  FormatDisplay(5,0,month); 
  lcd.setCursor(7, 0); 
  lcd.print('-'); 
}

/** 计算当月天数 */ 
void Day()
{ 
  int days = Days(year,month); 
  int days_up; 
  if(month == 1)
  { 
    days_up = Days(year - 1, 12); 
  } 
  else
  { 
    days_up = Days(year, month - 1); 
  } 
  day = (DAY + d) % days; 
  if(day == 0)
  { 
    day = days; 
  } 
  if((DAY + d) == days + 1 )
  { 
    DAY -= days; 
    mon++; 
  } 
  if((DAY + d) == 0)
  { 
    DAY += days_up; 
    mon--; 
  } 
  FormatDisplay(8,0,day); 
}

/** 根据年月计算当月天数 */ 
int Days(int year, int month)
{ 
  int days = 0; 
  if (month != 2)
  { 
    switch(month)
    { 
      case 1: case 3: case 5: case 7: case 8: case 10: case 12: days = 31; break; 
      case 4: case 6: case 9: case 11: days = 30; break; 
    } 
  }
  else
  {  
    if(year % 4 == 0 && year % 100 != 0 || year % 400 == 0)   //闰年
    { 
      days = 29; 
    }
    else
    { 
      days = 28; 
    } 
  } 
  return days; 
}

/** 格式化输出 */ 
void FormatDisplay(int col, int row,int num)
{ 
  lcd.setCursor(col, row); 
  if(num < 10) lcd.print("0"); 
  lcd.print(num); 
} 

/** 计算时间 */ 
void time() 
{ 
  second = (SECOND + seconds) % 60; //计算秒
  m = (SECOND + seconds) / 60; //分钟进位
  FormatDisplay(6,1,second); 
  minute = (MINUTE + m) % 60; //计算分钟
  h = (MINUTE + m) / 60; //小时进位
  FormatDisplay(3,1,minute); 
  hour = (HOUR + h) % 24; //计算小时
  d = (HOUR + h) / 24; //天数进位
  FormatDisplay(0,1,hour); 
  lcd.setCursor(2, 1); 
  lcd.print(":"); 
  lcd.setCursor(5, 1); 
  lcd.print(":"); 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  /**********************************************************************/
  /*数字引脚2-13设置为输出模式*/
  for(int i = 2;i <= 13; i++)
  {
    pinMode(i,OUTPUT);
  }
  /*将按键引脚设置为高电平*/
  digitalWrite(add, HIGH);
  digitalWrite(minus, HIGH);
  digitalWrite(choose, HIGH);
  
  lcd.begin(16, 2); //初始化 LCD1602
  Time t;
  /*initial setting of teachers*/
  //t.hour=12;t.min=59;t.sec=59;
  //t.year=2020;t.mon=5;t.date=7;
  /*user can modify the time here for better use. There is an example below*/
  t.hour=22;t.min=56;t.sec=59;
  t.year=2022;t.mon=3;t.date=26;
  /*if you want the time you modified above to be the initial time for DS1302, just enable the operaitons for EEPROM*/
  /*
  EEPROM.write(0,t.year/256);
  EEPROM.write(1,t.year%256);
  EEPROM.write(2,t.mon);
  EEPROM.write(3,t.date);
  EEPROM.write(4,t.hour);
  EEPROM.write(5,t.min);
  EEPROM.write(6,t.sec);
  */
  /**********************操作DS1302芯片***********************/
  rtc.setTime(t.hour,t.min,t.sec); //设置 DS1302 芯片初始时间
  rtc.setDate(t.year,t.mon,t.date);
  /*设置初始时间*/
  u16 temp_year = EEPROM.read(0) *256 + EEPROM.read(1);
  //set(t.year,t.mon,t.date,t.hour,t.min,t.sec);
  set(temp_year,EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(6));
  //set(rtc.getTime().year, rtc.getTime().mon, rtc.getTime().date, rtc.getTime().hour, rtc.getTime().min, rtc.getTime().sec); //通电读取DS1302 芯片的时间
  rtc.writeProtect(false); // 关闭 DS1302 芯片写保护
  rtc.halt(false); //为 true 时 DS1302 暂停
  /***********************************************************/
}

void loop() 
{
 
  Time t; 
  // put your main code here, to run repeatedly:
  seconds = millis()/1000;        //获取单片机当前运行时间        
  Display();                      //显示时间        
  Set_Clock();                    //设置时间        
  Set_Alarm();                    //设置闹钟        
  Point_Time_Alarm();             //正点蜂鸣        
  Clock_Alarm();                  //闹钟时间蜂鸣        
  GetTemperatures();              //获取LM35温度        
  Temperatures_Alarm();           //超过指定温度报警      
  
  /*断电将单片机的当前时间写到DS1302芯片中*/  
  t = rtc.getTime();
  
  rtc.setTime(t.hour,t.min,t.sec);      //设置DS1302芯片初始时间        
  rtc.setDate(t.year,t.mon,t.date); 
  
  //rtc.setTime(hour,minute,second);    //设置DS1302芯片初始时间        
  //rtc.setDate(t.year,t.mon,t.date); 
  /*At every step, put the time data into the EEPROM in case that the MCU is dispowered*/
  EEPROM.write(0,year/256);
  EEPROM.write(1,year%256);
  EEPROM.write(2,month);
  EEPROM.write(3,day);
  EEPROM.write(4,hour);
  EEPROM.write(5,minute);
  EEPROM.write(6,second);
}