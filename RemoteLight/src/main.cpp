#include "RemoteLight.h"

RemoteLight *rml;

void setup() {

  rml = new RemoteLight();
  rml->init();
}

void loop() {
  rml->run();
  delay(10);
}