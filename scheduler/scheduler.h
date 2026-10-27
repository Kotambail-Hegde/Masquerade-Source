#pragma once

#include "helpers.h"
#include <chrono>

// Set to 0 to compile every statistic (and the optional handler profiler) out of the scheduler.
// The hot path (tick / advance / is_event_due) never touches the statistics either way.
#ifndef MASQ_SCHEDULER_STATS
#define MASQ_SCHEDULER_STATS 1
#endif

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

	// Per event type statistics
	struct TypeStats_t
	{
		uint64_t scheduled;		// schedule() calls that were accepted
		uint64_t fired;			// handler invocations
		uint64_t duplicates;	// schedule() calls ignored because the same (id, context) was already pending
		uint64_t dropped;		// schedule() calls dropped because the queue was full
		uint64_t cancelled;		// cancel() calls that actually removed a pending event
		uint64_t lateFires;		// events that fired after their timestamp (currentCycles > timestamp)
		int64_t  maxLateness;	// worst lateness in cycles
		uint64_t profiledFires;	// handler calls that were timed (profiling can be switched on mid-run, so this can be < fired)
		uint64_t handlerNs;		// time spent inside the handler (only while profiling is on)
		uint64_t maxHandlerNs;	// worst single handler call (only while profiling is on)
	};

	// Global statistics. Counters are cumulative since the last reset_stats().
	struct Stats_t
	{
		uint64_t scheduleCalls;
		uint64_t scheduleInvalid;
		uint64_t scheduleDuplicates;
		uint64_t scheduleDropped;
		uint64_t cancelCalls;
		uint64_t cancelMisses;
		uint64_t rescheduleCalls;

		uint64_t eventsFired;
		uint64_t processDueCalls;
		uint32_t maxDueInOneCall;	// most events fired from a single process_due_events() call
		uint32_t peakDepth;			// highest number of simultaneously pending events
		uint64_t depthSumAtFire;	// sum of the queue depth observed right before each fire (for the average)

		uint64_t insertShifts;		// elements moved while inserting (cost of the sorted insert)
		uint64_t removeShifts;		// elements moved while removing (cost of remove_at)
		uint64_t findScanSteps;		// elements visited by find_event

		uint64_t lateFires;
		int64_t  maxLateness;

		uint64_t profiledFiresTotal;	// handler calls that were timed
		uint64_t handlerNsTotal;	// only while profiling is on

		TypeStats_t type[MAX_EVENT_TYPES];
	};

	Scheduler_t() : eventDefCount(0) {
		profileHandlers = NO;
		reset();
		reset_stats();
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

	void post_load_fixup(const Scheduler_t& referenceInstance);

	// Called every cycle, advances the clock by one and fires the events that are due
	MASQ_INLINE void tick()
	{
		advance();

		if (is_event_due())
		{
			process_due_events();
		}
	}

	// ---- Statistics / diagnostics (read only from the GUI, the emulation never depends on them) ----
	const Stats_t& get_stats() const {
		RETURN stats;
	}
	void reset_stats();

	// Times every handler call with a steady_clock. Costs two clock reads per event, so it is off by default
	void set_handler_profiling(FLAG enable) {
		profileHandlers = enable;
	}
	FLAG is_handler_profiling() const {
		RETURN profileHandlers;
	}

	uint32_t get_event_type_count() const {
		RETURN eventDefCount;
	}
	uint8_t get_event_priority(EventId id) const {
		RETURN (id < eventDefCount) ? eventDefs[id].priority : (uint8_t)0;
	}
	uint32_t get_pending_count() const {
		RETURN eventCount;
	}
	const ScheduledEvent_t& get_pending(uint32_t index) const {
		RETURN events[index];
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

	// mutable: find_event() is const but still records how many elements it visited
	mutable Stats_t stats;
	FLAG profileHandlers;
};
