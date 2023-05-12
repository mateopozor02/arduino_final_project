#include <LiquidCrystal_I2C.h>
#include <virtuabotixRTC.h>
#include <Servo.h>
#include <Wire.h> 
#include <Keypad.h>

//Declaracion de pines
const int RELE_PIN = 2; 
const int TRIGGER_PIN = 4; 
const int ECHO_PIN = 3; 
const int SERVO_PIN = 5; 

//Setup del teclado
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = { 13, 12, 11, 10 };
byte colPins[COLS] = { 9, 8, 7, 6 }; 

//Objetos de sensores y actuadores
LiquidCrystal_I2C lcd(0x27,16,2);
virtuabotixRTC myRTC(A0, A1, A2);
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Servo myServo;  

//Variables de sensores
int cm = 0; 
char breed_type;
int currentHours = 0; 
int currentMinutes = 0; 
int horas_0 = 100; 
int minutos_0 = 100; 
int horas_1 = 100; 
int minutos_1 = 100; 
int numIntervals; 
 
class Interval {
  public:
    Interval(int h0, int m0, int h1, int m1); 

    int hours_0;
    int minutes_0;
    int hours_1;
    int minutes_1; 
};

Interval::Interval(int h0, int m0, int h1, int m1){
  hours_0 = h0;
  minutes_0 = m0;
  hours_1 = h1; 
  minutes_1 = m1;
}

class Mascota {
  private:
  	char type; 
  
  public:
  	Mascota();
  	void setType(char t); 
  	void feed(); //Abre la compuerta 
};

Mascota::Mascota(){}

void Mascota::setType(char t){
  type = t; 
}

void Mascota::feed(){
  if (type == 'A'){

    digitalWrite(RELE_PIN, LOW);
    delay(1500);  
    myServo.write(145); 
    delay(1500); 
    myServo.write(180); 
    delay(1500);
    digitalWrite(RELE_PIN, HIGH); 
    delay(1000); 

  } else if (type == 'B'){

    digitalWrite(RELE_PIN, LOW);
    delay(1500);  
    myServo.write(145); 
    delay(2500); 
    myServo.write(180); 
    delay(1500);
    digitalWrite(RELE_PIN, HIGH); 
    delay(1000); 
    
  } else if (type == 'C'){
    
    digitalWrite(RELE_PIN, LOW);
    delay(1500);  
    myServo.write(145); 
    delay(3500); 
    myServo.write(180); 
    delay(1500);
    digitalWrite(RELE_PIN, HIGH); 
    delay(1000);

  }
}

Mascota pet; 
Interval *intervals;
bool *wasOpened; 

void setup(){
  //Mensaje de bienvenida del display
  lcd.init(); 
  lcd.backlight(); 
  lcd.setCursor(1,0);
  lcd.print("Dispensador de");
  lcd.setCursor(0,1); 
  lcd.print("Alim. de Mascota");
  delay(3000); 
  lcd.clear();
  lcd.setCursor(0,0); 

  //Setup del rele
  pinMode(RELE_PIN, OUTPUT); 
  digitalWrite(RELE_PIN, HIGH);

  //Setup del servo cerrando la compuerta 
  myServo.attach(SERVO_PIN); 
  myServo.write(180); 

  //myRTC.setDS1302Time(0, 34, 17, 3, 10, 5, 2023);
  
  Serial.begin(9600);
  
}

void loop(){
  //lcd.setCursor(0,0); 

  //Se actualiza y registra el tiempo a cada momento
  myRTC.updateTime(); 

  //Cuando no se presiona botones sirve como reloj
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(myRTC.dayofmonth);
  lcd.print("/");
  lcd.print(myRTC.month);
  lcd.print("/");
  lcd.print(myRTC.year);
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);
  // Delay so the program doesn't print non-stop
  delay(250);
  if (myRTC.seconds == 59){
    lcd.clear(); 
  }

  //A cada momento se lee la distancia del sensor: 
  cm = 0.01723 * readUltrasonicDistance(TRIGGER_PIN, ECHO_PIN); 
  
  char buffer = kpd.getKey();
  
  if (buffer){
    //* indica que se busca modificar tipo de raza
    if (buffer == '*'){
      lcd.clear(); 
      lcd.setCursor(0,0); 
      lcd.print("Tipo de raza: "); 
      
      buffer = kpd.waitForKey();
      Serial.println(buffer); 
      
      if (buffer != NO_KEY){
        lcd.setCursor(14, 0); 
        lcd.print(buffer);
        delay(3000); 
        
        switch(buffer){
          case 'A': {
            pet.setType('A'); 
            lcd.clear(); 
            lcd.setCursor(0,0); 
            lcd.print("Raza Pequenia"); 
            delay(3000); 
            lcd.clear(); 
            break; 
          }
          case 'B': {
            pet.setType('B'); 
            lcd.clear(); 
            lcd.setCursor(0,0); 
            lcd.print("Raza Mediana"); 
            delay(3000); 
            lcd.clear(); 
            break;
          }
          case 'C': {
            pet.setType('C'); 
            lcd.clear(); 
            lcd.setCursor(0,0); 
            lcd.print("Raza Grande"); 
            delay(3000); 
            lcd.clear(); 
            break;
          }
        }
      } 
    } else if (buffer == '#'){
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("Ingrese no. de");     // print the first part of the message
      lcd.setCursor(0,1);              // set the cursor to the second line
      lcd.print("intervalos: ");
      lcd.setCursor(12, 1); 
      char nums = kpd.waitForKey(); 

      if (nums != NO_KEY){
        numIntervals = nums - '0'; 
        Serial.print(numIntervals); 
        lcd.clear();

        intervals = (Interval*) malloc(numIntervals * sizeof(Interval)); 
        wasOpened = (bool*) malloc(numIntervals * sizeof(bool)); 

        //Al inicio todos los intervalos no se han abierto 
        for (int i = 0; i < numIntervals; i++){
          wasOpened[i] = false; 
        }

      for (int i = 0; i < numIntervals; i++){
        Serial.println("dentro del for"); 
        lcd.setCursor(2, 0); 
        lcd.print("Intervalo de"); 
        lcd.setCursor(0, 3);
        lcd.print("alimentacion");
        delay(3000); 
        lcd.clear(); 
        lcd.setCursor(0, 0);
        lcd.print("Desde: HHMM");   
        delay(1000); 
        lcd.setCursor(7, 0); 

        //Arreglos auxiliares para manejar el input
        char hoursArray[2];
        char minutesArray[2]; 

        for (int i = 0; i < 2; i++){
          char digit = kpd.waitForKey();
          if (digit != NO_KEY){
            lcd.print(digit); 
            hoursArray[i] = digit;
          } 
           
        }
        
        for (int i = 0; i < 2; i++){
          char digit = kpd.waitForKey(); 
          if (digit != NO_KEY){
            lcd.print(digit);
            minutesArray[i] = digit; 
          }
          
        }

        //Se recupera el valor entero del input del usuario
        horas_0 = getNumber(hoursArray); 
        minutos_0 = getNumber(minutesArray); 

        lcd.clear(); 
        lcd.setCursor(0,0); 
        lcd.print("Hasta: HHMM"); 
        delay(1000); 
        lcd.setCursor(7, 0); 

        for (int i = 0; i < 2; i++){
          char digit = kpd.waitForKey(); 
          if (digit != NO_KEY){
            lcd.print(digit); 
            hoursArray[i] = digit; 
          }   
        }

        for (int i = 0; i < 2; i++){
          char digit = kpd.waitForKey(); 
          if (digit != NO_KEY){
            lcd.print(digit); 
            minutesArray[i] = digit;
          }  
        }

        horas_1 = getNumber(hoursArray); 
        minutos_1 = getNumber(minutesArray); 

        Serial.println(horas_0);
        Serial.println(minutos_0);
        Serial.println(horas_1); 
        Serial.println(minutos_1);

        intervals[i] = Interval(horas_0, minutos_0, horas_1, minutos_1); 

        lcd.clear(); 
        lcd.setCursor(0,0);
      }

      } 

      lcd.clear(); 
      lcd.setCursor(0,0);
      lcd.print("Registrado!"); 
      delay(2000);  
      lcd.clear(); 
    }
  } 

  //Se monitorea siempre el tiempo de funcionamiento
  currentHours = myRTC.hours; 
  currentMinutes = myRTC.minutes; 
  Serial.print("Horas: "); 
  Serial.println(currentHours);
  Serial.print("Minutes: "); 
  Serial.println(currentMinutes);  
   
  for (int i = 0; i < numIntervals; i++){
    //Serial.println("for ejecutado"); 
    Interval myInterval = intervals[i];
    int compareHours0 = myInterval.hours_0*60 + myInterval.minutes_0; 
    int compareHours1 = myInterval.hours_1*60 + myInterval.minutes_1;
    int compareCurrent = currentHours*60 + currentMinutes; 

    if ((compareHours0 <= compareCurrent) && (compareCurrent <= compareHours1)){
      if (cm <= 15){
        if (!wasOpened[i]){
          pet.feed(); 
          wasOpened[i] = true; 
        }
      }
    }
    /*if ((myInterval.hours_0 <= currentHours) && (currentHours <= myInterval.hours_1)){
      Serial.println("Primera comparacion: ");
      if ((myInterval.minutes_0 <= currentMinutes) && (currentMinutes <= myInterval.minutes_1)){
        Serial.println("Comparacion."); 
        if (cm <= 15){
          pet.feed(); 
        }
      }
    }*/ 
  }
}

//Function to read the distance from the ultrasonic sensor 
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

int getNumber(char numArray[]){
  char buffer[3]; 
  buffer[0] = numArray[0]; 
  buffer[1] = numArray[1]; 
  buffer[2] = '\0'; 

  int result = atoi(buffer); 
  return result; 
}
