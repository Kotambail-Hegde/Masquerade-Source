#include "scheduler.h"

EventId Scheduler_t::register_event(MASQ_MAYBE_UNUSED EventHandler handler, MASQ_MAYBE_UNUSED void* userData, MASQ_MAYBE_UNUSED uint8_t priority)
{
	// TODO
	RETURN INVALID_EVENT_ID;
}

void Scheduler_t::reset()
{
	currentCycles = 0;
	nextEventTime = UINT64_MAX;
	nextSequence = 0;
	eventCount = 0;
}

void Scheduler_t::schedule(MASQ_MAYBE_UNUSED EventId id, MASQ_MAYBE_UNUSED uint64_t target_cycle, MASQ_MAYBE_UNUSED int context)
{
	// TODO
}

void Scheduler_t::cancel(MASQ_MAYBE_UNUSED EventId id, MASQ_MAYBE_UNUSED int context)
{
	// TODO
}

void Scheduler_t::reschedule(MASQ_MAYBE_UNUSED EventId id, MASQ_MAYBE_UNUSED uint64_t new_target_cycle, MASQ_MAYBE_UNUSED int context)
{
	// TODO
}

void Scheduler_t::process_due_events()
{
	// TODO
}

bool Scheduler_t::is_before(MASQ_MAYBE_UNUSED const ScheduledEvent_t& a, MASQ_MAYBE_UNUSED const ScheduledEvent_t& b) const
{
	// TODO
	RETURN NO;
}

int32_t Scheduler_t::find_event(MASQ_MAYBE_UNUSED EventId id, MASQ_MAYBE_UNUSED int context) const
{
	// TODO
	RETURN INVALID;
}

void Scheduler_t::remove_at(MASQ_MAYBE_UNUSED uint32_t index)
{
	// TODO
}

void Scheduler_t::recompute_next_event_time()
{
	// TODO
}