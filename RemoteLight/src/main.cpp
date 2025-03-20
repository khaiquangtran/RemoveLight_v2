#include "RemoteLight.h"

std::shared_ptr<RemoteLight>rml;

void setup() {

  rml = std::make_shared<RemoteLight>();
  rml->init();
}

void loop() {
  rml->run();
  delay(10);
}