/* Programa para el aspirador de Olesa
 
* Sketch para ArdBox  
  * Versión 2.0 
   
* Sketch para arduino NANO  

 * Versión 1.1 -->
    Empieza la primera vez con 3 minutos y hay un pulsador para subir y otro para bajar tiempo. cada pulso sube o baja 10 segundos

 * Versión 1.0 -->
    Programa basico, tiempo fijado al programar


*/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

/* Inicializo la LCD, dirección 0x27. LCD de 1602 */
LiquidCrystal_I2C lcd(0x3F,16,2);


// Variables fijas  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Entradas fisicas ///////////////////////////////////////////////////////////////////////////////////////////////////////////
const int moneda = A0;    //entrada moneda A0         // I0.4
const int pulsUP = A1;    //pulsador subir tiempo A1  // I0.5
const int pulsDown = A2;  //pulsador bajar tiempo A2  // I0.6 
const int termico = A3;   // entrada termico saltado  // I0.7

/// Salidas Fijas /////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int salidaRele = 12;  // salida de relé (relé 2)
const int hab_mon = 13;     // salida habilitar monedero (relé 3)

//Tiempos /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long tiempoAgregado=0;
float segundos = 1.8;                             // tiempo en segundos
float ultSegundos = 1.8;
unsigned long tMoneda = 180000;     // tiempo que da la moneda 2 (3 minutos)  181500
const unsigned long tMaxHab  = 999000;    // tiempo para deshabilitar monedero (18-20 minutos)
unsigned long millisPrim = 0;             //millis al pulsar cualquier moneda
unsigned long millisRestan = 0;           // milisegundos de crédito que quedan

int minTempRestan = 0;      // minutos que restan de crédito
int minUniTempRestan = 0;   // unidades de minutos que restan de crédito 
int minDecTempRestan = 0;   // decenas de minutos que restan de crédito
int segTempRestan = 0;      // segundos que restan de crédito
int segUniTempRestan = 0;   // Unidades de segundos que restan de crédito
int segDecTempRestan = 0;   // decenas de segundos que restan de crédito
int millisTempRestan = 0;   // millis que restan de credito
int SegTotalRestan = 0;     // Segundos totales que restan de credito 
int k = 0;                  // Vaariable para el display

/// EEPROM          ////////////////////////////////////////////////////////////////////////////////////////////////////////////
int dirEepContador = 150;      // Dirección para contador
float contador = 0;          // Contador monedas
float ultContador = 0;       // Lee contador monedas al arrancar
float contDinero = 0;        // Calcula total dinero

int dirEepSegundos = 1;      // Dirección para segundos

int dirEepVers = 0;          // Direccion donde guardo la version de la placa
int vers = 10;                // Version del software
int versionAsp= 0;
/// CONTADORES    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int contTotal = 0;            // Suma de contadores
float contDineroTotal = 0;    // Contador total de dinero

short est_mon = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// display 7 segemntos

Adafruit_7segment matrix = Adafruit_7segment();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  // put your setup code here, to run once:

  pinMode(moneda, INPUT);       // configuro el pin como entrada
//  digitalWrite(moneda, HIGH);   // Pull up   
  pinMode(pulsUP, INPUT);       // configuro el pin como entrada
//  digitalWrite(pulsUP, HIGH);   // Pull up  
  pinMode(pulsDown, INPUT);       // configuro el pin como entrada
//  digitalWrite(pulsDown, HIGH);   // Pull up  
  pinMode(termico, INPUT);       // configuro el pin como entrada
//  digitalWrite(termico, HIGH);   // Pull up  
    
  pinMode(salidaRele, OUTPUT);  // configuro el pin como entrada
  digitalWrite(salidaRele, LOW);   // Para que el relé esté desactivado
  pinMode(hab_mon, OUTPUT);  // configuro el pin como entrada
  digitalWrite(hab_mon, LOW);   // Para que el relé esté desactivado
  
//EEPROM.put(150, 1);
//EEPROM.put(dirEepSegundos,segundos); 
segundos = EEPROM.get(dirEepSegundos, ultSegundos);     // Leo los segundos de la EEprom
contador = EEPROM.get(dirEepContador,ultContador);      // Leo la lectura de contador de la EEprom
//EEPROM.write(0,20);                                     // Grabo la versión del software
//versionAsp = EEPROM.read(0);                            // Leo la versión del software
contador = ultContador;
//contador = 0;

  matrix.begin(0x70);
  matrix.print(10000, DEC);
  matrix.writeDisplay();

  Wire.begin();
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

   lcd.setCursor(0,0); 
   lcd.print("   Felisucker"); 
   lcd.setCursor(0,1); 
   lcd.print("Version 2.0"); 
   
  delay(3000);
  lcd.clear();
}

void segDisplay() {
      matrix.print(SegTotalRestan, DEC);
      matrix.writeDisplay();
}

void segDisplayOff() {
      matrix.print(10000, DEC);
      matrix.writeDisplay();
}

void DisplayLCDContador() {
  lcd.setCursor(0,0); 
  lcd.print("Contador: ");
  lcd.setCursor(10,0); 
  lcd.print(contador);    
  lcd.setCursor(0,1); 
  lcd.print("Temp. prog: ");
  lcd.setCursor(11,1); 
  lcd.print(segundos*100); 
}

void tiempo() {

tMoneda = segundos * 100000;

  if (digitalRead(moneda) == HIGH) {
      tiempoAgregado+=tMoneda; 
      contador++;;
      EEPROM.put(dirEepContador, contador);
      est_mon = 1;
    }
  if (est_mon == 1) {
  if (tiempoAgregado>0) {
    if (millisRestan == 0) {
    millisPrim = millis(); // si no había crédito tomamos inicio de partida
    }
    millisRestan += tiempoAgregado; // En todo caso sumamos crédito
    tiempoAgregado = 0;           // pone a 0 tiempoAgregado para que no se vaya multiplicando el tiempo    
    est_mon = 0;  
    delay(500);
  }
}
  if (millisRestan > 0) { 
     digitalWrite(salidaRele, HIGH);
    unsigned long transcurridos = millis() - millisPrim; // Operación primordial con millis. Así calculamos el tiempo transcurrido
    if (millisRestan>transcurridos) {  // Si hay más crédito que transcurridos
      millisRestan-=transcurridos; // Restamos los transcurridos del crédito
      millisPrim += transcurridos;  // y tomamos el millis actual para calcular el próximo transcurrido
      }

        else { // Si ha transcurrido tiempo restante o más
          millisRestan=0;
          digitalWrite(salidaRele, LOW);
  }
}
  if (millisRestan > tMaxHab) {
    desHabilitadorMonedero();
  }

  SegTotalRestan = millisRestan / 1000;
}

void Pulsadores() {
  if (digitalRead(pulsUP) == HIGH){
    segundos = segundos + 0.1;
    EEPROM.put(dirEepSegundos,segundos);
    delay(100);
     }
  if (digitalRead(pulsDown) == HIGH){
    segundos = segundos - 0.1;
    EEPROM.put(dirEepSegundos,segundos);
    delay(100);
     }
}

void LCDOFF() {
  lcd.setCursor(0,0); 
  lcd.print("¡¡¡ ERROR !!! ");   
  lcd.setCursor(0,1); 
  lcd.print("TERMICO SALTADO");
}

void desHabilitadorMonedero() {
      digitalWrite(hab_mon, LOW);
}

void habilitadorMonedero() {
      digitalWrite(hab_mon, HIGH);
}

void loop() {
  if (digitalRead(termico) == HIGH){
      segDisplay();
      DisplayLCDContador();
      tiempo(); 
      Pulsadores();
      habilitadorMonedero();
  } else {
    digitalWrite(salidaRele, LOW);
    LCDOFF();
    segDisplayOff();
    desHabilitadorMonedero();
  }

  Serial.println(millisRestan);
  Serial.println(contador);
}
