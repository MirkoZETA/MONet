#include "event.hpp"
Event::Event(void) {
  this->type = PERIOD_UPDATE;
  this->period = -1;
}

Event::Event(eventType type, double period) {
  this->type = type;
  this->period = period;
}

eventType Event::getType(void) const {
  return this->type;
}
double Event::getPeriod(void) const {
  return this->period;
}
