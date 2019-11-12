 int trig= 12; // membuat varibel trig yang di set ke-pin A0
 int echo= 11; // membuat variabel echo yang di set ke-pin A1

 int en1 = 8 ;
 int in3 = 10;
 int in4 = 9 ; 

 int echo1= 2;
 int trig1= 3;

 int en0 = 4;
 int in5 = 6;
 int in6 = 5;

/**
   Initialize.
*/
void setup() {
  Serial.begin (9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);  
  pinMode(en1, OUTPUT);

  Serial.begin (9600);
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);
  pinMode(en0, OUTPUT);

  }
  
/**
   Main loop.
*/
void loop () {
  float duration, distance;
  digitalWrite(trig, LOW);
  delay(1000);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = (duration/2) * 0.03448;
  
  Serial.print("Jarak benda 1 = ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance <= 5){ //mengatur jarak pendeteksian barang
     digitalWrite(in3, LOW);
     digitalWrite(in4, HIGH);
     analogWrite(en1, 150);
     delay(10); // waktu pada saat mendeteksi barang
  }else{
     digitalWrite(in3, LOW);
     digitalWrite(in4, LOW);
     analogWrite(en1, 150);

  }
  delay(500);

  digitalWrite(trig1, LOW);
  delay(1000);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  duration = pulseIn(echo1, HIGH);
  distance = (duration/2) * 0.03448;
  
  Serial.print("Jarak benda 2 = ");
  Serial.print(distance);
  Serial.println(" cm");

    if (distance > 0 && distance <= 5){ //mengatur jarak pendeteksian barang
     digitalWrite(in5, LOW);
     digitalWrite(in6, HIGH);
     analogWrite(en0, 150);
     delay(10); // waktu pada saat mendeteksi barang
  }else{
     digitalWrite(in5, LOW);
     digitalWrite(in6, LOW);
     analogWrite(en0, 150);

  }
  delay(500);
  }
