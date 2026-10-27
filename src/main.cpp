#include <Arduino.h>

#include "Olgalimex/_config.h"
#include "Olgalimex/_ethernet.h"
#include "Olgalimex/_wifi.h"
#include "Olgalimex/_dmx.h"
#include "Olgalimex/_artnet.h"
#include "Olgalimex/_osc.h"
#include "Olgalimex/_web.h"

int motorEnablePin = GPIO_NUM_8;
int motorDirPin = GPIO_NUM_5;
int motorPulsePin = GPIO_NUM_6;

int bubblesPin = GPIO_NUM_3;

#include "FastAccelStepper.h"
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
unsigned int bootBubbles = 0;

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    const float run = in_max - in_min;
    if(run == 0) { return out_min;}
    const float rise = out_max - out_min;
    const float delta = x - in_min;
    return (delta * rise) / run + out_min;
}

void onArtnetFrame(uint16_t universeRcv, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
}

void onDmxFrame()
{
}

void triggerTriggered(String str) {
  if (str == "start") {
    stepper->runForward();
  } else if (str == "stop") {
    stepper->stopMove();
  } else if (str == "startBubbles") {
	if (config["bubblesBootDuration"].as<int>() > 0) {
		bootBubbles = millis() + config["bubblesBootDuration"].as<int>();
		analogWrite(bubblesPin, config["bubblesBootSpeed"].as<int>());
	} else {
		analogWrite(bubblesPin, config["speedBubbles"].as<int>());
	}
  } else if (str == "stopBubbles") {
	analogWrite(bubblesPin, 0);
  }
}


void configUpdatedMain(String key)
{
  if (key == "speed") {
    stepper->setSpeedInHz(config["speed"].as<int>());
  }
  if (key == "speedBubbles") {
	analogWrite(bubblesPin, config["speedBubbles"].as<int>());
	stepper->setSpeedInHz(config["speedBubbles"].as<int>());
  }
}

void setup() {
	// generic parameters
	oscPort = 9004;
	chipName = "Aquarium 32N";
	eepromMarker = 0xFB;
	// config : read and write parameters
	// infos : read only parameters

	pinMode(bubblesPin, OUTPUT);
	//analogWriteFrequency(10000);
	analogWrite(bubblesPin, 0);

	addIntConfig("speed", 8000);
	addTrigger("start");
	addTrigger("stop");
	addIntConfig("speedBubbles", 255);	
	addIntConfig("bubblesBootSpeed", 255);
	addIntConfig("bubblesBootDuration", 100);
	addTrigger("startBubbles");
	addTrigger("stopBubbles");


	Serial.begin(115200);
	initConfig();

  
	//setupEthernet();
	setupWifi();
	setupOSC();
	setupWebServer();
	//setupArtnet();
	receivePin = 36;
	//setupDMX();

	oscSubscribe("/speed", [](const OscMessage& m) { if (m.size()>0) { stepper->setSpeedInHz(m.arg<float>(0)); stepper->runForward(); } });
	oscSubscribe("/speedBubbles", [](const OscMessage& m) { if (m.size()>0) { analogWrite(bubblesPin, m.arg<float>(0)); } });


	engine.init();
	stepper = engine.stepperConnectToPin(motorPulsePin);
	if (stepper) {
		stepper->setDirectionPin(motorDirPin);
		stepper->setEnablePin(motorEnablePin);
		stepper->setAutoEnable(true);

		stepper->setSpeedInHz(config["speed"].as<int>());
		stepper->setAcceleration(8000);
	   	//stepper->enableOutputs();
	}

}


void loop()
{
	if (bootBubbles >0 && bootBubbles < millis()) {
		bootBubbles = 0;
		analogWrite(bubblesPin, config["speedBubbles"].as<int>());
	}
	vTaskDelay(5);
}
