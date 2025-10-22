#ifndef __EVENT_H__
#define __EVENT_H__

// TODO: Implement failure events
typedef enum eventType {
  PERIOD_UPDATE,
  CONNECTION_FAILURE,
  LINK_FAILURE,
  NODE_FAILURE,
  FIBER_FAILURE,
  LINK_RECOVERY,
  NODE_RECOVERY,
  FIBER_RECOVERY
} eventType;

/**
 * @brief Class Event
 *
 */
class Event {
 public:
  /**
   * @brief Constructs a new Event object with default values for it's
   * attributes: (Event Type: ARRIVE, Id Connection: -1, Time: -1)
   */
  Event(void);
  /**
   * @brief Constructs a new Event object for period update events in incremental
   * simulation mode. These events trigger demand updates and resource allocation
   * at the beginning of each provisioning period.
   *
   * @param type (eventType): the type of current Event object, must be PERIOD_UPDATE.
   *
   * @param time (double): the actual time at which the period update Event occurs.
   */
  Event(eventType type, double time);

  /**
   * @brief Gets the type attribute of the Event object. This represents the
   * kind of Event of the current Event object.
   *
   * @return (eventType): the type of Event (ARRIVE, DEPARTURE, or PERIOD_UPDATE).
   */
  eventType getType(void) const;

  /**
   * @brief Gets the time attribute of the Event object. This represents the
   * time at which the current Event object has occurred.
   *
   * @return (double): the actual time at which the Event has occurred.
   */
  double getPeriod(void) const;

 private:
  eventType type;
  double period;
};

#endif