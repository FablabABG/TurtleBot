/*
    TurtleBot Main Program

    El siguiente programa fue creado para el proyecto
    TurtleBot, un robot educativo de bajo costo para ser
    implementado en estudiantes de jardin y primer grado.
        
    Created 23-02-2022
    By Erickson Gallardo
    Modified 10-05-2022
    By Erickson Gallardo

*/


//#define DEBUG

#ifdef DEBUG
#define DEBUGPRINTLN(X) Serial.println(X);
#define DEBUGPRINT(X) Serial.print(X);
#else
#define DEBUGPRINTLN(X) // nothing
#define DEBUGPRINT(X)   // nothing 
#endif


#define AIA 9  // (pwm) pin 9 connected to pin A-IA
#define AIB 5  // (pwm) pin 5 connected to pin A-IB
#define BIA 10 // (pwm) pin 10 connected to pin B-IA
#define BIB 6  // (pwm) pin 6 connected to pin B-IB

// buttons
#define FORWARD_BTN A0
#define RIGHT_BTN A1
#define BACK_BTN A2
#define LEFT_BTN A3
#define WAIT_BTN A4
#define DELETE_BTN A5
#define START_BTN 4
#define BUZZER 8

#define MOTION_TIME 750
#define ROTATION_TIME 450
#define WAIT_TIME 1000

#define FORWARD_NOTE 1000
#define RIGHT_NOTE 1150
#define BACKWARD_NOTE 1300
#define LEFT_NOTE 1450
#define WAIT_NOTE 1600

byte SPEED = 150;  // change this (0-255) to control the SPEED of the motors
byte movements[50]; // list of moves
byte movements_count = 0;
byte STATE = 0;

/*
   STATE CODE
   -1 = ERROR
   0 = WHAIT TO INSTRUCTIONS
   1 = EXECUTING INSTRUCTIONS
   2 = DELETE INSTRUCTION
*/

void setup() { 
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  
  DEBUGPRINTLN("Inicializando pines Puente H");
  pinMode(AIA, OUTPUT);
  pinMode(AIB, OUTPUT);
  pinMode(BIA, OUTPUT);
  pinMode(BIB, OUTPUT);
  DEBUGPRINTLN("Iniciando Botones");
  pinMode(FORWARD_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(BACK_BTN, INPUT_PULLUP);
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(WAIT_BTN, INPUT_PULLUP);
  pinMode(DELETE_BTN, INPUT_PULLUP);
  pinMode(START_BTN, INPUT_PULLUP);
  DEBUGPRINTLN("Sonido inicio");
  resumeAlert();
  resumeAlert();
  DEBUGPRINTLN("Iniciando programa principal\n");
}

void loop() {

  /*
      BUTTON READ STATE

    BUTTONS NUMBER CODE
    FORWARD 1
    RIGHT 2
    BACK 3
    LEFT 4
    WAIT 5
    DELETE 6
    START 7
  */
  DEBUGPRINTLN("Iniciando Ciclo");
  if (STATE == 0) {
    DEBUGPRINTLN("Estado S = 0\n Esperando accion botones");
    byte btn_pressed = readButtons();
    delay(300);
    if (btn_pressed == 6) {
      DEBUGPRINTLN("Historial de botones borrado \n Cambiando a estado S = 2\n");
      STATE = 2;
    } else if (btn_pressed == 7) {
      DEBUGPRINTLN("Iniciar movimientos \n Cambiando a estado S = 1\n");
      STATE = 1;
    } else {
      addMovement(btn_pressed);

    }
  }
  /*
      EXECUTION STATE
  */
  if (STATE == 1) {
    DEBUGPRINTLN("Iniciando Estado S = 1\n Ejecutar ");
    if (movements_count == 0) {
      DEBUGPRINTLN("Error - No hay movimientos registrados");
      errorAlert();
    } else {
      DEBUGPRINTLN("Iniciando ejecución de movimientos\n");      
      okAlert();
      delay(1000);
      executeMovements();
    }
    STATE = 0;
  }
  /*
     DELETE STATE
  */
  if (STATE == 2) {
    DEBUGPRINTLN("Iniciando estado S = 2\n Borrar movimientos");
    if (movements_count == 0) {
      Serial.println("There aren't registeres movements");
      errorAlert();
    } else {
      deleteAlert();
      deleteMovements();

    }
    STATE = 0;
  }
}

void errorAlert() {
  tone(BUZZER, 550, 150);
  delay(175);
  tone(BUZZER, 550, 150);
  delay(175);
  tone(BUZZER, 550, 150);
  delay(175);
}

void okAlert() {
  tone(BUZZER, 500, 150);
  delay(175);
  tone(BUZZER, 750, 150);
  delay(175);
  tone(BUZZER, 1000, 150);
  delay(175);
}

void deleteAlert() {
  tone(BUZZER, 1000, 150);
  delay(160);
  tone(BUZZER, 750, 150);
  delay(160);
  tone(BUZZER, 500, 150);
  delay(160);
  tone(BUZZER, 250, 150);
  delay(160);
}

void stopAlert() {
  tone(BUZZER, 500, 400);
}

void pauseAlert() {
  tone(BUZZER, 2000, 150);
  delay(160);
  tone(BUZZER, 1500, 150);
  delay(160);
}

void resumeAlert() {
  tone(BUZZER, 1500, 150);
  delay(160);
  tone(BUZZER, 2000, 150);
  delay(160);
}

void endAlert() {
  tone(BUZZER, 1500, 150);
  delay(160);
  tone(BUZZER, 1750, 150);
  delay(160);
  tone(BUZZER, 2000, 150);
  delay(160);
  tone(BUZZER, 2000, 150);
  delay(160);
}

void deleteMovements() {
  for ( int i = 0; i < movements_count + 1; i++) {
    movements[i] = 0;
  }
  movements_count = 0;
  Serial.print("All moves have been deleted, count: "); Serial.println(movements_count);
}

void waitingFor(int total_time) {
  unsigned long checkpoint = millis();
  while (millis() - checkpoint < total_time) {
    if (readButton(DELETE_BTN) || readButton(START_BTN)) {
      break;
    }
  }
}

void getAction(byte action) {
  if (action == 1) {
    forward(MOTION_TIME);
    stopMotion(WAIT_TIME);

  } else if (action == 2) {
    right(ROTATION_TIME);
    stopMotion(WAIT_TIME);
  } else if (action == 3) {
    backward(MOTION_TIME);
    stopMotion(WAIT_TIME);
  } else if (action == 4) {
    left(ROTATION_TIME);
    stopMotion(WAIT_TIME);
  } else if (action == 5) {
    stopMotion(1000);
  }
}


void executeMovements() {
  DEBUGPRINTLN("Proceso: executeMovements");
  bool inPause = false;
  bool stopped = false;

  for ( int i = 0; i < movements_count; i++) {
    /*Serial.println(movements[i]);
      delay(250);
    */
    DEBUGPRINTLN("Ejecutando movimiento");    
    getAction(movements[i]);

    if (!digitalRead(START_BTN)) {
      Serial.println("execution paused");
      pauseAlert();
      inPause = true;
      while (inPause) {
        inPause = !digitalRead(START_BTN) == 1 ? false : true;
      }
      Serial.println("resumed execution ");
      resumeAlert();
      delay(500);
    }

    if (!digitalRead(DELETE_BTN)) {
      stopAlert();
      Serial.println("execution stopped");
      stopped = true;
      break;
    }
  }
  if (!stopped) {
    endAlert();
  }
  delay(200);
}

/*
  Añade un movimiento a la lista de ejecución, el
  arreglo cuenta con un limite de 50 posiciones, al
  llegar al limite se sobrescribe la ultima accion.

  @param movement, numero del 1 - 7 correspondiente a
  un movimiento en espesifico.
*/
void addMovement(byte movement) {
  movements[movements_count] = movement;
  movements_count = movements_count < 50 ? movements_count + 1 : movements_count;
  DEBUGPRINT("\n Movimiento ");
  DEBUGPRINT(movement);
  DEBUGPRINT(" añadido, Movimientos totales:");  
  DEBUGPRINTLN(movements_count);  

}

/*
  Activa un bucle infinito hasta la activación de
  un boton, cada boton es evaluado y codificado con
  un valor numerico correspondiente a una accion de 
  movimiento.

  @return valor numerico de 1 - 7 correspondiente a
  un movimiento en particular. 
*/
byte readButtons() {
  byte btn = 0;
  while (true) {
    if (!digitalRead(FORWARD_BTN)) {
      DEBUGPRINTLN("Btn: Adelante - Activado");
      btn = 1;
      tone(BUZZER, FORWARD_NOTE, 200);
      break;
    } else if (!digitalRead(RIGHT_BTN)) {
      DEBUGPRINTLN("Btn: Derecha - Activado");
      btn = 2;
      tone(BUZZER, RIGHT_NOTE, 200);
      break;
    } else if (!digitalRead(BACK_BTN)) {
      DEBUGPRINTLN("Btn: Atras - Activado");
      btn = 3;
      tone(BUZZER, BACKWARD_NOTE, 200);
      break;
    } else if (!digitalRead(LEFT_BTN)) {
      DEBUGPRINTLN("Btn: Izquierda - Activado");
      btn = 4;
      tone(BUZZER, LEFT_NOTE, 200);
      break;
    } else if (!digitalRead(WAIT_BTN)) {
      DEBUGPRINTLN("Btn: Esperar - Activado");
      btn = 5;
      tone(BUZZER, WAIT_NOTE, 200);
      break;
    } else if (!digitalRead(DELETE_BTN)) {
      btn = 6;
      DEBUGPRINTLN("Btn: Stop - Activado");
      break;
    } else if (!digitalRead(START_BTN)) {
      DEBUGPRINTLN("Btn: Ejecutar - Activado");
      btn = 7;
      break;
    }
  }
  return btn;
}

/*
  Evalua el estado de activación de un boton determinado

  @param btn, pin de coneccion del botón a evaluar
  @return pressed, valor buleano correspondiente a la 
  activación del botón
*/
bool readButton(int btn) {
  bool pressed = !digitalRead(btn) == 1 ? true : false;
  return pressed;
}

/*
  Activa el movimiento de los motores para realizar
  un movimiento frontal, manteniendo un tiempo de 
  inactividad antes de ejecutar otro movimiento

  @param t, valor entero que corresponde al tiempo de
  inactividad posterior de ejecutar el movimiento actual.
*/
void forward(unsigned int t) {
  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);

  waitingFor(t);
}

/*
  Activa el movimiento de los motores para realizar
  un movimiento en retroceso, manteniendo
  un tiempo de inactividad antes de ejecutar otro 
  movimiento

  @param t, valor entero que corresponde al tiempo de
  inactividad posterior de ejecutar el movimiento actual.
*/
void backward(unsigned int t) {
  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);

  waitingFor(t);
}

/*
  Activa el movimiento de los motores para realizar
  un giro de 90 grados en direccion izquierda, manteniendo
  un tiempo de inactividad antes de ejecutar otro 
  movimiento

  @param t, valor entero que corresponde al tiempo de
  inactividad posterior de ejecutar el movimiento actual.
*/
void left(unsigned int t) {
  analogWrite(AIA, SPEED);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, SPEED);

  waitingFor(t);
}

/*
  Activa el movimiento de los motores para realizar
  un giro de 90 grados en direccion derecha, manteniendo
  un tiempo de inactividad antes de ejecutar otro 
  movimiento

  @param t, valor entero que corresponde al tiempo de
  inactividad posterior de ejecutar el movimiento actual.
*/
void right(unsigned int t) {
  analogWrite(AIA, 0);
  analogWrite(AIB, SPEED);
  analogWrite(BIA, SPEED);
  analogWrite(BIB, 0);

  waitingFor(t);
}

/*
  Detiene el movimiento de los motores manteniendo
  un tiempo de inactividad antes de ejecutar
  otro movimiento

  @param t, valor entero que corresponde al tiempo de
  inactividad posterior de ejecutar el movimiento actual.
*/
void stopMotion(unsigned int t) {
  analogWrite(AIA, 0);
  analogWrite(AIB, 0);
  analogWrite(BIA, 0);
  analogWrite(BIB, 0);

  waitingFor(t);
}
