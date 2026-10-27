#include "scheduler.h"

#if (MASQ_SCHEDULER_STATS != 0)
#define SCHED_STAT(statement) statement
#else
#define SCHED_STAT(statement)
#endif

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

void Scheduler_t::reset_stats()
{
	memset(&stats, 0, sizeof(stats));
}

void Scheduler_t::schedule(EventId id, int64_t target_cycle, int context)
{
	SCHED_STAT(stats.scheduleCalls++);

	ASSERT(id < eventDefCount);
	if (id >= eventDefCount)
	{
		SCHED_STAT(stats.scheduleInvalid++);
		RETURN;
	}

	// Already pending: ignored, use reschedule() to move it
	if (find_event(id, context) != INVALID)
	{
		SCHED_STAT(stats.scheduleDuplicates++);
		SCHED_STAT(stats.type[id].duplicates++);
		RETURN;
	}

	// Queue is full: the event is dropped. This is counted instead of asserted so that the stats window can show it
	if (eventCount >= MAX_EVENTS)
	{
		SCHED_STAT(stats.scheduleDropped++);
		SCHED_STAT(stats.type[id].dropped++);
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
	SCHED_STAT(stats.insertShifts += (uint64_t)(eventCount - index));
	eventCount++;
	SCHED_STAT(stats.type[id].scheduled++);
	SCHED_STAT(stats.peakDepth = (eventCount > stats.peakDepth) ? eventCount : stats.peakDepth);
	recompute_next_event_time();
}

void Scheduler_t::cancel(EventId id, int context)
{
	SCHED_STAT(stats.cancelCalls++);

	const int32_t index = find_event(id, context);
	if (index != INVALID)
	{
		SCHED_STAT(if (id < MAX_EVENT_TYPES) stats.type[id].cancelled++);
		remove_at((uint32_t)index);
	}
	else
	{
		SCHED_STAT(stats.cancelMisses++);
	}
}

void Scheduler_t::reschedule(EventId id, int64_t new_target_cycle, int context)
{
	SCHED_STAT(stats.rescheduleCalls++);

	// Creates the event if it isn't pending, and counts as a new insertion for same-cycle order
	cancel(id, context);
	schedule(id, new_target_cycle, context);
}

void Scheduler_t::post_load_fixup(const Scheduler_t& referenceInstance)
{
	// 1. Restore the static function pointers from the valid template instance
	eventDefCount = referenceInstance.eventDefCount;
	std::memcpy(eventDefs, referenceInstance.eventDefs, sizeof(eventDefs));

	// 2. Recompute the next event time using the newly loaded queue data from disk
	recompute_next_event_time();
}

void Scheduler_t::process_due_events()
{
	SCHED_STAT(stats.processDueCalls++);
	SCHED_STAT(uint32_t firedThisCall = 0);

	while (eventCount > 0 && events[0].timestamp <= currentCycles)
	{
		const ScheduledEvent_t dueEvent = events[0];

		SCHED_STAT(stats.depthSumAtFire += eventCount);

		remove_at(0);

		const EventDef_t& def = eventDefs[dueEvent.id];

#if (MASQ_SCHEDULER_STATS != 0)
		TypeStats_t& typeStats = stats.type[dueEvent.id];
		const int64_t lateness = currentCycles - dueEvent.timestamp;

		stats.eventsFired++;
		typeStats.fired++;
		firedThisCall++;

		if (lateness > 0)
		{
			stats.lateFires++;
			typeStats.lateFires++;
			if (lateness > typeStats.maxLateness) typeStats.maxLateness = lateness;
			if (lateness > stats.maxLateness)     stats.maxLateness = lateness;
		}

		if (profileHandlers == YES)
		{
			const auto start = std::chrono::steady_clock::now();
			def.handler(def.userData, dueEvent.context);
			const uint64_t ns = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();

			typeStats.profiledFires++;
			stats.profiledFiresTotal++;
			typeStats.handlerNs += ns;
			if (ns > typeStats.maxHandlerNs) typeStats.maxHandlerNs = ns;
			stats.handlerNsTotal += ns;
		}
		else
		{
			def.handler(def.userData, dueEvent.context);
		}
#else
		def.handler(def.userData, dueEvent.context);
#endif
	}

	SCHED_STAT(stats.maxDueInOneCall = (firedThisCall > stats.maxDueInOneCall) ? firedThisCall : stats.maxDueInOneCall);
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
			SCHED_STAT(stats.findScanSteps += (uint64_t)(i + 1));
			RETURN (int32_t)i;
		}
	}

	SCHED_STAT(stats.findScanSteps += (uint64_t)eventCount);
	RETURN INVALID;
}

void Scheduler_t::remove_at(uint32_t index)
{
	ASSERT(index < eventCount);
	if (index >= eventCount)
	{
		RETURN;
	}

	SCHED_STAT(stats.removeShifts += (uint64_t)(eventCount - index - 1));

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
