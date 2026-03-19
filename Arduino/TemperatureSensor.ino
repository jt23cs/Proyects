const int potenciometro = 14;  //pin del potenciometro
const int led_alarma = 3;      //pin LED rojo salida PWM (afecta brillo)
const int led_ventilador = 6;  //pin LED azul salida PWM (afecta brillo)
const int btn_1 = 8;             // pin botón
enum btn_state_t { BTN_RELEASED,
                   BTN_PRESSED_EVENT,
                   BTN_PRESSED,
                   BTN_RELEASED_EVENT };  // estados del boton

void setup() {
  Serial.begin(9600);
  pinMode(led_alarma, OUTPUT);
  pinMode(led_ventilador, OUTPUT);
  pinMode(btn_1, INPUT_PULLUP);
}

void blink_led_ventilador(int led_pin, float duty_cycle, float temperatura, unsigned long t_on = 1, unsigned long t_off = -1) {
  /*Recibe un pin,brillo,valor t ventialador  ,tiempo encendido,tiempo apagado -> int,float,int,int 
    Enciede y apaga el led segun el tiempo de encendido y apagado 
    Retorna nada
    control ventilador para establecer el intervalo en donde funciona 
  */
  if (t_off == -1) t_off = t_on;
  static long t_inicial = millis();
  static bool status = false;  // encendido si true, apagado si false
  unsigned long t_ahora = millis();
  if (status == true && t_ahora - t_inicial > t_on && comprobar_ventilador(temperatura,-5,45) == false){
    analogWrite(led_pin, duty_cycle);
    status = false;
    t_inicial = millis();
  } else if (status == false && t_ahora - t_inicial > t_off) {
    analogWrite(led_pin, 0.0);
    status = true;
    t_inicial = millis();
  }
}

bool comprobar_ventilador(float control_ventilador, float min, float max){
    // valores en donde funciona el ventilador
    return control_ventilador < min || control_ventilador > max;
}


float linear_map(float in, float in_min, float in_max, float out_min, float out_max) {
  /*
  Recibe valor entrada/lectura analógica
  Calculo de la tensión e impresión de su valor por segundo
  Devuelve la magnitud de entrada escalada
  */
  float salida =0, sensibilidad=0;
  sensibilidad = (out_max - out_min) / (in_max - in_min);
  salida = (sensibilidad * (in - in_min)) + out_min;
  return salida;
}


void print_btn_state(btn_state_t act_state) {
  /*
  Recibe el estado del boton (LOW o HIGH), pin del boton
  Definir 4 estados de un boton
  Devolver nuevo estado
    *BTN_RELEASED, BTN_PRESSED_EVENT, BTN_PRESSED, BTN_RELEASED_EVENT
  */
  //modificar evitar infinitas ejecuciones
  static btn_state_t prev_state = BTN_RELEASED_EVENT;
  if (act_state != prev_state) {
    switch (act_state) {
      case BTN_RELEASED: Serial.println("BTN_RELEASED"); break;
      case BTN_PRESSED_EVENT: Serial.println("BTN_PRESSED_EVENT"); break;
      case BTN_PRESSED: Serial.println("BTN_PRESSED"); break;
      case BTN_RELEASED_EVENT: Serial.println("BTN_RELEASED_EVENT"); break;

      default: Serial.println("NO STATE DEFINED"); break;
    }
    prev_state = act_state;
  }
}

btn_state_t btn_state_handler(btn_state_t act_state, int btn_pin) {
  /*
  Recibe el estado del boton (btn_state_t), pin del boton
  Definir 4 estados de un boton
  Devolver nuevo estado
    *BTN_RELEASED, BTN_PRESSED_EVENT, BTN_PRESSED, BTN_RELEASED_EVENT 0,1,2,3
  */
  const unsigned long t_captacion = 200; // 50 ms típico
  int lectura = digitalRead(btn_pin);
  // Esperar a que el estado se mantenga estable por un tiempo
  if (millis() > t_captacion) {

  if (act_state == BTN_RELEASED && lectura == LOW) {
    return BTN_PRESSED_EVENT;
  } else if (act_state == BTN_PRESSED_EVENT && lectura == LOW) {
    return BTN_PRESSED;
  } else if (act_state == BTN_PRESSED && lectura == HIGH) {
    return BTN_RELEASED_EVENT;
  } else if (act_state == BTN_RELEASED_EVENT && lectura == HIGH) {
    return BTN_RELEASED;
  }
  }
  return act_state;
}

void btn_press_lighting(int duty_cycle, int btn_pin, btn_state_t act_state, int frequency, unsigned long time = 2000) {
  // 255/5 = 51 es el duty cycle en un potenciometro al 20%
  // frequency es la cantidad de veces que se puede aumentar hasta el reinicio
  static int i = 1;
  unsigned long timer;
  if (btn_state_handler(act_state, btn_pin) == BTN_PRESSED_EVENT) {
    analogWrite(btn_pin, i * duty_cycle);
    i++;
    if (i == frequency) i = 1;
  }
  if (btn_state_handler(act_state, btn_pin) == BTN_PRESSED_EVENT) {
    static unsigned long timer = millis();
  }
  if (btn_state_handler(act_state, btn_pin) == BTN_PRESSED && (millis() - timer) > time) {
    analogWrite(btn_pin, frequency * duty_cycle);
  }
  if (btn_state_handler(act_state, btn_pin) == BTN_RELEASED_EVENT) {
    digitalWrite(btn_pin, HIGH);
  }
}

int error_control(float temperature, float temper_max, float temper_min,unsigned int error_sample, unsigned long time_check,int status = 0){
  // temperature, 55,45,5,3,2000 
    static unsigned long t_ahora = millis();
    static float muestra_temp[5];
    static unsigned int j = 0,i =0; // j -> muestras fuera de rango  i -> indice de la muestra
    
    muestra_temp[i] = temperature;
    if (muestra_temp[i] >= temper_min && muestra_temp[i] <= temper_max && millis() - t_ahora > time_check){
      j++;
      t_ahora = millis();
    }
    i++;
    if(i==5) i=0;
    if (j == error_sample){
      status = 1;  // Cambia a Estado Error
      Serial.println("Estado Error");
      j=0;
    }
    return status;
}

//Control de impresión por pantalla (funcion)
void print_temperature_control(btn_state_t state, unsigned long time_press,float temperature,int btn){
  static unsigned long t_ahora = millis();
  static long stastus = 0; // comprobar que se pase por los diferenetes puntos
  bool pressed =true;
  // Pulsar boton
  if (state == BTN_RELEASED && millis() - t_ahora > 2000) {
    //Serial.println(temperature);
    t_ahora = millis();
  }
  //print_btn_state(act_state);
   if(state == BTN_PRESSED_EVENT){
    Serial.println(temperature);
  }
}

int running_status(float temperature,float duty_cycle){
  //Control del parpadeo del LED azul.
  blink_led_ventilador(led_ventilador, duty_cycle,temperature, 1000, 1000);

  //Control del estado de ERROR 0 estado running 1 estado error con numero de muestra 5
  int new_status = error_control(temperature,55,45,3,2000);
  if(new_status != 0) return new_status;

  //Control de impresión por pantalla
  btn_state_t act_state = btn_state_handler(act_state, btn_1);
  print_temperature_control(act_state,1000 ,temperature,btn_1);

  // Control del LED alarma
  if(analogRead(led_alarma) >0){
    analogWrite(led_alarma,0);
  }
  return 0;
}

void control_alarma(int parpadeos,int led_alarma,unsigned int t_parpadeo = 1000){
  int i=0,j=0;
  unsigned long t = millis();
  if(i<=parpadeos && millis() > t_parpadeo) {
    analogWrite(led_alarma,255 * i/parpadeos);
    i++;
    t=millis();
  } 
  else if (parpadeos>=j && millis() > t_parpadeo) {
    analogWrite(led_alarma,255 * (i-j)/parpadeos);
    j++;
    t=millis();
  }
}
int running_control(int status){
  if(digitalRead(btn_1)==LOW){
    status=1;
  }
  return status;
}
int error_status(int status){
  //Control del LED Rojo o alarma
  control_alarma(5,led_alarma);

  //Control del estado RUNNING
  int new_status=running_control(status);
  if(new_status != 1) return 0;

  //Control del LED azul o ventilador
  if(analogRead(led_ventilador) >0){
    analogWrite(led_ventilador,0);
  }

  //Control de impresión por pantalla
  


  return 1;
}

void loop() {
  static btn_state_t act_state = BTN_RELEASED;
  static int status = 0;  // 0 estado running 1 estado error
  float temperature = linear_map(analogRead(potenciometro), 0, 1023, -15, 65);
  float duty_cycle = linear_map(analogRead(potenciometro), 0, 1023, 0, 255);

  print_btn_state(act_state);
  // ESTADO RUNNING
  if (status == 0) {
    running_status(temperature,duty_cycle);
  }
  // ESTADO DE ERROR
  else error_status(status);
}
