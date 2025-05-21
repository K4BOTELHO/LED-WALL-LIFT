#include <IRremote.h>        //IR
#include <RCSwitch.h>        //RF
  
// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:

#define BSubir 6                      //ACIONAMENTO VALVULA DE SUBIDA
#define BDescer 8                     //ACIONAMENTO VALVULA DE DESCIDA
#define ContatoraM 3                  //CONTATORA MOTOR
#define SensorAbre 4                  //FC DE ABERTURA
#define SensorFecha 5                 //FC DE FECHAMENTO
#define SensorWeather A3              //PINO DE LEITURA DO SENSOR DE TEMPO

#define CCABRE A4                //FC DE FECHAMENTO
#define CCFECHA A5                 //FC DE ABERTURA

unsigned long int tempobotao = 0;         // Variavel que guarda o momento que comeca a girar o motor
unsigned long int TempoAbreFecha = 0; 

char estadoCmdTil = 0; 
int ultimoCmdTil = 0;
int TempMotor = 0;

int CodCmd = 100; 
int CodIFs = 200;
int CodExecutation = 300;

int BState1 = HIGH;             // ESTADO DE SAIDA DO PINO
int BState2 = HIGH;             // ESTADO DE SAIDA DO PINO
int CState = HIGH;              // ESTADO DE SAIDA DO PINO

int estadotil = 0;                  // Estado controle abertura

//----------------------------------------------------- IR
int RECV_PIN = A2;              // Pino IR
float armazenavalor;            // Variavel valor do IR

IRrecv irrecv(RECV_PIN);
decode_results results;

//----------------------------------------------------- CC

int ccoState1;                // the current reading from the input pin
int lastButtonState1 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 50;    // the debounce time; increase if the output flickers

int ccoState2;                // the current reading from the input pin
int lastButtonState2 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay2 = 50;    // the debounce time; increase if the output flickers

//----------------------------------------------------- Weather
int ccoState3;                // the current reading from the input pin
int lastButtonState3 = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay3 = 50;    // the debounce time; increase if the output flickers

//----------------------------------------------------- RF

RCSwitch mySwitch = RCSwitch();

void setup()
{
  pinMode(BDescer, OUTPUT);          // Define os pinos dos motores como saida
  pinMode(BSubir, OUTPUT);           // Define os pinos dos motores como saida
  pinMode(ContatoraM, OUTPUT);       // Define os pinos dos motores como saida
  pinMode(SensorFecha, INPUT);       // Define os pinos dos motores como saida
  pinMode(SensorAbre, INPUT);        // Define os pinos dos motores como saida
  pinMode(CCABRE, INPUT);       // Define os pinos dos motores como saida
  pinMode(CCFECHA, INPUT);        // Define os pinos dos motores como saida
  pinMode(SensorWeather, INPUT);        // Define os pinos dos motores como saida
  pinMode(13, OUTPUT);               // Define os pinos dos motores como saida
  
  digitalWrite(BDescer, BState1);    // Define o estado inicial
  digitalWrite(BSubir, BState2);     // Define o estado inicial
  digitalWrite(ContatoraM, CState);  // Define o estado inicial
  
  irrecv.enableIRIn();               // Inicializa o receptor IR

  mySwitch.enableReceive(0);         // Receiver on interrupt 0 => that is pin #2
  
  Serial.begin(9600);

}

void loop() {

  //----------------------------------------------------- LOOP DO RF

    if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();
     if (value == 0){
     Serial.print("Codigo desconhecido");
     CodCmd = 101; // ##################################################### Codigo RF Desconhecido
     } else {
      
      if (mySwitch.getReceivedValue() == 5592368) {             //*******SUBIR
           if (millis() - tempobotao >= 500){
               Subir();  
           }   
      } else if (mySwitch.getReceivedValue() == 5592512) {      //*******DESCER
           if (millis() - tempobotao >= 500){
               Descer();     
           }
      } else if (mySwitch.getReceivedValue() == 5592560) {      //*******PARAR
           if (millis() - tempobotao >= 500){
               PararControl();
           }     
      }        
   }
         
    Serial.print("Received ");
    Serial.println( mySwitch.getReceivedValue() );
    
    mySwitch.resetAvailable();
  }
  
  
  //----------------------------------------------------- LOOP DO IR
 

  if (irrecv.decode(&results)) {
    
    Serial.print("Valor lido : ");
    Serial.println(results.value, HEX);
    armazenavalor = (results.value);
    if (armazenavalor == 0x202708F || armazenavalor == 0x6EDFE961)            //*******SUBIR
    {
      if (millis() - tempobotao >= 500){
          Subir();
          CodCmd = 108; // ##################### Subir via IR
      }
    }
    else if (armazenavalor == 0x202D02F || armazenavalor == 0x76B366E3 )      //*******DESCER
    {
      if (millis() - tempobotao >= 500){
          Descer();
          CodCmd = 109; // ##################### Descer via IR
      }
    }
    else if (armazenavalor == 0x923F150B || armazenavalor == 0x202B04F)       //*******PARAR
    {
      if (millis() - tempobotao >= 500){
          PararControl();
          CodCmd = 110; // ##################### Direito via IR
      }      
    }
    irrecv.resume(); //Le o próximo valor
}

  //----------------------------------------------------- LOOP DO SERIAL
  
      if (Serial.available() > 0) {
        char letra = Serial.read();
            if (letra == 'w'){                 //*******SUBIR
                    Subir();
                    CodCmd = 117; // ##################################################### Subir via Serial
                    
            } else if (letra == 's'){          //*******DESCER
                    Descer();
                    CodCmd = 118; // ##################################################### Descer via Serial
                    
            } else if (letra == 'q'){          //*******PARAR
                    PararControl();
                    CodCmd = 119; // ##################################################### Parar via Serial
                    
            } 
}

ContatoSecoAbre();
ContatoSecoFecha();
Weather();

    if (TempMotor > 0){
        if (millis() - TempoAbreFecha >= 1700){
          
            TempMotor = 0;
            digitalWrite(BDescer, BState1);              // ESCREVE NOS PINOS
            digitalWrite(BSubir, BState2);               // ESCREVE NOS PINOS
            CodIFs = 201; // ##################################################### IF do Encoder
         }
      }


if (ultimoCmdTil > 0){
   
      if (digitalRead(SensorAbre) == LOW) {
            if (ultimoCmdTil == 2){
             PararControl();
            }
          }
      if (digitalRead(SensorFecha) == LOW ) {
            if (ultimoCmdTil == 1){
             PararControl();
            }
      }
}
/*
if (millis() - TempoAbreFecha >= 5000){
    if (digitalRead(Sensor) == LOW) {
          if (CState == LOW){
              PararControl();
          }
    }
}
*/
   /*   
if (CState == HIGH) {
      if (digitalRead(Sensor) == LOW){
        PararControl();
      }
}*/
/*
   if (controleoff == 10 && controleoff2 == 10 && volta == true){
            if (digitalRead(pinoSensor) == LOW){          // No centro -> Posicao correta
              
                Subir();
                CodIFs = 202; // ##################################################### IF da subida automatica
         }        
      }
      
   if (millis() - TempoAbreFecha >= 27000){   //750
            PararControl();
            CodIFs = 204; // ##################################################### IF do Encoder
   }
*/
            
   /*if (controleoff == 0) {
       if (Position == 62500 || Position == -62500){
          PararControl();
          CodIFs = 203; // ##################################################### IF da posicao de 90 graus
       }
   }*/
//Serial.println(ultimoCmdTil);             
digitalWrite(ContatoraM, CState);
digitalWrite(13, digitalRead(SensorAbre)); 

}
//----------------------------------------------------------------------

void Subir() {
             TempoAbreFecha = millis();
             tempobotao = millis();

             CState = LOW;
             digitalWrite(ContatoraM, CState);

             BState1 = LOW;                             //VALVULA SOLENOIDE
             BState2 = HIGH;                             //VALVULA SOLENOIDE

             TempMotor = 1;
             ultimoCmdTil = 1;
             estadoCmdTil = 10;
             
             
             /*
             digitalWrite(BDescer, BState1);   // ESCREVE NOS PINOS
             digitalWrite(BSubir, BState2);               // ESCREVE NOS PINOS
             */
             
            
             Serial.println("SUBINDO");
             CodExecutation = 301; // ##################################################### Funcao subir Executada        
}

void Descer() {
         TempoAbreFecha = millis();
         tempobotao = millis();

         CState = LOW;
         digitalWrite(ContatoraM, CState);
             
         BState1 = HIGH;                             //VALVULA SOLENOIDE
         BState2 = LOW;                             //VALVULA SOLENOIDE
         
         TempMotor = 1;
         ultimoCmdTil = 2;
         estadoCmdTil = 10;
         
         /*
         digitalWrite(BDescer, BState1);   // ESCREVE NOS PINOS
         digitalWrite(BSubir, BState2);               // ESCREVE NOS PINOS
         */
         Serial.println("DESCENDO");
         CodExecutation = 303; // ##################################################### Funcao descer Executada   
}

void PararControl() {
         tempobotao = millis();

         BState1 = HIGH;                             //VALVULA SOLENOIDE
         BState2 = HIGH;                             //VALVULA SOLENOIDE
         CState = HIGH;
         
         digitalWrite(BDescer, BState1);               // ESCREVE NOS PINOS
         digitalWrite(BSubir, BState2);               // ESCREVE NOS PINOS
         digitalWrite(ContatoraM, CState);

         ultimoCmdTil = 0;
         TempMotor = 0;
         Serial.println("PARANDO");
}

void ContatoSecoAbre(){  // declaração da função espera 
   int reading1 = digitalRead(CCABRE);

    if (reading1 != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis(); }

      if ((millis() - lastDebounceTime1) > debounceDelay1) {

       if (reading1 != ccoState1) {
          ccoState1 = reading1;

         if (ccoState1 == LOW) {

            if (millis() - tempobotao >= 500){
              Subir();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState1 = reading1;
     //Serial.println("ContatoSeco() Executou");
}

void ContatoSecoFecha(){  // declaração da função espera 
   int reading2 = digitalRead(CCFECHA);

    if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis(); }

      if ((millis() - lastDebounceTime2) > debounceDelay2) {

       if (reading2 != ccoState2) {
          ccoState2 = reading2;

         if (ccoState2 == LOW) {

            if (millis() - tempobotao >= 500){
               Descer();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState2 = reading2;
     //Serial.println("ContatoSeco() Executou");
}

void Weather(){  // declaração da função espera 
   int reading3 = digitalRead(SensorWeather);

    if (reading3 != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis(); }

      if ((millis() - lastDebounceTime3) > debounceDelay3) {

       if (reading3 != ccoState3) {
          ccoState3 = reading3;

         if (ccoState3 == LOW) {

            if (millis() - tempobotao >= 500){
               Subir();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState3 = reading3;
     //Serial.println("ContatoSeco() Executou");
}
