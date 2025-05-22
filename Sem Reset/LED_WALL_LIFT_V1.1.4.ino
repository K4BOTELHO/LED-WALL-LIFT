/* --------------------------------------------------------------------------------------------------------------------------------------------------------

Este software é protegido por direitos autorais e leis internacionais. Qualquer cópia não autorizada, distribuição ou uso deste software, total ou parcial,
será considerada uma violação dos direitos autorais e sujeita a medidas legais. 
Conforme estipulado pela Lei de Direitos Autorais, Lei nº 9.610/98, a pirataria de software é estritamente proibida e sujeita a penalidades legais. A cópia
não autorizada deste software constitui uma violação dos direitos de propriedade intelectual, passível de processo civil e criminal.
Ressaltamos que qualquer tentativa de reprodução, distribuição ou uso não autorizado deste software será monitorada e tratada com rigor dentro dos limites 
da lei.

-------------------------------------------------------------------------------------------------------------------------------------------------------- */

#include <IRremote.h>                     // Biblioteca do sistema IR.
#include <RCSwitch.h>                     // Biblioteca do sistema RF.
#include <EEPROM.h>                       // Biblioteca EEPROM (Memoria).

// --------------------------------------------------------------------------------------------------------------------------------------------------------  

#define SolenoideSubir 6                  // Acionamento para a valvula solenoide de sentido "Subir"
#define SolenoideDescer 8                 // Acionamento para a valvula solenoide de sentido "Descer"

#define ContatoraMotor 3                  // Acionamento para a contatora

#define FCAberto 4                        // Fim de curso que identifica o Led Wall Fift "Aberto".
#define FCFechado 5                       // Fim de curso que identifica o Led Wall Fift "Fechado".

#define SensorWeather A3                  // Pino de leitura do sensor de velocidade do vento (Weather)

#define CSAberto A4                       // Fim de curso que identifica o Led Wall Fift "Fechado" (Contato seco).
#define CSFechado A5                      // Fim de curso que identifica o Led Wall Fift "Aberto" (Contato seco).

// --------------------------------------------------------------------------------------------------------------------------------------------------------  

unsigned long int TempoAbreFecha = 0;     // Variavel que guarda o momento.

int ultimoCmdTil = 0;

unsigned long int TBotao = 0;             // Variavel que guarda o momento do botão.

int TMotor = 0;                           // Estado inicial do TMotor.

int SDescer = HIGH;                       // Estado inicial do SDescer = SolenoideDescer (8).
int SSubir = HIGH;                        // Estado inicial do SSubir = SolenoideSubir (6).

int CMotor = HIGH;                        // Estado inicial do CMotor = ContatoraMotor (3).

unsigned long tempoMotorAtivado = 0;      // Variável para armazenar o tempo de ativação do motor.

bool motorEmMovimento = false;            // Flag para indicar se o motor está em movimento.

// --------------- IR --------------------------------------------------- IR  --------------------------------------------------- IR ----------------------

int RECV_PIN = A2;                        // Pino IR
float armazenavalor;                      // Variavel valor do IR

IRrecv irrecv(RECV_PIN);
decode_results results;

// --------------- CC --------------------------------------------------- CC  --------------------------------------------------- CC ----------------------

int ccoState1;                            // A leitura atual do pino de entrada
int lastButtonState1 = LOW;               // A leitura anterior do pino de entrada
unsigned long lastDebounceTime1 = 0;      // A última vez que o pino de saída foi alternado
unsigned long debounceDelay1 = 50;        // the debounce time; increase if the output flickers

int ccoState2;                            // A leitura atual do pino de entrada
int lastButtonState2 = LOW;               // A leitura anterior do pino de entrada
unsigned long lastDebounceTime2 = 0;      // A última vez que o pino de saída foi alternado
unsigned long debounceDelay2 = 50;        // the debounce time; increase if the output flickers

// ------------ WEATHER ----------------------------------------------- WEATHER  ----------------------------------------------- WEATHER ------------------

int ccoState3;                            // A leitura atual do pino de entrada
int lastButtonState3 = LOW;               // A leitura anterior do pino de entrada
unsigned long lastDebounceTime3 = 0;      // A última vez que o pino de saída foi alternado
unsigned long debounceDelay3 = 50;        // O tempo de rejeição; aumentar se a saída piscar

// --------------- RF --------------------------------------------------- RF  --------------------------------------------------- RF ----------------------

RCSwitch mySwitch = RCSwitch();

// --------------------------------------------------------------------------------------------------------------------------------------------------------  

void setup(){

  pinMode(SolenoideDescer, OUTPUT);          // Define o pino que vai para a Solenoide do sentido "Descer" como uma saida.
  pinMode(SolenoideSubir, OUTPUT);           // Define o pino que vai para a Solenoide do sentido "Subir" como uma saida.
  pinMode(ContatoraMotor, OUTPUT);           // Define o pino que vai para a Contatora como saida.
  
  pinMode(FCFechado, INPUT);                 // Define o pino que vai para o Fim de Curso "Fechado" como uma entrada.
  pinMode(FCAberto, INPUT);                  // Define o pino que vai para o Fim de Curso "Aberto" como uma entrada.
  
  pinMode(CSAberto, INPUT);                  // Define o pino que vai para o Contato Seco "Aberto" como uma entrada.
  pinMode(CSFechado, INPUT);                 // Define o pino que vai para o Contato Seco "Fechado" como uma entrada.
  
  pinMode(SensorWeather, INPUT);             // Define o pino que vai para o Weather como entrada.
  
  digitalWrite(SolenoideDescer, SDescer);    // Define o estado inicial da Solenoide com acionamento para "Descer".
  digitalWrite(SolenoideSubir, SSubir);      // Define o estado inicial da Solenoide com acionamento para "Subir".
  
  digitalWrite(ContatoraMotor, CMotor);      // Define o estado inicial da Contatora.

  irrecv.enableIRIn();                       // Inicializa o receptor IR.

  mySwitch.enableReceive(0);                 // Indica que a interrupção 0 está associada ao pino 2 do microcontrolador.
  
  Serial.begin(115200);                      // Taxa de atualização da Serial.

}

// --------------------------------------------------------------------------------------------------------------------------------------------------------  

void loop() {
ContatoSAbrir();
ContatoSFechar();
Weather();
TPMotor();

// --------------- LOOP DO SERIAL --------------------------------------------------- LOOP DO SERIAL ------------------------------------------------------
// Utilizado apenas ao conectar no computador!

      if (Serial.available() > 0) {
        char letra = Serial.read();
            if (letra == 'i'){                 // Comando para "SUBIR" o LED WALL LIFT via SERIAL.
                    Subir();
                    Serial.print("Comando para SUBIR o LED WALL LIFT via SERIAL foi acionado");
                    
            } else if (letra == 'o'){          // Comando para "DESCER" o LED WALL LIFT via SERIAL.
                    Descer();
                    Serial.print("Comando para DESCER o LED WALL LIFT via SERIAL foi acionado"); 
                    
            } else if (letra == 'p'){          // Comando para "PARAR" o LED WALL LIFT via SERIAL.
                    PararGeral();
                    Serial.print("Comando para PARAR o LED WALL LIFT via SERIAL foi acionado");
       } 
}
// --------------- LOOP DO RF --------------------------------------------------- LOOP DO RF --------------------------------------------------------------

    if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();
     if (value == 0){
     Serial.print("Codigo desconhecido");
     } else {
      
      if (mySwitch.getReceivedValue() == 1200031) {             // Comando para "SUBIR" o LED WALL LIFT via RF.
           if (millis() - TBotao >= 650){
               Subir();  
           }   
      } else if (mySwitch.getReceivedValue() == 1200032) {      // Comando para "DESCER" o LED WALL LIFT via RF.
           if (millis() - TBotao >= 650){
               Descer();     
           }
      } else if (mySwitch.getReceivedValue() == 1200033) {      // Comando para "PARAR" o LED WALL LIFT via RF.
           if (millis() - TBotao >= 650){
               PararGeral();
           }     
      }        
   }
         
    Serial.print("Recebido");
    Serial.println( mySwitch.getReceivedValue() ); //Informa o valor do codigo RF.
    
    mySwitch.resetAvailable();
  }

// --------------- LOOP DO IR --------------------------------------------------- LOOP DO IR -------------------------------------------------------------- 
 
  if (irrecv.decode(&results)) {
    
    Serial.print("Valor lido : ");
    Serial.println(results.value, HEX);
    armazenavalor = (results.value);
    
    if (armazenavalor == 0x202708F || armazenavalor == 0x6EDFE961)            // Comando para "SUBIR" o LED WALL LIFT via IR.
    {
      if (millis() - TBotao >= 500){
          Subir();
      }
    }
    
    else if (armazenavalor == 0x202D02F || armazenavalor == 0x76B366E3 )      // Comando para "DESCER" o LED WALL LIFT via IR.
    {
      if (millis() - TBotao >= 500){
          Descer();
      }
    }
    
    else if (armazenavalor == 0x923F150B || armazenavalor == 0x202B04F)       // Comando para "PARAR" o LED WALL LIFT via IR.
    {
      if (millis() - TBotao >= 500){
          PararGeral();
      }      
    }
    
    irrecv.resume();
  }

 if (ultimoCmdTil > 0){
   
      if (digitalRead(FCAberto) == LOW) {
            if (ultimoCmdTil == 2){
             PararGeral();
            }
          }
      if (digitalRead(FCFechado) == LOW ) {
            if (ultimoCmdTil == 1){
             PararGeral();
            }
    }
  }   
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void Subir() {
   TBotao = millis();

   CMotor = LOW;
   digitalWrite(ContatoraMotor, CMotor);

   SDescer = LOW;                            // Desaciona a Solenoide para o LED WALL LIFT "Descer". 
   SSubir = HIGH;                            // Aciona a Solenoide para o LED WALL LIFT "Subir".                       
   
   delay(800);                               // Tempo de internalo entre o acionamento do motor apor o freio ser liberado.
   
   digitalWrite(SolenoideDescer, SDescer);   // Define o estado inicial da Solenoide com acionamento para "Descer".
   digitalWrite(SolenoideSubir, SSubir);     // Define o estado inicial da Solenoide com acionamento para "Subir".;
   
   TMotor = 1;

   motorEmMovimento = true;                  // Indica que o motor está em movimento.
   tempoMotorAtivado = millis();             // Marca o tempo de ativação do motor.  
          
   Serial.println("Comando para SUBIR o LED WALL LIFT foi acionado");
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void Descer() {
   TBotao = millis();

   CMotor = LOW;
   digitalWrite(ContatoraMotor, CMotor);
             
   SDescer = HIGH;                           // Acionado a Solenoide para o LED WALL LIFT "Descer".                              
   SSubir = LOW;                             // Desaciona a Solenoide para o LED WALL LIFT "Subir".                         
  
  delay(800);                                // Tempo de internalo entre o acionamento do motor apor o freio ser liberado.

  digitalWrite(SolenoideDescer, SDescer);    // Define o estado inicial da Solenoide com acionamento para "Descer".
  digitalWrite(SolenoideSubir, SSubir);      // Define o estado inicial da Solenoide com acionamento para "Subir".

   TMotor = 1;
         
   motorEmMovimento = true;                  // Indica que o motor está em movimento.
   tempoMotorAtivado = millis();             // Marca o tempo de ativação do motor.

   Serial.println("Comando para DESCER o LED WALL LIFT foi acionado");  
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void PararGeral() {
   TBotao = millis();

   SDescer = HIGH;                           // Desaciona a Solenoide para o LED WALL LIFT "Descer".                           
   SSubir = HIGH;                            // Desaciona a Solenoide para o LED WALL LIFT "Subir".                     
   CMotor = HIGH;                            // Desaciona a Contatora para o LED WALL LIFT.
         
   digitalWrite(SolenoideDescer, SDescer);    // Define o estado inicial da Solenoide com acionamento para "Descer".
   digitalWrite(SolenoideSubir, SSubir);      // Define o estado inicial da Solenoide com acionamento para "Subir".               
   digitalWrite(ContatoraMotor, CMotor);

   TMotor = 0;

   motorEmMovimento = false;                 // Para indicar que o motor não está mais em movimento.   

   Serial.println("Comando para PARAR o LED WALL LIFT foi acionado");
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void ContatoSAbrir(){  // declaração da função espera 
   int reading1 = digitalRead(CSAberto);

    if (reading1 != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis(); }

      if ((millis() - lastDebounceTime1) > debounceDelay1) {

       if (reading1 != ccoState1) {
          ccoState1 = reading1;

         if (ccoState1 == LOW) {

            if (millis() - TBotao >= 500){
              Subir();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState1 = reading1;
 
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void ContatoSFechar(){  // declaração da função espera 
   int reading2 = digitalRead(CSFechado);

    if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis(); }

      if ((millis() - lastDebounceTime2) > debounceDelay2) {

       if (reading2 != ccoState2) {
          ccoState2 = reading2;

         if (ccoState2 == LOW) {

            if (millis() - TBotao >= 500){
               Descer();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState2 = reading2;
     //Serial.println("ContatoSeco() Executou");
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- 

void Weather(){  // declaração da função espera 
   int reading3 = digitalRead(SensorWeather);

    if (reading3 != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis(); }

      if ((millis() - lastDebounceTime3) > debounceDelay3) {

       if (reading3 != ccoState3) {
          ccoState3 = reading3;

         if (ccoState3 == LOW) {

            if (millis() - TBotao >= 500){
               Subir();
            }
        }
     }
  }
     // save the reading. Next time through the loop, it'll be the lastButtonState:
     lastButtonState3 = reading3;
     //Serial.println("ContatoSeco() Executou");
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// Usado para controlar e saber o estado dos dois Fim de Curso, seja eles o Aberto, quanto o Fechado.
void TPMotor() { 
    
         // Verifica se o motor está ligado por mais de 4 minutos sem acionar nenhum fim de curso.
  if (motorEmMovimento && (millis() - tempoMotorAtivado >= 240000)) {
    PararGeral();
    Serial.println("Motor foi desligado automaticamente após 4 minutos sem atingir o fim de curso.");
  }
 }

// --------------------------------------------------------------------------------------------------------------------------------------------------------
