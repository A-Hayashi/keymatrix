#include <Wire.h>
#include "Queue.h"

/*
   A  D12
   B  D11
   C  D10
   D  D8
   E  D7
   F  D6
   G  D5
*/

#define pinA  12
#define pinB  11
#define pinC  10
#define pinD  8
#define pinE  7
#define pinF  6
#define pinG  5
#define LED  13

const int rowNum = 4;
const int colNum = 3;

const int rowPin[rowNum] = { pinD, pinE, pinF, pinG };
const int colPin[colNum] = { pinA, pinB, pinC };
const byte keyMap[rowNum][colNum]  = {
  { '0', '4', '8' },
  { '1', '5', '9' },
  { '2', '6', '*' },
  { '3', '7', '#' },
};

bool currentState[rowNum][colNum];
bool beforeState[rowNum][colNum];

typedef struct {
  byte key;
  byte push;
} KEY_ST;

Queue<KEY_ST> queue = Queue<KEY_ST>(32);


void setup() {
  int i, j;
  pinMode(LED, OUTPUT);
  for ( i = 0; i < rowNum; i++) {
    pinMode(rowPin[i], OUTPUT);
  }

  for ( i = 0; i < colNum; i++) {
    pinMode(colPin[i], INPUT);
  }

  for ( i = 0; i < rowNum; i++) {
    for ( j = 0; j < colNum; j++) {
      currentState[i][j] = HIGH;
      beforeState[i][j] = HIGH;
    }
    digitalWrite(rowPin[i], HIGH);
  }

  Serial.begin(9600);
  Serial.println("Key Matrix Start");

  i2c_init();

}

void loop() {
  int i, j;
  KEY_ST key_st;
  for ( i = 0; i < rowNum; i++) {
    digitalWrite( rowPin[i], LOW );

    for ( j = 0; j < colNum; j++) {
      currentState[i][j] = digitalRead(colPin[j]);
      delay(10);
      if ( (currentState[i][j] != beforeState[i][j]) && (currentState[i][j] == digitalRead(colPin[j]))) {

        Serial.print("key(");
        Serial.print(i);
        Serial.print(",");
        Serial.print(j);
        Serial.print(") ");
        Serial.write(keyMap[i][j]);

        if ( currentState[i][j] == LOW) {
          Serial.println(" Push!");
          key_st.key = keyMap[i][j];
          key_st.push = true;
          queue.push(key_st);
          digitalWrite(LED, HIGH);
        } else {
          Serial.println(" Release!");
          key_st.key = keyMap[i][j];
          key_st.push = false;
          queue.push(key_st);
          digitalWrite(LED, LOW);
        }
        beforeState[i][j] = currentState[i][j];
      }
    }
    digitalWrite( rowPin[i], HIGH );
  }
}


void i2c_init()
{
  Wire.begin(0x10) ;                 // Ｉ２Ｃの初期化、自アドレスを10とする
  Wire.onRequest(requestEvent);     // マスタからのデータ取得要求のコールバック関数登録
  Wire.onReceive(receiveEvent);     // マスタからのデータ送信対応のコールバック関数登録
  Serial.println("i2c slave test");
}


byte d;
  
// マスターからを受信
void receiveEvent(int n) {

  Serial.println("receiveEvent");
  for (int i = 0; i < n; i++) {
    if (Wire.available() > 0)  {
      d = Wire.read();
      Serial.print(d, HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

// マスターからのリクエストに対するデータ送信
void requestEvent() {
  KEY_ST key_st;
  Serial.println("requestEvent");
  int count = queue.count();

  if (count == 0) {
    key_st.key = 0xff;
    key_st.push = 0xff;
  } else {
    key_st = queue.pop();
  }
  Wire.write(key_st.key);
  Wire.write(key_st.push);

  /*
    byte count = queue.count();
    Wire1.write(count);

    for (int i = 0; i < count; i++) {
      key_st = queue.pop();
      Wire1.write(key_st.key);
      Wire1.write(key_st.push);
    }
  */
}




