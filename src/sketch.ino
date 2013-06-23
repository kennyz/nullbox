
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS1302.h>
#include <Time.h>
#include <DateTime.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#define DHT11_PIN 0
char sTemp1[20] = "";
char sTemp2[20] = "";
int tLoopIndex = 0;

// RTC clock
/* 接口定义
CE(DS1302 pin5) -> Arduino D5
IO(DS1302 pin6) -> Arduino D6
SCLK(DS1302 pin7) -> Arduino D7
*/
uint8_t CE_PIN   = 5;
uint8_t IO_PIN   = 6;
uint8_t SCLK_PIN = 7;

/* 日期变量缓存 */
char time_buf[50];
char day_buf[10];
/* 串口数据缓存 */
String comdata = "";
int numdata[7] ={0}, j = 0, mark = 0;
/* 创建 DS1302 对象 */
DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN);


int buzzer=8;//设置控制蜂鸣器的数字IO脚
int pin_sound = 1;
int dpin_light = 2;
int dpin_red = 4;


void setup()
{
  lcd.init();                      // initialize the lcd 
 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.print("Kongwu.NET");
  delay(1000);
  
  //init temp sensor
  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);	
  Serial.begin(19200);  
  Serial.println("Ready");
  
  rtc.write_protect(false);
  rtc.halt(false);
  
  //read_time_from_serial();
  //Time t(2013, 6, 10, 4, 39, 46, 3);
  /* Set the time and date on the chip */
  //rtc.time(t);
  
  pinMode(buzzer,OUTPUT);//设置数字IO脚模式，OUTPUT为输出
}

byte read_dht11_dat()
{
	byte i = 0;
	byte result=0;
	for(i=0; i< 8; i++){
		
		
	     while(!(PINC & _BV(DHT11_PIN)));  // wait for 50us
	     delayMicroseconds(30);
		
	     if(PINC & _BV(DHT11_PIN)) 
	     result |=(1<<(7-i));
             while((PINC & _BV(DHT11_PIN)));  // wait '1' finish
         
		
	}
	return result;
}


void print_time()
{
    /* 从 DS1302 获取当前时间 */
    Time t = rtc.time();
    /* 将星期从数字转换为名称 */
    memset(day_buf, 0, sizeof(day_buf));
    switch (t.day)
    {
    case 1: strcpy(day_buf, "Sunday"); break;
    case 2: strcpy(day_buf, "Monday"); break;
    case 3: strcpy(day_buf, "Tuesday"); break;
    case 4: strcpy(day_buf, "Wednesday"); break;
    case 5: strcpy(day_buf, "Thursday"); break;
    case 6: strcpy(day_buf, "Friday"); break;
    case 7: strcpy(day_buf, "Saturday"); break;
    }
    /* 将日期代码格式化凑成buf等待输出 */
    //snprintf(time_buf, sizeof(time_buf), "%s %04d-%02d-%02d %02d:%02d:%02d", day_buf, t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    //snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d", t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    snprintf(time_buf, sizeof(time_buf), "%02d-%02d %02d:%02d:%02d", t.mon, t.date, t.hr, t.min, t.sec);
    /* 输出日期到串口 */
    //Serial.println(time_buf);
    
    if(t.sec==0)
    {
      digitalWrite(buzzer,HIGH);//发声音
      delay(1);//延时1ms
      digitalWrite(buzzer,LOW);//bu发声音
    }

    
  lcd.setCursor(0, 1);
  lcd.print(time_buf);    
}

void read_time_from_serial()
{
    /* 当串口有数据的时候，将数据拼接到变量comdata */
    //example: 2012,5,1,23,55,23,2,
    while (Serial.available() > 0)
    {
        comdata += char(Serial.read());
        delay(2);
        mark = 1;
    }
    /* 以逗号分隔分解comdata的字符串，分解结果变成转换成数字到numdata[]数组 */
    if(mark == 1)
    {
        Serial.print("You inputed : ");
        Serial.println(comdata);
        for(int i = 0; i < comdata.length() ; i++)
        {
            if(comdata[i] == ',' || comdata[i] == 0x10 || comdata[i] == 0x13)
            {
                j++;
            }
            else
            {
                numdata[j] = numdata[j] * 10 + (comdata[i] - '0');
            }
        }
        /* 将转换好的numdata凑成时间格式，写入DS1302 */
        Time t(numdata[0], numdata[1], numdata[2], numdata[3], numdata[4], numdata[5], numdata[6]);
        rtc.time(t);
        mark = 0;j=0;
        
        /* 打印当前时间 */
        print_time();
  
        
        /* 清空 comdata 变量，以便等待下一次输入 */
        comdata = String("");
        /* 清空 numdata */
        for(int i = 0; i < 7 ; i++) numdata[i]=0;
    
      //delay(1000);  
    }    
}

void check_temp_sensor()
{
	byte dht11_dat[5];
	byte dht11_in;
	byte i;
	// start condition
	// 1. pull-down i/o pin from 18ms
	PORTC &= ~_BV(DHT11_PIN);
	delay(18);
	PORTC |= _BV(DHT11_PIN);
	delayMicroseconds(40);
	
	DDRC &= ~_BV(DHT11_PIN);
	delayMicroseconds(40);
	
	dht11_in = PINC & _BV(DHT11_PIN);
	
	if(dht11_in){
		Serial.println("dht11 start condition 1 not met");
		return;
	}
	delayMicroseconds(80);
	
	dht11_in = PINC & _BV(DHT11_PIN);
	
	if(!dht11_in){
		Serial.println("dht11 start condition 2 not met");
		return;
	}
	delayMicroseconds(80);
	// now ready for data reception
	for (i=0; i<5; i++)
		dht11_dat[i] = read_dht11_dat();
		
	DDRC |= _BV(DHT11_PIN);
	PORTC |= _BV(DHT11_PIN);
	
        byte dht11_check_sum = dht11_dat[0]+dht11_dat[1]+dht11_dat[2]+dht11_dat[3];
	// check check_sum
	if(dht11_dat[4]!= dht11_check_sum)
	{
		Serial.println("DHT11 checksum error");
	}
	
	Serial.print("[HU]");
	Serial.print(dht11_dat[0], DEC);
	Serial.print(".");
	Serial.print(dht11_dat[1], DEC);
	Serial.println("");
	Serial.print("[TE]");
	Serial.print(dht11_dat[2], DEC);
	Serial.print(".");
	Serial.print(dht11_dat[3], DEC);
	Serial.println("");


        sprintf(sTemp1,"H:%d.%d%% T:%d.%dC",dht11_dat[0],dht11_dat[1],dht11_dat[2],dht11_dat[3]);
        //sprintf(sTemp1,"Humdity: %d.%d %%",dht11_dat[0],dht11_dat[1]);
        //sprintf(sTemp2,"Temp: %d.%d C",dht11_dat[2],dht11_dat[3]);
        //lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(sTemp1);
              
        
	
	//delay(5000);  
}

void loop()
{
  if(tLoopIndex++%5==0)
    check_temp_sensor();
  print_time();  
  
  int val; 
  val=analogRead(pin_sound);
  Serial.print("[SD]");
  Serial.println(val,DEC);

  //val=analogRead(pin_light);
  val = digitalRead(dpin_light); 
  Serial.print("[LT]");
  Serial.println(val,DEC);

  val = digitalRead(dpin_red); 
  Serial.print("[RD]");
  Serial.println(val,DEC);
  if(val==1) {
      digitalWrite(buzzer,HIGH);//发声音
      delay(1);//延时1ms
      digitalWrite(buzzer,LOW);//bu发声音
  }
 
  delay(1000);  
}
