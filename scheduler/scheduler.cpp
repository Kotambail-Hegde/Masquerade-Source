#include "scheduler.h"

EventId Scheduler_t::register_event(EventHandler handler, void* userData, uint8_t priority)
{
	if (handler == nullptr || eventDefCount >= MAX_EVENT_TYPES)
	{
		RETURN INVALID_EVENT_ID;
	}

	const EventId id = (EventId)eventDefCount;
	eventDefs[id] = { handler, userData, priority };
	eventDefCount++;

	RETURN id;
}

void Scheduler_t::reset()
{
	currentCycles = 0;
	nextEventTime = INT64_MAX;
	nextSequence = 0;
	eventCount = 0;

	memset(events, 0, sizeof(events));
}

void Scheduler_t::schedule(EventId id, int64_t target_cycle, int context)
{
	ASSERT(id < eventDefCount);
	if (id >= eventDefCount)
	{
		RETURN;
	}

	// Already pending: ignored, use reschedule() to move it
	if (find_event(id, context) != INVALID)
	{
		RETURN;
	}

	if (eventCount >= MAX_EVENTS)
	{
		RETURN;
	}

	const ScheduledEvent_t newEvent = { target_cycle, nextSequence++, context, id };

	uint32_t index = eventCount;
	while (index > 0 && is_before(newEvent, events[index - 1]))
	{
		events[index] = events[index - 1];
		index--;
	}

	events[index] = newEvent;
	eventCount++;
	recompute_next_event_time();
}

void Scheduler_t::cancel(EventId id, int context)
{
	const int32_t index = find_event(id, context);
	if (index != INVALID)
	{
		remove_at((uint32_t)index);
	}
}

void Scheduler_t::reschedule(EventId id, int64_t new_target_cycle, int context)
{
	// Creates the event if it isn't pending, and counts as a new insertion for same-cycle order
	cancel(id, context);
	schedule(id, new_target_cycle, context);
}

void Scheduler_t::process_due_events()
{
	while (eventCount > 0 && events[0].timestamp <= currentCycles)
	{
		const ScheduledEvent_t dueEvent = events[0];
		remove_at(0);

		const EventDef_t& def = eventDefs[dueEvent.id];
		def.handler(def.userData, dueEvent.context);
	}
}

FLAG Scheduler_t::is_before(const ScheduledEvent_t& a, const ScheduledEvent_t& b) const
{
	if (a.timestamp != b.timestamp)
	{
		RETURN a.timestamp < b.timestamp ? YES : NO;
	}

	// Same cycle: lower priority value first, then insertion order
	const uint8_t prioA = eventDefs[a.id].priority;
	const uint8_t prioB = eventDefs[b.id].priority;
	if (prioA != prioB)
	{
		RETURN prioA < prioB ? YES : NO;
	}

	RETURN a.sequence < b.sequence ? YES : NO;
}

int32_t Scheduler_t::find_event(EventId id, int context) const
{
	for (uint32_t i = 0; i < eventCount; i++)
	{
		if (events[i].id == id && events[i].context == context)
		{
			RETURN (int32_t)i;
		}
	}

	RETURN INVALID;
}

void Scheduler_t::remove_at(uint32_t index)
{
	ASSERT(index < eventCount);
	if (index >= eventCount)
	{
		RETURN;
	}

	for (uint32_t i = index; i + 1 < eventCount; i++)
	{
		events[i] = events[i + 1];
	}

	eventCount--;
	recompute_next_event_time();
}

void Scheduler_t::recompute_next_event_time()
{
	nextEventTime = (eventCount > 0) ? events[0].timestamp : INT64_MAX;
}
