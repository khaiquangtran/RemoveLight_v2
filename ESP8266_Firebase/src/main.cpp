#include "./Processor.h"

Processor *processor = new Processor();

void setup() {
  processor->init();
}

void loop() {
  processor->run();
}
