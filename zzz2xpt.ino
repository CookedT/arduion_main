#define PT_USE_TIMER
#define PT_USE_SEM
#include <pt.h>
static struct pt pt1, pt2;// 每个 protothread 需要一个


//串口门禁数据
long Password1 = 8143642; //可识别卡号1，此处修改卡号
long Password2 = 7908190; //可识别卡号2，此处修改卡号
int flag = 0, RX_Flag = 0; //串口标志
char Code[14]; //用于存放读到串口数据
long Num = 0; //解码数据
int Door = 0;
int mpin = 5;

//温度传感器数据
int dataPin = 2;
int tp_flag = 0;
byte tp_temp, RH_H, RH_L, T_H, T_L, crc;
float tt, rh;

void setup()
{
  PT_INIT(&pt1);  // initialise the two       //初始化这两个 protothread 变数
  PT_INIT(&pt2);  // protothread variables

  //温度
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, HIGH);
  //门禁
  pinMode(13, OUTPUT);
  pinMode(mpin, OUTPUT);
  Serial.begin(9600);
  Serial.println("This is a test for access control system");
  delay(100);
  digitalWrite(mpin, HIGH);
}




void Read_ID(void)
{
  int i = 0;
  char temp;
  if (Serial.available() > 0) // 串口空闲
  {
    temp = Serial.read();
    delay(2);
  }
  //判断是否刷卡解锁数据
  if (temp == 0X02) //接收起始位
  {
    flag = 1; RX_Flag = 0; //
    for (i = 0; i < 14; i++)  //检测到起始位,开始接收数据
    {
      if (temp == 0X03) //检测到结束码,
      {
        flag = 0; //标志清零
        if (i == 13) RX_Flag = 1; //第13位为结束码，收到数据，标志置1
        else RX_Flag = 0;
        break;
      }
      Code[i] = temp;
      temp = Serial.read();
      delay(2);
    }
    flag = 0; //标志清零
  }
  //判断解锁结束
}

/*
  void open_door()
  {
  digitalWrite(mpin, LOW);
  Serial.println(Num);
  Serial.println("door is open");
  digitalWrite(mpin, HIGH);

  Serial.print("Current Temperature: ");
  Serial.print(tt , DEC);
  Serial.print("C   ");
  Serial.print("Current Humidity: ");
  Serial.print(rh);
  Serial.println("%");
  }
*/


void open_door_led() {
  boolean ledstate = digitalRead(13);
  ledstate ^= 1;   // toggle LED state using xor //使用xor 来改变 LED 状态
  digitalWrite(13, ledstate); // write inversed state back //写入改变后的状态
}

static int open_door_1(struct pt *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  digitalWrite(mpin, HIGH);
  while (1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis(); // take a new timestamp
    digitalWrite(mpin, LOW);
    Serial.println("ccb_led_xh");
  }
  Serial.println("ccb_led");

  PT_END(pt);
}

/*
static int open_door_led(struct pt *pt, int interval, int sm) {
  static unsigned long timestamp = 0;
  static int i = 0;
  PT_BEGIN(pt);
  for ( i = 0; i < sm; i++)
  {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();
    open_door_led();

  }
  digitalWrite(13, false);
  PT_END(pt);
}
*/

static int open_door_led(struct pt *pt, int interval, int sm) {
  static unsigned long timestamp = 0;
  static int i = 0;
  PT_BEGIN(pt);
  while (1)
  {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval );
    timestamp = millis();
    open_door_led();
    Serial.println("ccb_led");
  }
  digitalWrite(13, false);
  PT_END(pt);
}


//am2301程序库
byte read_8bit_data()
{
  byte i, data = 0;
  for (i = 0; i < 8; i++)
  {
    tp_flag = 2;
    while ((digitalRead(dataPin) == 0) && tp_flag++);
    delayMicroseconds(30);
    tp_temp = 0;
    if (digitalRead(dataPin) == 1) tp_temp = 1;
    tp_flag = 2;
    while ((digitalRead(dataPin) == 1) && tp_flag++);
    if (tp_flag == 1) break;
    data <<= 1;
    data |= tp_temp;
  }
  return data;
}

byte get_dh21()
{
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, LOW);
  delay(4);

  digitalWrite(dataPin, HIGH);
  delayMicroseconds(40);

  pinMode(dataPin, INPUT);
  if (digitalRead(dataPin) == 0)
  {
    tp_flag = 2;
    while ((digitalRead(dataPin) == 0) && tp_flag++);
    tp_flag = 2;
    while ((digitalRead(dataPin) == 1) && tp_flag++);
    RH_H = read_8bit_data();
    RH_L = read_8bit_data();
    T_H = read_8bit_data();
    T_L = read_8bit_data();
    crc = read_8bit_data();
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, HIGH);
    tp_temp = (T_H + T_L + RH_H + RH_L);
    if (tp_temp != crc)
    {
      RH_H = 0;
      RH_L = 0;
      T_H = 0;
      T_L = 0;
      return -1;
    }
    return 0;
  }
  return -1;
}

//获取温度
float get_tmp()
{
  if (get_dh21() == -1)
  {
    Serial.println("Read DH21 error");
  }
  else
  {
    tt = (T_H << 8 ) | T_L;
    rh = (RH_H << 8 ) | RH_L;
    tt = tt / 10;
    rh = rh / 10;
  }
}




//am2301程序库结束



/*
  int DeRfHex(char* Code[])
  {
  Num = 0;
  for (int i = 5; i < 11; i++) //数据解码，6位字符转为十六进制数,对应十进制卡号
  {
    Num <<= 4;
    if ( *Code[i] > 64) Num += ((*Code[i]) - 55);
    else Num += ((*Code[i]) - 48);
    //Serial.println(Num);
  }
  return Num  ;
  }
*/


void loop()
{
  int i;
  long temp = 0, time = 0;
  RX_Flag = 0;

  Read_ID();

  //门禁开锁程序
  if (RX_Flag == 1)
  {
    for (int i = 5; i < 11; i++) //数据解码，6位字符转为十六进制数,对应十进制卡号
    {
      Num <<= 4;
      if (Code[i] > 64) Num += ((Code[i]) - 55);
      else Num += ((Code[i]) - 48);
    }

    if ((Num == Password1) || (Num == Password2)) //识别成功
    {
      open_door_1(&pt1, 5000);
      open_door_led(&pt2, 200, 25);

    }
    else //识别失败
    {
      digitalWrite(mpin, HIGH);
      while (Serial.read() > 0); //清除缓存区
      RX_Flag = 0; //标志位清零
      Num = 0; //数据清零
    }
  }

  //温度传感器程序
  get_tmp();


}
