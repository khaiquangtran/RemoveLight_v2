#include "./Processor.h"

Processor *processor;

void setup() {
  processor = new Processor();
}

void loop() {
  processor->run();
  delay(200);
}
