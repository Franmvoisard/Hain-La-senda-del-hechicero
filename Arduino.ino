#include <IRremote.h>

const byte _pinLedA[3] = {6,10,9};

const long INTERVALO_LARGO = 10 * 1000;
const long INTERVALO_CORTO = 4 * 1000;

const byte RED[3] = {255,0,0};
const byte GREEN[3] = {0,255,0};
const byte BLUE[3] = {0,0,255};

byte _lastColor[3] = {0,0,0};
byte _toColor[3] = {0,0,0};
byte _actualColor[3] = {0,0,0};

long _now, _last = 0, _transColor = 0;
long _blinkTime;
boolean _changingColor = false, _blinking = false, _blinkOff = true;
int _colorIndex = 0, _countBlink, _totalBlink = 0;

int IRpin = 2;
IRrecv irrecv(IRpin);
decode_results results;

void setup() {
  Serial.begin(9600);
  for(int i = 0; i < 3; i++) {    // Seteas los pines de los leds como salida en baja
    pinMode(_pinLedA[i], OUTPUT);
    analogWrite(_pinLedA[i], LOW);
  }
  
  irrecv.enableIRIn(); //Inicializar la recepción IR
}
  

void loop() {
  receiveIR();
  refreshLeds();
}  

void receiveIR() {
  // Aca el llamado a la funcion que lee el IR, te guardas el valor en cmd para saber para donde switchear
  if (irrecv.decode(&results)){
  int cmd = results.value;
  Serial.print(cmd);
  switch(cmd){
    case 2064:
      changeColor(INTERVALO_LARGO, BLUE); // Segun lo que toque, llamas a una funcion que SOLAMENTE LEVANTA BANDERAS
      break;
	case 2065:
	  blinkOnColor(BLUE, INTERVALO_CORTO, 3);	// Parpadear 3 veces, en intervalos cortos, desde blue a apagado.
	  break;
  case 2080:
    blinkOnColor(RED, INTERVALO_CORTO,2);
    break;
		
  }
  irrecv.resume();
}
}

void refreshLeds() {
  if(_changingColor) {    // Solo si la bandera esta levantada
	refreshChange();
  } else if(_blinking) {
	refreshBlink();
  }
  for(int i = 0; i < 3; i++) {	// El color actual siempre esta guardado en _actualColor, por lo cual actualizo.
	analogWrite(_pinLedA[i], _actualColor[i]);
  }
}

void changeColor(long inTime, byte toColor[3]) {  // Recibe en cuanto tiempo debe durar la transicion de color, y hacia que color
    _transColor = inTime;                          // Guardo en global la duracion de la transicion
    for(int i = 0; i < 3; i++) {
      _toColor[i] = toColor[i];                   // Guardo en global el color pasado como parametro
      _lastColor[i] = _actualColor[i];            // Guardo en _lastColor el valor del color actual
    }
    _changingColor = true;                        // Levanto la bandera que avisa que hay que prender generar una transicion con el led
    _colorIndex = 0;                              // Esto es para llevar un conteo de que tanto hay que alterar el color
    _last = millis();                             // Tomo la muestra de tiempo antes de empezar
}

void blinkOnColor(byte onColor[3], long blinkTime, int blinks) {
	_blinking = true;
	_blinkTime = blinkTime;
	_totalBlink = blinks;
	_countBlink = 0;
	for(int i = 0; i < 3; i++) {
		_lastColor[i] = onColor[i];	// Me guardo el color sobre el que debo parpadear
	}
	_last = millis();
}

void refreshBlink() {
	_now = millis();
	if(_now - _last < _blinkTime) {
		if(_countBlink >= _totalBlink){		// Si parpadeo todas las veces que se le pidio
			_blinking = false;				// deja de parpadear
		} else {
			int fraction = floor((_blinkTime / (_now - _last)));	// Sino, calcula la fraccion de tiempo que paso respecto de el tiempo por parpadeo total
			if(_blinkOff) {											// Si tiene color, y deberia ir hacia apagado
				for(int i = 0; i < 3; i++)
					_actualColor[i] = _lastColor[i] - (_lastColor[i] / fraction);	// Al color con el que comenzo, se le resta la 
																					//fraccion del color correspondiente al tiempo transcurrido
			} else {												// Si llego a apagarse, y se deberia estar encendiendo
				for(int i = 0; i < 3; i++)
					_actualColor[i] = (_lastColor[i] / fraction);
			}
		}
	} else if(_now - _last >= _blinkTime) {		// Cuando termino la transicion, hacia apagado u encendido
		_last = millis();						// Se toma un nuevo muestreo para contar el la otra parte del parpadeo
		_blinkOff = !_blinkOff;					// Si se estaba apagando, ahora se va a prender, o viceversa
		if(_blinkOff)								// Hago el conteo de parpadeo solo para cuando prende, o solo para cuando apaga.
			_countBlink++;
	}
}

void refreshChange() {
	_now = millis();      // Tomo el muestro de tiempo actual
    if(_now - _last <= _transColor){     // Si la diferencia es menor/igual a lo que deberia durar la transicion
      _colorIndex = floor(_transColor / (_now - _last));      // Me guardo en _colorIndex el denominador de la fraccion que representa la transicion
      for(int i = 0; i < 3; i++) {                    // Si la transicion debe durar 10 segundos, y pasaron 5, _colorIndex = 2, entonces la diferencia entre el color al 
        _actualColor[i] = ((_toColor[i] - _lastColor[i]) / _colorIndex) + _lastColor;    // que se quiere llegar, y al que se tenia en un comienzo, es el total / 2, la mitad.
      }
    } else {                  
      _changingColor = false;
      for(int i = 0; i < 3; i++) {
        _lastColor[i] = _actualColor[i];
      }
    }
}
