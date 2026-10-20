#pragma once

#include "helpers.h"

using EventId = uint16_t;
using EventHandler = void (*)(void* userData, int context);

constexpr EventId INVALID_EVENT_ID = 0xFFFF;

struct ScheduledEvent_t
{
	int64_t timestamp;
	int64_t sequence;
	int      context;
	EventId  id;
};

static_assert(std::is_trivially_copyable_v<ScheduledEvent_t>);

class Scheduler_t
{
public:
	static constexpr uint32_t MAX_EVENT_TYPES = 32;
	static constexpr uint32_t MAX_EVENTS = 32;

	Scheduler_t() : eventDefCount(0) {
		reset();
	}

	// Called once per event type at startup by the owning module, lower priority value runs first on the same cycle
	EventId register_event(EventHandler handler, void* userData, uint8_t priority = 0);

	// Clears the event queue and resets the master cycle counter to zero, registered event types are kept
	void reset();

	int64_t get_current_cycles() const {
		RETURN currentCycles;
	}
	int64_t peek_next_timestamp() const {
		RETURN nextEventTime;
	}

	void schedule(EventId id, int64_t target_cycle, int context = 0);
	void cancel(EventId id, int context = 0);
	void reschedule(EventId id, int64_t new_target_cycle, int context = 0);

	// Called every cycle, advances the clock by one and fires the events that are due
	MASQ_INLINE void tick()
	{
		advance();

		if (is_event_due())
		{
			process_due_events();
		}
	}

private:
	struct EventDef_t
	{
		EventHandler handler;
		void* userData;
		uint8_t      priority;
	};

	MASQ_INLINE void advance() {
		currentCycles++;
	}
	MASQ_INLINE FLAG is_event_due() const {
		RETURN currentCycles >= nextEventTime ? YES : NO;
	}

	void    process_due_events();
	FLAG    is_before(const ScheduledEvent_t& a, const ScheduledEvent_t& b) const;
	int32_t find_event(EventId id, int context) const;
	void    remove_at(uint32_t index);
	void    recompute_next_event_time();

	EventDef_t eventDefs[MAX_EVENT_TYPES];
	uint32_t   eventDefCount;

	int64_t currentCycles;
	int64_t nextEventTime;
	int64_t nextSequence;
	uint32_t eventCount;
	ScheduledEvent_t events[MAX_EVENTS];
};
