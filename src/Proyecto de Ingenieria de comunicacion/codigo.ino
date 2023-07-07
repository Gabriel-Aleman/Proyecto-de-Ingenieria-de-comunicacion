//INCLUDES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include <SoftwareSerial.h>
#include <Servo.h>


//DEFINES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#define LED_R A2
#define LED_G A1
#define LED_B A0

#define MOTOR 3
#define buzz  4



//FUNCIONES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
int getDutyCycle(int percentage){
  int pulseWidth = map(percentage, 0, 100, 0, 255);  // Mapear el porcentaje a un ancho de pulso PWM adecuado
  return pulseWidth;
}


void blink(int times, double myDelay){
  int sig = 0;
  for (int i = 1; i <= (2*times+1); i++) {
    delay(myDelay);
    digitalWrite(buzz,  sig);
    sig = ~sig;
  }
  digitalWrite(buzz,  LOW);
}




//MAIN:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
SoftwareSerial bluetooth(10, 11);               // Configura los pines RX y TX para el módulo Bluetooth
Servo myservo;                                // Crear un objeto servo
int miDutyCycle = getDutyCycle(90);           //Duty cycle del 50%

String data; 
int tries = 0;
const String password = "2";
const String cerrar   = "1";

bool close = false;                   //¿Cerrado?

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);  //Inicializa la comunicación Bluetooth a 9600 baudios

  //LED:
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  //ALARMA:
  pinMode(buzz, OUTPUT);

  //MOTOR:
  myservo.attach(MOTOR);  // Adjuntar el objeto servo al pin
  
  //Partimos de la suposición que la puerta está primero abierta:
  myservo.write(90);                   //Abrir la puerta
  analogWrite(LED_G, miDutyCycle);     //Encender luz verde



}

void loop() {
  
  if (bluetooth.available()) {
    data = bluetooth.readString(); // Lee los datos recibidos a través de Bluetooth
    Serial.print("Datos recibidos: ");    //Imprime valor recibido en el puerto serial
    Serial.println(data);
    if(close){tries++;}
  }
  
  
  if(close){
    analogWrite(LED_G, 0);               //Apagar luz verde
    analogWrite(LED_R, miDutyCycle);     //Encender luz roja
    


    if(data == password){
      myservo.write(90);                   //Abrir la puerta
      close = false;

      //Sonar el parlante 2 veces (delay de 500 ms):
      blink(2, 500);
    }
    
    else{
      
      if(tries > 3){
        analogWrite(LED_R, 0);                //Apar luz roja
        analogWrite(LED_B, miDutyCycle);      //Encender luz azul
        delay(3000);                         // Retraso de 1 minuto (60,000 milisegundos)
        analogWrite(LED_B, 0);                //Apar luz azul
        tries = 0;
      }
    }

  }
  
  else{
    analogWrite(LED_R, 0);                       //Apagar luz roja
    analogWrite(LED_G, miDutyCycle);             //Encender luz verde
    if(data == cerrar){
      myservo.write(0);                    //Cerrar la puerta

      close = true;

      //Sonar el parlante 3 veces (delay de 500 ms):
      blink(3, 500);
    }
  }

}
