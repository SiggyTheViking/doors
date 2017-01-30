void setup() {
  // put your setup code here, to run once:
  pinMode(13,INPUT);
  pinMode(12,INPUT);
  pinMode(11,INPUT);
  pinMode(10,INPUT);
  pinMode(9,INPUT);
  Serial.begin(9600);
}
char state;
void loop() {
  state = 0;
  state |= !digitalRead(13)<<0;
  state |= !digitalRead(12)<<1;
  state |= !digitalRead(11)<<2;
  state |= !digitalRead(10)<<3;
  state |= !digitalRead(9)<<4;
  Serial.println(state);
  delay(1000);
}
