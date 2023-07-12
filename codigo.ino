//INCLUDES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#include <SoftwareSerial.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

//DEFINES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#define LED_R A2    //PIN LED rojo
#define LED_G A1    //PIN LED verde
#define LED_B A0    //PIN LED azul

#define MOTOR A3    //Pin motor
#define buzz  4     //Pin alarma



//FUNCIONES:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/*
getDutyCycle: Dado un porcentaje deseado para el ciclo de trabajo, 
convertirlo a un valor de 0 a 255 que podamos escribir en los LED.
-inputs:
  *percentage (int): Porcentaje deseado.
-outputs:
  *pulseWidth (int): Valor de 0 - 255 a escribir en la salida ana-
                     lógica.
*/
int getDutyCycle(int percentage){
  int pulseWidth = map(percentage, 0, 100, 0, 255);  // Mapear el porcentaje a un ancho de pulso PWM adecuado
  return pulseWidth;
}

/*
blink: Hacer sonar el buzzer un determinado número de veces.
-inputs:
  *times (int): Cantidad de veces que queremos que suene la
                alarma.
  *myDelay (double): Retardo en ms entre cada alarma.
*/
void blink(int times, double myDelay){
  int sig = 0;
  for (int i = 1; i <= (2*times+1); i++) {
    delay(myDelay);
    digitalWrite(buzz,  sig);
    sig = ~sig;
  }
  digitalWrite(buzz,  LOW);
}


//Variables globales y objetos:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
SoftwareSerial bluetooth(10, 11);           // Configura los pines RX y TX para el módulo Bluetooth
Servo myservo;                              // Crear un objeto servo
LiquidCrystal_I2C lcd(0x27, 16, 2);         // Dirección I2C, número de columnas y número de filas de la pantalla


bool close = false;                         //Variable de Estado para saber si la puerta está cerrada
int miDutyCycle = getDutyCycle(90);         // Duty cycle del 90%
String data;                                //Datos recibidos por medio del modulo Bluetooth
int tries = 0;                              //Número de intentos
const int maxTries = 3;                     //Cantidad máxima de intentos permitidos
const String password = "B90175";           //Contraseña de acceso
const String cerrar   = "1";                //Codigo para cerrar


//MAIN:
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
void setup() {
  Serial.begin(9600);     //Habilita la comunicación Serial
  bluetooth.begin(9600);  //Inicializa la comunicación Bluetooth a 9600 baudios

  //LEDs:
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  //PANTALLA:
   lcd.begin(16, 2);  // Inicializar la pantalla LCD
  lcd.backlight();
  lcd.setCursor(0, 0);  // Establecer la posición del cursor en la primera fila y la primera columna

  //ALARMA:
  pinMode(buzz, OUTPUT);

  //MOTOR:
  myservo.attach(MOTOR);  // Adjuntar el objeto servo al pin
  
  //Partimos de la suposición que la puerta está primero abierta:
  myservo.write(90);                   //Abrir la puerta
  analogWrite(LED_G, miDutyCycle);     //Encender luz verde
  lcd.print("ABIERTO");  // Imprimir el texto en la pantalla
}

void loop() {
  //Si se recibe información del modulo:
  if (bluetooth.available()) {
    data = bluetooth.readString();      // Lee los datos recibidos a través de Bluetooth
    Serial.print("Datos recibidos: ");  //Imprime valor recibido en el puerto serial
    Serial.println(data);               
    if(close){tries++;}                 //Aumentar en 1+ los intentos si se intenta abrir.
  }
  
  //Si está cerrado:
  if(close){

    //Imprimir que la puerta está cerrada:
    lcd.setCursor(0, 0); 
    lcd.print("CERRADO");  

    analogWrite(LED_G, 0);               //Apagar luz verde
    analogWrite(LED_R, miDutyCycle);     //Encender luz roja
    


    if(data == password){
      myservo.write(90);                  //Abrir la puerta
      close = false;
      lcd.clear();                        // Limpiar la pantalla

      //Sonar el parlante 2 veces (delay de 500 ms):
      blink(2, 500);
    }
    
    //Si está abierto:
    else{
      //Si se excede la cantidad máxima de intentos
      if(tries > maxTries){
        //Dar alerta de que excedió los intentos:
        /*-----------------------------------------------------------*/
        lcd.clear();                        // Limpiar la pantalla
        lcd.setCursor(0, 0); 
        lcd.print("EXCEDIO EL NUM");
        lcd.setCursor(0, 1); 
        lcd.print("DE INTENTOS");
        /*-----------------------------------------------------------*/
        
        analogWrite(LED_R, 0);            //Apar luz roja
        analogWrite(LED_B, miDutyCycle);  //Encender luz azul
        delay(30000);                      // Retraso de 1 minuto (60,000 milisegundos)
        analogWrite(LED_B, 0);            //Apar luz azul
        tries = 0;                        //Reinciar el contador de intentos

        lcd.clear();                        // Limpiar la pantalla
      }
    }

  }
  
  //Si está abierto:
  else{
    //Imprimir que la puerta está abierta:
    lcd.setCursor(0, 0); 
    lcd.print("ABIERTO");  

    analogWrite(LED_R, 0);                //Apagar luz roja
    analogWrite(LED_G, miDutyCycle);      //Encender luz verde

    //Si se recibe la instrucción de cerrar:
    if(data == cerrar){
      myservo.write(0);                   //Cerrar la puerta
      close = true;
      tries = 0;                          //Reinciar el contador de intentos
      lcd.clear();                        //Limpiar la pantalla

      //Sonar el parlante 3 veces (delay de 500 ms):
      blink(3, 500);
    }
  }

}
