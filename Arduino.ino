#include <IRremote.h>

const byte _pinLedA[3] = {6,10,9};
const byte IRpin = 2;	// El pin que se usa no cambia durante la ejecucion, la buena practica es que sea
// una constante. Como la cantidad de pines del arduino es pequeña, utilizar byte en vez de int permite
// ahorrar memoria.

// Definir intervalos en millis
const long INTERVALO_LARGO = 10 * 1000;
const long INTERVALO_CORTO = 4 * 1000;

// Definir los colores a utilizar
const byte RED[3] = {255,0,0};
const byte GREEN[3] = {0,255,0};
const byte BLUE[3] = {0,0,255};

// _actualColor debe estar tener el valor default del LED
byte _actualColor[3] = {0,0,0};
byte _lastColor[3] = {0,0,0};
byte _toColor[3] = {0,0,0};


long _now, _last = 0, _transColor = 0;
long _blinkTime;
boolean _changingColor = false, _blinking = false, _blinkOff = true;
float _colorIndex = 0, _countBlink = 0, _totalBlink = 0, fraction = 0;

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
  Serial.print("Color: (");
  for(byte i = 0; i < 3; i++) {
  	Serial.print(_actualColor[i]);
  	Serial.print(", ");
  }
  Serial.println(")");
}  

void receiveIR() {
  	if (irrecv.decode(&results)){
	  int cmd = results.value;
	  Serial.print(cmd);
	  switch(cmd){
	    case 2064:	// Cambia del color actual, al seleccionado, en el tiempo definido
	      changeColor(INTERVALO_LARGO, BLUE);
	      break;
		case 2065:	// Parpadea n veces, desde el primer color al segundo, con x tiempo entre colores
		  blinkOnColor(BLUE, RED, INTERVALO_CORTO, 3);
		  break;
	  	case 2080:
	      blinkOnColor(RED, BLUE, INTERVALO_CORTO,2);
		  break;
			
	  }
	  irrecv.resume();
	}
}

void refreshLeds() {
  if(_changingColor) {
	refreshChange();
  } else if(_blinking) {
	refreshBlink();
  }
}

void changeColor(long inTime, byte toColor[3]) {  // Recibe cuanto tiempo debe durar la transicion de color, y hacia que color
    _transColor = inTime;
    for(int i = 0; i < 3; i++) {
      _toColor[i] = toColor[i];
      _lastColor[i] = _actualColor[i];
    }
    _changingColor = true;
    _colorIndex = 0;
    _last = millis();
}

void blinkOnColor(byte fromColor[3], byte toColor[3], long blinkTime, int blinks) {
	_blinking = true;
	_blinkTime = blinkTime;
	_totalBlink = blinks;
	_countBlink = 0;
	for(int i = 0; i < 3; i++) {
		_lastColor[i] = fromColor[i];
		_toColor[i] = toColor[i];
	}
	_last = millis();
}

void refreshBlink() {
	_now = millis();
	if(_now - _last <= _blinkTime) {
		if(_countBlink >= _totalBlink){		// Si parpadeo todas las veces que se le pidio
			_blinking = false;				// deja de parpadear
			return;
		} else {
			fraction = (( _now - _last) * 100) / _blinkTime;	// El porcentaje de tiempo transcurrido respecto al seteado
			if(_blinkOff) {
				for(int i = 0; i < 3; i++)
					_actualColor[i] = _lastColor[i] + ((_toColor[i] - _lastColor[i]) * (fraction / 100));
			} else {
				for(int i = 0; i < 3; i++)
					_actualColor[i] = _toColor[i] + ((_lastColor[i] - _toColor[i]) * (fraction / 100));
			}
		}
	} else {		// Cuando termino la transicion, hacia apagado u encendido
		_last = millis();		// Se toma un nuevo muestreo para contar la otra parte del parpadeo
		_blinkOff = !_blinkOff;	// Si se estaba apagando, ahora se va a prender, o viceversa
		if(_blinkOff)			// Hago el conteo de parpadeo solo para cuando prende, o solo para cuando apaga.
			_countBlink++;
	}
}

void refreshChange() {
	_now = millis();
    if(_now - _last <= _transColor){
      _colorIndex = ((_now - _last) * 100) / _transColor;
      for(int i = 0; i < 3; i++) {
        _actualColor[i] = _lastColor[i] + ((_toColor[i] - _lastColor[i])  * (_colorIndex / 100));
      }
    } else {                  
      _changingColor = false;
      for(int i = 0; i < 3; i++) {
      	_actualColor[i] = _toColor[i];
        _lastColor[i] = _actualColor[i];
      }
    }
}