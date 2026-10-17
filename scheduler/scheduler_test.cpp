// Standalone tests for Scheduler_t (scheduler.h / scheduler.cpp)
//
// g++/clang : g++ -std=c++17 -O2 -Wall -Wextra -pthread scheduler.cpp scheduler_test.cpp -o scheduler_test
// MSVC      : cl /std:c++17 /EHsc /O2 scheduler.cpp scheduler_test.cpp /Fe:scheduler_test.exe
//
// scheduler_test [-v] [-q] [--list] [--filter text] [--seed N] [--fuzz-iters N] [--timeout seconds]
//                [--probe] [--probe-capacity] [--bench]
//
//   default           the tests that must pass
//   --probe           prints what your implementation does for behaviour that is not decided yet (INFO, never fails)
//   --probe-capacity  same, for running out of event slots (may abort if you ASSERT there)
//   --bench           speed of tick() (use an optimized build)
//   -v / -q           full logs / results only
//   --timeout N       a test running longer than N seconds is reported as a hang and the program exits (default 20, 0 = off)

#include "scheduler.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if defined(_MSC_VER)
#include <intrin.h>
#define BENCH_BARRIER() _ReadWriteBarrier()
#else
#define BENCH_BARRIER() asm volatile("" ::: "memory")
#endif

using u64 = uint64_t;
static constexpr u64 NO_EVENT = UINT64_MAX;

// ---------------------------------------------------------------------------------------------------------------------
// framework
// ---------------------------------------------------------------------------------------------------------------------

static std::string fmt(const char* f, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, f);
	vsnprintf(buf, sizeof(buf), f, ap);
	va_end(ap);
	return buf;
}

static unsigned long long ull(u64 v) { return (unsigned long long)v; }

struct Options
{
	bool verbose = false;
	bool quiet = false;
	bool list = false;
	bool probe = false;
	bool probeCapacity = false;
	bool bench = false;
	std::string filter;
	u64 seed = 1;
	int fuzzIters = 25;
	int timeoutSec = 20;
};
static Options opt;

struct TestCtx
{
	std::vector<std::string> log;
	std::vector<std::string> failures;
	std::vector<std::string> info;
};
static TestCtx* g_ctx = nullptr;

static void fail_at(const char* file, int line, const std::string& msg)
{
	const char* base = file;
	for (const char* p = file; *p; ++p)
	{
		if (*p == '/' || *p == '\\') base = p + 1;
	}
	g_ctx->failures.push_back(fmt("%s:%d  %s", base, line, msg.c_str()));
}

#define CHECK(c)                                                                                                       \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(c)) fail_at(__FILE__, __LINE__, "CHECK(" #c ")");                                                        \
	} while (0)

template <class A, class B>
static void check_eq(const A& a, const B& b, const char* ea, const char* eb, const char* file, int line)
{
	if (!(a == b))
	{
		std::ostringstream os;
		os << "CHECK_EQ(" << ea << ", " << eb << ")  got " << +a << " vs " << +b;
		fail_at(file, line, os.str());
	}
}
#define CHECK_EQ(a, b) check_eq((a), (b), #a, #b, __FILE__, __LINE__)

enum class Kind { Normal, Probe, ProbeCapacity, Bench };

struct TestCase
{
	const char* name;
	void (*fn)(TestCtx&);
	Kind kind;
};

static std::vector<TestCase>& registry()
{
	static std::vector<TestCase> r;
	return r;
}

struct Registrar
{
	Registrar(const char* n, void (*f)(TestCtx&), Kind k) { registry().push_back({n, f, k}); }
};

#define TEST(name)                                                                                                     \
	static void name(TestCtx& t);                                                                                      \
	static Registrar registrar_##name(#name, &name, Kind::Normal);                                                     \
	static void name([[maybe_unused]] TestCtx& t)
#define PROBE(name)                                                                                                    \
	static void name(TestCtx& t);                                                                                      \
	static Registrar registrar_##name(#name, &name, Kind::Probe);                                                      \
	static void name([[maybe_unused]] TestCtx& t)
#define PROBE_CAPACITY(name)                                                                                           \
	static void name(TestCtx& t);                                                                                      \
	static Registrar registrar_##name(#name, &name, Kind::ProbeCapacity);                                              \
	static void name([[maybe_unused]] TestCtx& t)
#define BENCH(name)                                                                                                    \
	static void name(TestCtx& t);                                                                                      \
	static Registrar registrar_##name(#name, &name, Kind::Bench);                                                      \
	static void name([[maybe_unused]] TestCtx& t)

// ---------------------------------------------------------------------------------------------------------------------
// harness: wraps Scheduler_t, logs every call and every fired event with its cycle
// ---------------------------------------------------------------------------------------------------------------------

class Harness
{
public:
	struct Fire
	{
		u64 t;
		int slot;
		int ctx;
		bool operator==(const Fire& o) const { return t == o.t && slot == o.slot && ctx == o.ctx; }
	};

	explicit Harness(TestCtx& c) : ctx(c) {}
	Harness(const Harness&) = delete;
	Harness& operator=(const Harness&) = delete;

	int reg(const char* name, uint8_t priority = 0, std::function<void(int)> action = nullptr)
	{
		const int slot = (int)slots.size();
		bindings.push_back(std::make_unique<Binding>(Binding{this, slot}));
		Slot s;
		s.name = name;
		s.priority = priority;
		s.action = std::move(action);
		s.id = sched.register_event(&Harness::trampoline, bindings.back().get(), priority);
		slots.push_back(std::move(s));
		note("register %-8s prio=%u id=%u", name, (unsigned)priority, (unsigned)slots.back().id);
		return slot;
	}

	EventId id(int slot) const { return slots[slot].id; }
	u64 now() const { return sched.get_current_cycles(); }

	void at(int slot, u64 target, int c = 0)
	{
		note("sched    %-8s ctx=%d  @%llu", slots[slot].name.c_str(), c, ull(target));
		sched.schedule(slots[slot].id, target, c);
	}

	void cancel(int slot, int c = 0)
	{
		note("cancel   %-8s ctx=%d", slots[slot].name.c_str(), c);
		sched.cancel(slots[slot].id, c);
	}

	void resched(int slot, u64 target, int c = 0)
	{
		note("resched  %-8s ctx=%d  @%llu", slots[slot].name.c_str(), c, ull(target));
		sched.reschedule(slots[slot].id, target, c);
	}

	void tick(u64 n = 1)
	{
		for (u64 i = 0; i < n; i++) sched.tick();
	}

	void run_to(u64 cycle)
	{
		while (sched.get_current_cycles() < cycle) sched.tick();
	}

	void reset()
	{
		note("reset");
		sched.reset();
	}

	void expect(std::initializer_list<Fire> expected) { expect_vec(std::vector<Fire>(expected)); }

	void expect_vec(const std::vector<Fire>& expected)
	{
		if (!(expected == fires))
		{
			fail_at(__FILE__, __LINE__, "fired events differ\n        expected: " + describe(expected) + "\n        actual:   " + describe(fires));
		}
	}

	std::string describe(const std::vector<Fire>& v) const
	{
		std::string s;
		for (const Fire& f : v)
		{
			s += fmt("[%llu:%s:%d] ", ull(f.t), slots[f.slot].name.c_str(), f.ctx);
		}
		return s.empty() ? "(none)" : s;
	}

	void note(const char* f, ...)
	{
		if (!logCalls) return;
		char buf[512];
		va_list ap;
		va_start(ap, f);
		vsnprintf(buf, sizeof(buf), f, ap);
		va_end(ap);
		ctx.log.push_back(fmt("[t=%6llu] %s", ull(sched.get_current_cycles()), buf));
	}

	Scheduler_t sched;
	std::vector<Fire> fires;
	bool logCalls = true;
	bool logEvents = true;

private:
	struct Binding
	{
		Harness* h;
		int slot;
	};
	struct Slot
	{
		std::string name;
		uint8_t priority = 0;
		EventId id = INVALID_EVENT_ID;
		std::function<void(int)> action;
	};

	static void trampoline(void* user, int c)
	{
		auto* b = static_cast<Binding*>(user);
		b->h->on_fire(b->slot, c);
	}

	void on_fire(int slot, int c)
	{
		if (fires.size() >= MAX_FIRES) throw std::runtime_error("runaway: more than 2000000 events fired in one test");
		const u64 t = sched.get_current_cycles();
		fires.push_back({t, slot, c});
		if (logEvents) ctx.log.push_back(fmt("[t=%6llu] FIRE     %-8s ctx=%d", ull(t), slots[slot].name.c_str(), c));
		if (slots[slot].action) slots[slot].action(c);
	}

	static constexpr size_t MAX_FIRES = 2000000;

	TestCtx& ctx;
	std::vector<Slot> slots;
	std::vector<std::unique_ptr<Binding>> bindings;
};

using F = Harness::Fire;

// ---------------------------------------------------------------------------------------------------------------------
// basics
// ---------------------------------------------------------------------------------------------------------------------

TEST(reset_state)
{
	Harness h(t);
	CHECK_EQ(h.now(), 0u);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);

	const int a = h.reg("A");
	CHECK(h.id(a) != INVALID_EVENT_ID);
	h.at(a, 5);
	h.at(a, 9, 1);
	h.tick(3);
	CHECK_EQ(h.now(), 3u);
	CHECK_EQ(h.sched.peek_next_timestamp(), 5u);

	h.reset();
	CHECK_EQ(h.now(), 0u);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
	h.fires.clear();
	h.tick(20);
	CHECK(h.fires.empty());

	h.at(a, h.now() + 4, 7);
	h.tick(4);
	h.expect({{24, a, 7}});
}

TEST(register_ids_unique)
{
	Harness h(t);
	std::set<EventId> seen;
	for (uint32_t i = 0; i < Scheduler_t::MAX_EVENT_TYPES; i++)
	{
		const int s = h.reg(fmt("T%u", i).c_str());
		CHECK(h.id(s) != INVALID_EVENT_ID);
		CHECK(seen.insert(h.id(s)).second);
	}
	CHECK_EQ(seen.size(), (size_t)Scheduler_t::MAX_EVENT_TYPES);
}

TEST(tick_advances_clock)
{
	Harness h(t);
	for (u64 i = 1; i <= 1000; i++)
	{
		h.sched.tick();
		if (h.now() != i)
		{
			CHECK_EQ(h.now(), i);
			break;
		}
	}
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
	CHECK(h.fires.empty());
}

TEST(fires_at_exact_cycle)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 5, 7);
	h.tick(4);
	CHECK(h.fires.empty());
	CHECK_EQ(h.now(), 4u);
	h.tick(1);
	h.expect({{5, a, 7}});
	h.tick(20);
	h.expect({{5, a, 7}});
}

TEST(peek_tracks_next_event)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 30, 0);
	CHECK_EQ(h.sched.peek_next_timestamp(), 30u);
	h.at(a, 10, 1);
	CHECK_EQ(h.sched.peek_next_timestamp(), 10u);
	h.at(a, 20, 2);
	CHECK_EQ(h.sched.peek_next_timestamp(), 10u);
	h.run_to(10);
	CHECK_EQ(h.sched.peek_next_timestamp(), 20u);
	h.run_to(20);
	CHECK_EQ(h.sched.peek_next_timestamp(), 30u);
	h.run_to(30);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

// ---------------------------------------------------------------------------------------------------------------------
// ordering
// ---------------------------------------------------------------------------------------------------------------------

TEST(order_by_timestamp)
{
	Harness h(t);
	const int a = h.reg("A");
	const int hi = h.reg("HI", 0);
	const int lo = h.reg("LO", 3);
	const u64 times[] = {15, 5, 10, 1, 20};
	for (int i = 0; i < 5; i++) h.at(a, times[i], i);
	h.at(hi, 12);
	h.at(lo, 11);
	h.run_to(20);
	h.expect({{1, a, 3}, {5, a, 1}, {10, a, 2}, {11, lo, 0}, {12, hi, 0}, {15, a, 0}, {20, a, 4}});
}

TEST(order_same_cycle_by_priority)
{
	Harness h(t);
	const int p2 = h.reg("P2", 2);
	const int p0 = h.reg("P0", 0);
	const int pmax = h.reg("PMAX", 255);
	const int p1 = h.reg("P1", 1);
	h.at(p2, 10);
	h.at(pmax, 10);
	h.at(p0, 10);
	h.at(p1, 10);
	h.run_to(10);
	h.expect({{10, p0, 0}, {10, p1, 0}, {10, p2, 0}, {10, pmax, 0}});
}

TEST(order_same_cycle_same_priority_is_insertion_order)
{
	Harness h(t);
	const int a = h.reg("A", 1);
	const int b = h.reg("B", 1);
	const int c = h.reg("C", 0);
	h.at(a, 10, 3);
	h.at(b, 10, 0);
	h.at(a, 10, 1);
	h.at(b, 10, 9);
	h.at(c, 10, 5);
	h.run_to(10);
	h.expect({{10, c, 5}, {10, a, 3}, {10, b, 0}, {10, a, 1}, {10, b, 9}});
}

TEST(order_insertion_order_independent_of_target_order)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10, 0);
	h.at(a, 8, 1);
	h.at(a, 10, 2);
	h.at(a, 8, 3);
	h.run_to(10);
	h.expect({{8, a, 1}, {8, a, 3}, {10, a, 0}, {10, a, 2}});
}

TEST(many_events_same_cycle_all_fire_once)
{
	Harness h(t);
	const int a = h.reg("A");
	std::vector<F> exp;
	for (int c = 0; c < 20; c++)
	{
		h.at(a, 7, c);
		exp.push_back({7, a, c});
	}
	h.run_to(6);
	CHECK(h.fires.empty());
	h.tick(1);
	h.expect_vec(exp);
	h.tick(10);
	h.expect_vec(exp);
}

TEST(all_event_types_same_cycle)
{
	Harness h(t);
	struct Item { int slot; uint8_t prio; };
	std::vector<Item> items;
	for (uint32_t i = 0; i < Scheduler_t::MAX_EVENT_TYPES; i++)
	{
		const uint8_t prio = (uint8_t)((i * 7) % 4);
		items.push_back({h.reg(fmt("T%u", i).c_str(), prio), prio});
	}
	for (const Item& it : items) h.at(it.slot, 10);
	h.run_to(10);

	std::vector<Item> sorted = items;
	std::stable_sort(sorted.begin(), sorted.end(), [](const Item& x, const Item& y) { return x.prio < y.prio; });
	std::vector<F> exp;
	for (const Item& it : sorted) exp.push_back({10, it.slot, 0});
	h.expect_vec(exp);
}

// ---------------------------------------------------------------------------------------------------------------------
// payload
// ---------------------------------------------------------------------------------------------------------------------

TEST(context_and_userdata_passed_through)
{
	Harness h(t);
	int countA = 0, countB = 0;
	const int a = h.reg("A", 0, [&](int) { countA++; });
	const int b = h.reg("B", 0, [&](int) { countB++; });
	const int ctxs[] = {std::numeric_limits<int>::min(), -1, 0, 1, std::numeric_limits<int>::max()};
	std::vector<F> exp;
	u64 when = 2;
	for (int c : ctxs)
	{
		h.at(a, when, c);
		exp.push_back({when, a, c});
		h.at(b, when + 1, c);
		exp.push_back({when + 1, b, c});
		when += 2;
	}
	h.run_to(when);
	h.expect_vec(exp);
	CHECK_EQ(countA, 5);
	CHECK_EQ(countB, 5);
}

// ---------------------------------------------------------------------------------------------------------------------
// cancel
// ---------------------------------------------------------------------------------------------------------------------

TEST(cancel_prevents_fire)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	const int c = h.reg("C");
	h.at(a, 10);
	h.at(b, 10);
	h.at(c, 12);
	h.cancel(b);
	CHECK_EQ(h.sched.peek_next_timestamp(), 10u);
	h.run_to(12);
	h.expect({{10, a, 0}, {12, c, 0}});
}

TEST(cancel_next_event_updates_peek)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 5, 0);
	h.at(a, 8, 1);
	h.cancel(a, 0);
	CHECK_EQ(h.sched.peek_next_timestamp(), 8u);
	h.cancel(a, 1);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
	h.tick(20);
	CHECK(h.fires.empty());
}

TEST(cancel_matches_id_and_context)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	h.at(a, 10, 0);
	h.at(a, 10, 1);
	h.at(a, 10, 2);
	h.at(b, 10, 1);
	h.cancel(a, 1);
	h.run_to(10);
	h.expect({{10, a, 0}, {10, a, 2}, {10, b, 1}});
}

// ---------------------------------------------------------------------------------------------------------------------
// reschedule
// ---------------------------------------------------------------------------------------------------------------------

TEST(reschedule_later)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	h.at(a, 10);
	h.at(b, 15);
	h.resched(a, 20);
	CHECK_EQ(h.sched.peek_next_timestamp(), 15u);
	h.run_to(25);
	h.expect({{15, b, 0}, {20, a, 0}});
}

TEST(reschedule_earlier)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	h.at(a, 20);
	h.at(b, 15);
	h.resched(a, 5);
	CHECK_EQ(h.sched.peek_next_timestamp(), 5u);
	h.run_to(25);
	h.expect({{5, a, 0}, {15, b, 0}});
}

TEST(reschedule_to_same_time)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10);
	h.resched(a, 10);
	h.run_to(15);
	h.expect({{10, a, 0}});
}

TEST(reschedule_keeps_context_and_isolates_others)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10, 3);
	h.at(a, 12, 4);
	h.resched(a, 30, 3);
	h.run_to(30);
	h.expect({{12, a, 4}, {30, a, 3}});
}

TEST(reschedule_repeatedly)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10);
	u64 last = 0;
	for (u64 target = 11; target <= 40; target += 3)
	{
		h.resched(a, target);
		last = target;
	}
	h.run_to(50);
	h.expect({{last, a, 0}});
}

// ---------------------------------------------------------------------------------------------------------------------
// handlers that call back into the scheduler
// ---------------------------------------------------------------------------------------------------------------------

TEST(handler_reschedules_itself_periodic)
{
	Harness h(t);
	h.logEvents = true;
	int a = -1;
	a = h.reg("PERIODIC", 0, [&](int c) { h.at(a, h.now() + 10, c); });
	h.at(a, 10, 0);
	h.run_to(100);
	std::vector<F> exp;
	for (u64 k = 1; k <= 10; k++) exp.push_back({k * 10, a, 0});
	h.expect_vec(exp);
	CHECK_EQ(h.sched.peek_next_timestamp(), 110u);
}

TEST(handler_clock_equals_event_time)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	h.at(a, 3);
	h.at(b, 3);
	h.at(a, 9, 1);
	h.run_to(12);
	for (const F& f : h.fires)
	{
		const bool ok = (f.t == 3 && f.ctx == 0) || (f.t == 9 && f.ctx == 1);
		if (!ok) CHECK(false);
	}
	CHECK_EQ(h.fires.size(), (size_t)3);
}

TEST(handler_schedules_future_event_of_other_type)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.at(b, h.now() + 3, 5); });
	b = h.reg("B");
	h.at(a, 5);
	h.run_to(10);
	h.expect({{5, a, 0}, {8, b, 5}});
}

TEST(handler_cancels_event_pending_in_same_cycle)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.cancel(b); });
	b = h.reg("B", 1);
	h.at(b, 10);
	h.at(a, 10);
	h.run_to(15);
	h.expect({{10, a, 0}});
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(handler_cancels_future_event)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.cancel(b); });
	b = h.reg("B");
	h.at(a, 5);
	h.at(b, 20);
	h.run_to(25);
	h.expect({{5, a, 0}});
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(handler_reschedules_other_event)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.resched(b, 12); });
	b = h.reg("B");
	h.at(a, 5);
	h.at(b, 6);
	h.run_to(15);
	h.expect({{5, a, 0}, {12, b, 0}});
}

TEST(cancel_periodic_from_another_event)
{
	Harness h(t);
	int p = -1;
	p = h.reg("P", 1, [&](int c) { h.at(p, h.now() + 4, c); });
	const int killer = h.reg("KILL", 0, [&](int) { h.cancel(p); });
	h.at(p, 4);
	h.at(killer, 10);
	h.run_to(30);
	h.expect({{4, p, 0}, {8, p, 0}, {10, killer, 0}});
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(timer_register_write_pattern)
{
	Harness h(t);
	const int reload = h.reg("RELOAD_WR", 1);
	const int control = h.reg("CTRL_WR", 2);

	h.run_to(10);
	h.at(control, h.now() + 1, 0);
	h.run_to(11);
	h.at(control, h.now() + 1, 0);
	h.run_to(12);

	h.run_to(20);
	h.at(control, h.now() + 1, 1);
	h.at(reload, h.now() + 1, 1);
	h.run_to(22);

	h.expect({{11, control, 0}, {12, control, 0}, {21, reload, 1}, {21, control, 1}});
}

// ---------------------------------------------------------------------------------------------------------------------
// limits and stress
// ---------------------------------------------------------------------------------------------------------------------

TEST(capacity_fill_all_slots)
{
	Harness h(t);
	h.logCalls = false;
	h.logEvents = false;
	const int a = h.reg("A");
	std::vector<F> exp;
	for (int c = (int)Scheduler_t::MAX_EVENTS - 1; c >= 0; c--) h.at(a, 100 + (u64)c * 3, c);
	t.log.push_back(fmt("scheduled %u events (MAX_EVENTS) in reverse time order", (unsigned)Scheduler_t::MAX_EVENTS));
	for (int c = 0; c < (int)Scheduler_t::MAX_EVENTS; c++) exp.push_back({100 + (u64)c * 3, a, c});
	CHECK_EQ(h.sched.peek_next_timestamp(), 100u);
	h.run_to(100 + (u64)Scheduler_t::MAX_EVENTS * 3);
	h.expect_vec(exp);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(capacity_reuse_after_fire)
{
	Harness h(t);
	h.logCalls = false;
	h.logEvents = false;
	const int a = h.reg("A");
	for (int round = 0; round < 5; round++)
	{
		const u64 base = h.now();
		for (int c = 0; c < (int)Scheduler_t::MAX_EVENTS; c++) h.at(a, base + 1 + (u64)c, c);
		h.run_to(base + Scheduler_t::MAX_EVENTS);
	}
	CHECK_EQ(h.fires.size(), (size_t)5 * Scheduler_t::MAX_EVENTS);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(far_future_event)
{
	Harness h(t);
	const int a = h.reg("A");
	const u64 far = (u64)1 << 40;
	h.at(a, far);
	CHECK_EQ(h.sched.peek_next_timestamp(), far);
	h.tick(1000);
	CHECK(h.fires.empty());
	CHECK_EQ(h.sched.peek_next_timestamp(), far);
	h.cancel(a);
	CHECK_EQ(h.sched.peek_next_timestamp(), NO_EVENT);
}

TEST(recurring_events_stress)
{
	Harness h(t);
	h.logCalls = false;
	h.logEvents = false;
	const u64 periods[] = {3, 5, 7, 16, 31, 64, 100, 1000};
	const int n = 8;
	Scheduler_t s;
	struct Rec { u64 t; int which; };
	struct Env
	{
		Scheduler_t* s;
		EventId ids[8];
		u64 periods[8];
		std::vector<Rec> recs;
	} env;
	struct B { Env* e; int which; } binds[8];
	env.s = &s;
	for (int i = 0; i < n; i++)
	{
		env.periods[i] = periods[i];
		binds[i] = {&env, i};
		env.ids[i] = s.register_event(
			[](void* u, int) {
				auto* b = static_cast<B*>(u);
				const u64 now = b->e->s->get_current_cycles();
				b->e->recs.push_back({now, b->which});
				b->e->s->schedule(b->e->ids[b->which], now + b->e->periods[b->which], 0);
			},
			&binds[i], (uint8_t)(i % 3));
		s.schedule(env.ids[i], periods[i], 0);
	}
	const u64 total = 50000;
	for (u64 i = 0; i < total; i++) s.tick();

	std::vector<std::vector<u64>> per(n);
	u64 lastT = 0;
	bool ordered = true;
	for (const Rec& r : env.recs)
	{
		per[r.which].push_back(r.t);
		if (r.t < lastT) ordered = false;
		lastT = r.t;
	}
	CHECK(ordered);
	for (int i = 0; i < n; i++)
	{
		CHECK_EQ(per[i].size(), (size_t)(total / periods[i]));
		for (size_t k = 0; k < per[i].size(); k++)
		{
			if (per[i][k] != (k + 1) * periods[i])
			{
				CHECK_EQ(per[i][k], (k + 1) * periods[i]);
				break;
			}
		}
		t.log.push_back(fmt("period %-5llu fired %zu times (expected %llu)", ull(periods[i]), per[i].size(), ull(total / periods[i])));
	}
	t.log.push_back(fmt("ran %llu cycles, %zu events fired in total", ull(total), env.recs.size()));
}

// ---------------------------------------------------------------------------------------------------------------------
// GBA-like scenario: scanlines, 4 timers, APU frame sequencer and sample events, run for whole frames
// ---------------------------------------------------------------------------------------------------------------------

struct GbaLike
{
	enum Kind { HBLANK, LINE_END, TIMER, FRAME_SEQ, SAMPLE, KIND_COUNT };
	struct Rec { u64 t; int kind; int ctx; };
	struct Bind { GbaLike* g; int kind; };

	static constexpr u64 CLOCK_HZ = 16777216;
	static constexpr u64 LINE = 1232;
	static constexpr u64 H_VISIBLE = 1007;
	static constexpr u64 LINES = 228;
	static constexpr u64 VBLANK_LINE = 160;
	static constexpr u64 FRAME = LINE * LINES;
	static constexpr u64 SAMPLE_RATE = 48000;
	static constexpr u64 SEQ_PERIOD = CLOCK_HZ / 512;
	static constexpr u64 TIMER_PERIOD[4] = {1024, 4096, 65536, 262144};

	Scheduler_t s;
	EventId id[KIND_COUNT];
	Bind bind[KIND_COUNT];
	const uint8_t prio[KIND_COUNT] = {3, 3, 0, 2, 2};

	u64 line = 0, sampleIndex = 1, frames = 0;
	u64 hblanks = 0, lineEnds = 0, vblanks = 0, frameSeqs = 0, samples = 0;
	u64 timerFires[4] = {0, 0, 0, 0};
	bool record = false;
	std::vector<Rec> recs;

	GbaLike()
	{
		for (int k = 0; k < KIND_COUNT; k++)
		{
			bind[k] = {this, k};
			id[k] = s.register_event(&GbaLike::thunk, &bind[k], prio[k]);
		}
		restart(false);
	}

	static u64 sample_time(u64 k) { return k * CLOCK_HZ / SAMPLE_RATE; }
	static void thunk(void* u, int c)
	{
		auto* b = static_cast<Bind*>(u);
		b->g->fire(b->kind, c);
	}

	void restart(bool rec)
	{
		s.reset();
		line = 0;
		sampleIndex = 1;
		frames = hblanks = lineEnds = vblanks = frameSeqs = samples = 0;
		for (u64& v : timerFires) v = 0;
		recs.clear();
		record = rec;
		s.schedule(id[HBLANK], H_VISIBLE);
		for (int i = 0; i < 4; i++) s.schedule(id[TIMER], TIMER_PERIOD[i], i);
		s.schedule(id[FRAME_SEQ], SEQ_PERIOD);
		s.schedule(id[SAMPLE], sample_time(1));
	}

	void fire(int kind, int ctx)
	{
		const u64 now = s.get_current_cycles();
		if (record) recs.push_back({now, kind, ctx});
		switch (kind)
		{
		case HBLANK:
			hblanks++;
			s.schedule(id[LINE_END], now + (LINE - H_VISIBLE));
			break;
		case LINE_END:
			lineEnds++;
			if (++line == LINES)
			{
				line = 0;
				frames++;
			}
			if (line == VBLANK_LINE) vblanks++;
			s.schedule(id[HBLANK], now + H_VISIBLE);
			break;
		case TIMER:
			timerFires[ctx]++;
			s.schedule(id[TIMER], now + TIMER_PERIOD[ctx], ctx);
			break;
		case FRAME_SEQ:
			frameSeqs++;
			s.schedule(id[FRAME_SEQ], now + SEQ_PERIOD);
			break;
		case SAMPLE:
			samples++;
			sampleIndex++;
			s.schedule(id[SAMPLE], sample_time(sampleIndex));
			break;
		}
	}

	void run(u64 ticks)
	{
		for (u64 i = 0; i < ticks; i++) s.tick();
	}

	static const char* kind_name(int k)
	{
		static const char* n[] = {"hblank", "line_end", "timer", "frame_seq", "sample"};
		return n[k];
	}
};

TEST(gba_like_one_frame)
{
	GbaLike g;
	g.restart(true);
	g.run(GbaLike::FRAME);

	for (size_t i = 0; i < g.recs.size() && i < 60; i++)
	{
		const GbaLike::Rec& r = g.recs[i];
		t.log.push_back(fmt("[t=%6llu] FIRE     %-9s ctx=%d", ull(r.t), GbaLike::kind_name(r.kind), r.ctx));
	}
	t.log.push_back(fmt("... %zu events fired in one frame (%llu cycles)", g.recs.size(), ull(GbaLike::FRAME)));

	CHECK_EQ(g.s.get_current_cycles(), GbaLike::FRAME);

	std::vector<u64> hb, le, fs, sm, tm[4];
	u64 lastT = 0;
	int lastPrio = -1;
	bool ordered = true;
	for (const GbaLike::Rec& r : g.recs)
	{
		const int p = g.prio[r.kind];
		if (r.t < lastT || (r.t == lastT && p < lastPrio)) ordered = false;
		lastT = r.t;
		lastPrio = p;
		switch (r.kind)
		{
		case GbaLike::HBLANK: hb.push_back(r.t); break;
		case GbaLike::LINE_END: le.push_back(r.t); break;
		case GbaLike::FRAME_SEQ: fs.push_back(r.t); break;
		case GbaLike::SAMPLE: sm.push_back(r.t); break;
		case GbaLike::TIMER: tm[r.ctx].push_back(r.t); break;
		}
	}
	CHECK(ordered);

	CHECK_EQ(hb.size(), (size_t)((GbaLike::FRAME - GbaLike::H_VISIBLE) / GbaLike::LINE + 1));
	CHECK_EQ(le.size(), (size_t)(GbaLike::FRAME / GbaLike::LINE));
	CHECK_EQ(g.hblanks, 228u);
	CHECK_EQ(g.lineEnds, 228u);
	CHECK_EQ(g.vblanks, 1u);
	CHECK_EQ(g.frames, 1u);
	CHECK_EQ(fs.size(), (size_t)(GbaLike::FRAME / GbaLike::SEQ_PERIOD));
	for (size_t i = 0; i < hb.size(); i++) CHECK_EQ(hb[i], i * GbaLike::LINE + GbaLike::H_VISIBLE);
	for (size_t i = 0; i < le.size(); i++) CHECK_EQ(le[i], (i + 1) * GbaLike::LINE);
	for (size_t i = 0; i < fs.size(); i++) CHECK_EQ(fs[i], (i + 1) * GbaLike::SEQ_PERIOD);
	for (int k = 0; k < 4; k++)
	{
		CHECK_EQ(tm[k].size(), (size_t)(GbaLike::FRAME / GbaLike::TIMER_PERIOD[k]));
		for (size_t i = 0; i < tm[k].size(); i++) CHECK_EQ(tm[k][i], (i + 1) * GbaLike::TIMER_PERIOD[k]);
	}
	size_t expectedSamples = 0;
	while (GbaLike::sample_time(expectedSamples + 1) <= GbaLike::FRAME) expectedSamples++;
	CHECK_EQ(sm.size(), expectedSamples);
	for (size_t i = 0; i < sm.size(); i++) CHECK_EQ(sm[i], GbaLike::sample_time(i + 1));
}

TEST(gba_like_many_frames_stay_exact)
{
	GbaLike g;
	const u64 frames = 20;
	g.run(GbaLike::FRAME * frames);
	CHECK_EQ(g.hblanks, 228 * frames);
	CHECK_EQ(g.lineEnds, 228 * frames);
	CHECK_EQ(g.vblanks, frames);
	CHECK_EQ(g.frames, frames);
	for (int k = 0; k < 4; k++) CHECK_EQ(g.timerFires[k], GbaLike::FRAME * frames / GbaLike::TIMER_PERIOD[k]);
	CHECK_EQ(g.frameSeqs, GbaLike::FRAME * frames / GbaLike::SEQ_PERIOD);
	t.log.push_back(fmt("ran %llu frames (%llu cycles), hblank=%llu vblank=%llu samples=%llu", ull(frames), ull(GbaLike::FRAME * frames), ull(g.hblanks), ull(g.vblanks), ull(g.samples)));
}

TEST(reset_replays_identically)
{
	GbaLike g;
	g.restart(true);
	g.run(GbaLike::FRAME);
	const std::vector<GbaLike::Rec> first = g.recs;
	g.restart(true);
	g.run(GbaLike::FRAME);
	CHECK_EQ(first.size(), g.recs.size());
	bool same = first.size() == g.recs.size();
	for (size_t i = 0; same && i < first.size(); i++)
	{
		same = first[i].t == g.recs[i].t && first[i].kind == g.recs[i].kind && first[i].ctx == g.recs[i].ctx;
	}
	CHECK(same);
	t.log.push_back(fmt("two runs of one frame, %zu events each, identical = %s", first.size(), same ? "yes" : "no"));
}

// ---------------------------------------------------------------------------------------------------------------------
// state copy (save states): a copy of the scheduler taken mid-run must continue exactly like the original
// ---------------------------------------------------------------------------------------------------------------------

namespace
{
struct SnapEnv
{
	Scheduler_t* cur = nullptr;
	EventId id[2] = {INVALID_EVENT_ID, INVALID_EVENT_ID};
	std::vector<std::pair<u64, int>> fires;
};
struct SnapBind
{
	SnapEnv* env;
	int which;
};
void snap_handler(void* u, int c)
{
	auto* b = static_cast<SnapBind*>(u);
	SnapEnv& e = *b->env;
	const u64 now = e.cur->get_current_cycles();
	e.fires.push_back({now, b->which * 1000 + c});
	if (b->which == 0)
		e.cur->schedule(e.id[0], now + 7, c);
	else
		e.cur->schedule(e.id[1], now + 11, c + 1);
}
} // namespace

TEST(copy_of_scheduler_continues_identically)
{
	SnapEnv env;
	SnapBind b0{&env, 0}, b1{&env, 1};
	Scheduler_t a;
	env.cur = &a;
	env.id[0] = a.register_event(&snap_handler, &b0, 0);
	env.id[1] = a.register_event(&snap_handler, &b1, 1);
	a.schedule(env.id[0], 7, 0);
	a.schedule(env.id[1], 5, 0);

	for (int i = 0; i < 50; i++) a.tick();
	Scheduler_t b = a;
	t.log.push_back(fmt("copied scheduler at cycle %llu, next event @%llu", ull(a.get_current_cycles()), ull(a.peek_next_timestamp())));

	env.fires.clear();
	for (int i = 0; i < 150; i++) a.tick();
	const auto fromOriginal = env.fires;

	env.fires.clear();
	env.cur = &b;
	for (int i = 0; i < 150; i++) b.tick();
	const auto fromCopy = env.fires;

	CHECK(!fromOriginal.empty());
	CHECK(fromOriginal == fromCopy);
	CHECK_EQ(a.get_current_cycles(), b.get_current_cycles());
	CHECK_EQ(a.peek_next_timestamp(), b.peek_next_timestamp());
	t.log.push_back(fmt("original fired %zu events, copy fired %zu events, both ended at cycle %llu", fromOriginal.size(), fromCopy.size(), ull(a.get_current_cycles())));
}

// ---------------------------------------------------------------------------------------------------------------------
// random operations compared with a simple reference model
// ---------------------------------------------------------------------------------------------------------------------

TEST(fuzz_against_reference_model)
{
	struct M { u64 t; int prio; u64 seq; int slot; int ctx; };
	const uint8_t prios[6] = {0, 0, 1, 2, 2, 3};
	int failedSeeds = 0;
	u64 totalFires = 0;

	for (int it = 0; it < opt.fuzzIters && failedSeeds == 0; it++)
	{
		const u64 seed = opt.seed + (u64)it;
		Harness h(t);
		h.logCalls = false;
		h.logEvents = false;
		t.log.clear();
		int slots[6];
		for (int i = 0; i < 6; i++) slots[i] = h.reg(fmt("T%d", i).c_str(), prios[i]);

		std::vector<M> model;
		std::vector<std::string> ops;
		u64 mseq = 0, mnow = 0;
		size_t consumed = 0;
		std::mt19937_64 rng(seed);
		bool ok = true;

		auto find = [&](int s, int c) -> int {
			for (size_t i = 0; i < model.size(); i++)
				if (model[i].slot == s && model[i].ctx == c) return (int)i;
			return -1;
		};
		auto bad = [&](int step, const std::string& what) {
			ok = false;
			failedSeeds++;
			t.log.push_back(fmt("seed %llu failed at step %d: %s", ull(seed), step, what.c_str()));
			t.log.push_back("last operations:");
			for (size_t i = ops.size() > 25 ? ops.size() - 25 : 0; i < ops.size(); i++) t.log.push_back("    " + ops[i]);
			fail_at(__FILE__, __LINE__, fmt("model mismatch, seed %llu step %d: %s", ull(seed), step, what.c_str()));
		};

		for (int step = 0; step < 5000 && ok; step++)
		{
			const int op = (int)(rng() % 100);
			if (op < 40)
			{
				const int s = (int)(rng() % 6), c = (int)(rng() % 4);
				if (find(s, c) != -1 || model.size() >= Scheduler_t::MAX_EVENTS) continue;
				const u64 target = mnow + 1 + ((rng() % 2) ? rng() % 8 : rng() % 300);
				ops.push_back(fmt("sched T%d ctx=%d @%llu (now %llu)", s, c, ull(target), ull(mnow)));
				h.at(slots[s], target, c);
				model.push_back({target, prios[s], mseq++, s, c});
			}
			else if (op < 55)
			{
				if (model.empty()) continue;
				const size_t i = rng() % model.size();
				ops.push_back(fmt("cancel T%d ctx=%d (was @%llu)", model[i].slot, model[i].ctx, ull(model[i].t)));
				h.cancel(slots[model[i].slot], model[i].ctx);
				model.erase(model.begin() + (long)i);
			}
			else if (op < 70)
			{
				if (model.empty()) continue;
				const size_t i = rng() % model.size();
				const u64 target = mnow + 1 + rng() % 100;
				bool clash = false;
				for (size_t j = 0; j < model.size(); j++)
					if (j != i && model[j].t == target && model[j].prio == model[i].prio) clash = true;
				if (clash) continue;
				ops.push_back(fmt("resched T%d ctx=%d @%llu -> @%llu", model[i].slot, model[i].ctx, ull(model[i].t), ull(target)));
				h.resched(slots[model[i].slot], target, model[i].ctx);
				model[i].t = target;
				model[i].seq = mseq++;
			}
			else
			{
				const int n = 1 + (int)(rng() % 3);
				for (int k = 0; k < n && ok; k++)
				{
					ops.push_back(fmt("tick -> %llu", ull(mnow + 1)));
					h.tick(1);
					mnow++;
					std::vector<M> due;
					for (size_t i = 0; i < model.size();)
					{
						if (model[i].t <= mnow)
						{
							due.push_back(model[i]);
							model.erase(model.begin() + (long)i);
						}
						else
							i++;
					}
					std::sort(due.begin(), due.end(), [](const M& x, const M& y) {
						if (x.t != y.t) return x.t < y.t;
						if (x.prio != y.prio) return x.prio < y.prio;
						return x.seq < y.seq;
					});
					std::vector<F> exp;
					for (const M& m : due) exp.push_back({m.t, slots[m.slot], m.ctx});
					std::vector<F> got(h.fires.begin() + (long)consumed, h.fires.end());
					consumed = h.fires.size();
					totalFires += got.size();
					if (!(exp == got)) bad(step, "fired events differ\n        expected: " + h.describe(exp) + "\n        actual:   " + h.describe(got));
				}
			}
			if (!ok) break;

			u64 expectNext = NO_EVENT;
			for (const M& m : model) expectNext = std::min(expectNext, m.t);
			if (h.sched.peek_next_timestamp() != expectNext) bad(step, fmt("peek_next_timestamp %llu, expected %llu", ull(h.sched.peek_next_timestamp()), ull(expectNext)));
			else if (h.now() != mnow) bad(step, fmt("clock %llu, expected %llu", ull(h.now()), ull(mnow)));
		}
	}
	if (failedSeeds == 0)
	{
		t.log.push_back(fmt("%d seeds (from %llu) x 5000 steps against the reference model, %llu events fired and compared", opt.fuzzIters, ull(opt.seed), ull(totalFires)));
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// probes: behaviour that is not decided yet. They only report what the implementation does.
// ---------------------------------------------------------------------------------------------------------------------

static std::string fires_str(const Harness& h) { return h.describe(h.fires); }

PROBE(probe_schedule_same_id_and_context_twice)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10, 0);
	h.at(a, 12, 0);
	h.run_to(20);
	const char* verdict = h.fires.size() == 2 ? "both are kept" : (h.fires.size() == 1 ? (h.fires[0].t == 12 ? "the second replaces the first" : "the second is ignored") : "other");
	t.info.push_back(fmt("same (id, ctx) scheduled @10 then @12: %s -> %s", verdict, fires_str(h).c_str()));
}

PROBE(probe_schedule_same_id_and_context_same_cycle)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10, 0);
	h.at(a, 10, 0);
	h.run_to(20);
	t.info.push_back(fmt("same (id, ctx) scheduled twice @10: fired %zu time(s) -> %s", h.fires.size(), fires_str(h).c_str()));
}

PROBE(probe_reschedule_same_cycle_order)
{
	Harness h(t);
	const int a = h.reg("A");
	const int b = h.reg("B");
	h.at(a, 10, 0);
	h.at(b, 10, 0);
	h.resched(a, 10, 0);
	h.run_to(10);
	const bool aFirst = !h.fires.empty() && h.fires[0].slot == a;
	t.info.push_back(fmt("A then B scheduled @10, then reschedule(A, 10): %s -> %s", aFirst ? "A keeps its place (reschedule keeps the original insertion order)" : "A goes after B (reschedule counts as a new insertion)", fires_str(h).c_str()));
}

PROBE(probe_cancel_with_duplicates)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 10, 0);
	h.at(a, 12, 0);
	h.cancel(a, 0);
	h.run_to(20);
	t.info.push_back(fmt("two pending (id, ctx), one cancel(): %zu remaining -> %s", h.fires.size(), fires_str(h).c_str()));
}

PROBE(probe_schedule_in_the_past)
{
	Harness h(t);
	const int a = h.reg("A");
	h.run_to(20);
	h.at(a, 5, 0);
	h.run_to(30);
	t.info.push_back(fmt("scheduled @5 while the clock is 20: %s", fires_str(h).c_str()));
}

PROBE(probe_schedule_at_current_cycle)
{
	Harness h(t);
	const int a = h.reg("A");
	h.run_to(20);
	h.at(a, 20, 0);
	h.run_to(30);
	t.info.push_back(fmt("scheduled @20 while the clock is 20: %s", fires_str(h).c_str()));
}

PROBE(probe_handler_schedules_same_cycle)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.at(b, h.now(), 0); });
	b = h.reg("B", 1);
	h.at(a, 10, 0);
	h.run_to(20);
	t.info.push_back(fmt("A @10 schedules B for the same cycle (10): %s", fires_str(h).c_str()));
}

PROBE(probe_handler_schedules_in_the_past)
{
	Harness h(t);
	int b = -1;
	const int a = h.reg("A", 0, [&](int) { h.at(b, h.now() - 3, 0); });
	b = h.reg("B", 1);
	h.at(a, 10, 0);
	h.run_to(20);
	t.info.push_back(fmt("A @10 schedules B for cycle 7: %s", fires_str(h).c_str()));
}

PROBE(probe_reschedule_not_pending)
{
	Harness h(t);
	const int a = h.reg("A");
	h.resched(a, 10, 0);
	h.run_to(20);
	t.info.push_back(fmt("reschedule() of an event that is not pending: %s", h.fires.empty() ? "nothing happens" : ("it is created -> " + fires_str(h)).c_str()));
}

PROBE(probe_cancel_not_pending)
{
	Harness h(t);
	const int a = h.reg("A");
	h.cancel(a, 0);
	h.at(a, 5, 1);
	h.cancel(a, 0);
	h.run_to(10);
	t.info.push_back(fmt("cancel() of events that are not pending: no crash, other events unaffected = %s", h.fires.size() == 1 ? "yes" : "NO"));
}

PROBE(probe_cancel_after_fire)
{
	Harness h(t);
	const int a = h.reg("A");
	h.at(a, 5, 0);
	h.run_to(6);
	h.cancel(a, 0);
	h.at(a, 9, 0);
	h.run_to(12);
	t.info.push_back(fmt("cancel() after the event fired, then scheduled again: %s", fires_str(h).c_str()));
}

PROBE(probe_handler_cancels_itself)
{
	Harness h(t);
	int a = -1;
	a = h.reg("A", 0, [&](int) { h.cancel(a, 0); });
	h.at(a, 5, 0);
	h.at(a, 8, 1);
	h.run_to(12);
	t.info.push_back(fmt("handler calls cancel() on its own (id, ctx): %s", fires_str(h).c_str()));
}

PROBE_CAPACITY(probe_schedule_over_capacity)
{
	Harness h(t);
	h.logCalls = false;
	const int a = h.reg("A");
	for (uint32_t c = 0; c < Scheduler_t::MAX_EVENTS + 1; c++) h.at(a, 100 + c, (int)c);
	h.run_to(100 + Scheduler_t::MAX_EVENTS + 2);
	t.info.push_back(fmt("scheduled MAX_EVENTS+1 (%u) events: %zu fired", (unsigned)(Scheduler_t::MAX_EVENTS + 1), h.fires.size()));
}

PROBE_CAPACITY(probe_register_over_capacity)
{
	Harness h(t);
	h.logCalls = false;
	EventId last = 0;
	for (uint32_t i = 0; i < Scheduler_t::MAX_EVENT_TYPES + 1; i++) last = h.id(h.reg(fmt("T%u", i).c_str()));
	t.info.push_back(fmt("registered MAX_EVENT_TYPES+1 (%u) types: the last returned id %u (%s)", (unsigned)(Scheduler_t::MAX_EVENT_TYPES + 1), (unsigned)last, last == INVALID_EVENT_ID ? "INVALID_EVENT_ID" : "valid id"));
}

// ---------------------------------------------------------------------------------------------------------------------
// benchmarks
// ---------------------------------------------------------------------------------------------------------------------

static void bench_report(TestCtx& t, const char* what, u64 ticks, double seconds, u64 fired)
{
	const double mticks = (double)ticks / seconds / 1e6;
	t.info.push_back(fmt("%-34s %8.1f M ticks/s  %6.2f ns/tick  = %6.1fx GBA speed  (%llu events fired)", what, mticks, seconds * 1e9 / (double)ticks, mticks / 16.777216, ull(fired)));
}

static double seconds_since(std::chrono::steady_clock::time_point t0)
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

BENCH(bench_tick_empty_queue)
{
	Scheduler_t s;
	const u64 n = 100000000ULL;
	const auto t0 = std::chrono::steady_clock::now();
	for (u64 i = 0; i < n; i++)
	{
		s.tick();
		BENCH_BARRIER();
	}
	bench_report(t, "tick(), empty queue", n, seconds_since(t0), 0);
	CHECK_EQ(s.get_current_cycles(), n);
}

BENCH(bench_tick_far_event_pending)
{
	Scheduler_t s;
	const EventId id = s.register_event([](void*, int) {}, nullptr, 0);
	s.schedule(id, (u64)1 << 60, 0);
	const u64 n = 100000000ULL;
	const auto t0 = std::chrono::steady_clock::now();
	for (u64 i = 0; i < n; i++)
	{
		s.tick();
		BENCH_BARRIER();
	}
	bench_report(t, "tick(), one far event pending", n, seconds_since(t0), 0);
}

BENCH(bench_dispatch_every_16_cycles)
{
	Scheduler_t s;
	struct Env { Scheduler_t* s; EventId id; u64 fired; } env{&s, INVALID_EVENT_ID, 0};
	env.id = s.register_event(
		[](void* u, int) {
			auto* e = static_cast<Env*>(u);
			e->fired++;
			e->s->schedule(e->id, e->s->get_current_cycles() + 16, 0);
		},
		&env, 0);
	s.schedule(env.id, 16, 0);
	const u64 n = 50000000ULL;
	const auto t0 = std::chrono::steady_clock::now();
	for (u64 i = 0; i < n; i++)
	{
		s.tick();
		BENCH_BARRIER();
	}
	bench_report(t, "one event every 16 cycles", n, seconds_since(t0), env.fired);
}

BENCH(bench_dispatch_every_cycle)
{
	Scheduler_t s;
	struct Env { Scheduler_t* s; EventId id; u64 fired; } env{&s, INVALID_EVENT_ID, 0};
	env.id = s.register_event(
		[](void* u, int) {
			auto* e = static_cast<Env*>(u);
			e->fired++;
			e->s->schedule(e->id, e->s->get_current_cycles() + 1, 0);
		},
		&env, 0);
	s.schedule(env.id, 1, 0);
	const u64 n = 20000000ULL;
	const auto t0 = std::chrono::steady_clock::now();
	for (u64 i = 0; i < n; i++)
	{
		s.tick();
		BENCH_BARRIER();
	}
	bench_report(t, "one event every cycle (worst case)", n, seconds_since(t0), env.fired);
}

BENCH(bench_gba_like_100_frames)
{
	GbaLike g;
	const u64 n = GbaLike::FRAME * 100;
	const auto t0 = std::chrono::steady_clock::now();
	for (u64 i = 0; i < n; i++)
	{
		g.s.tick();
		BENCH_BARRIER();
	}
	const u64 fired = g.hblanks + g.lineEnds + g.frameSeqs + g.samples + g.timerFires[0] + g.timerFires[1] + g.timerFires[2] + g.timerFires[3];
	bench_report(t, "GBA-like (scanlines, timers, apu)", n, seconds_since(t0), fired);
}

// ---------------------------------------------------------------------------------------------------------------------
// runner
// ---------------------------------------------------------------------------------------------------------------------

static void print_usage()
{
	std::printf("scheduler_test [-v] [-q] [--list] [--filter text] [--seed N] [--fuzz-iters N] [--timeout seconds] [--probe] [--probe-capacity] [--bench]\n");
}

static std::atomic<const char*> g_runningTest{nullptr};
static std::atomic<long long> g_testStartMs{0};

static long long now_ms()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void start_watchdog()
{
	if (opt.timeoutSec <= 0) return;
	std::thread([] {
		for (;;)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			const char* name = g_runningTest.load();
			if (name && now_ms() - g_testStartMs.load() > (long long)opt.timeoutSec * 1000)
			{
				std::printf("[ HANG   ] %s  running for more than %d s, probably an infinite loop (for example a dispatch loop that never ends, or a handler that keeps scheduling an event that is already due)\n", name, opt.timeoutSec);
				std::fflush(stdout);
				std::_Exit(3);
			}
		}
	}).detach();
}

static bool selected(const TestCase& tc)
{
	switch (tc.kind)
	{
	case Kind::Normal: break;
	case Kind::Probe: if (!opt.probe) return false; break;
	case Kind::ProbeCapacity: if (!opt.probeCapacity) return false; break;
	case Kind::Bench: if (!opt.bench) return false; break;
	}
	return opt.filter.empty() || std::string(tc.name).find(opt.filter) != std::string::npos;
}

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		const std::string a = argv[i];
		if (a == "-v") opt.verbose = true;
		else if (a == "-q") opt.quiet = true;
		else if (a == "--list") opt.list = true;
		else if (a == "--probe") opt.probe = true;
		else if (a == "--probe-capacity") opt.probeCapacity = true;
		else if (a == "--bench") opt.bench = true;
		else if (a == "--filter" && i + 1 < argc) opt.filter = argv[++i];
		else if (a == "--seed" && i + 1 < argc) opt.seed = std::strtoull(argv[++i], nullptr, 10);
		else if (a == "--fuzz-iters" && i + 1 < argc) opt.fuzzIters = std::atoi(argv[++i]);
		else if (a == "--timeout" && i + 1 < argc) opt.timeoutSec = std::atoi(argv[++i]);
		else
		{
			print_usage();
			return a == "-h" || a == "--help" ? 0 : 2;
		}
	}

	if (opt.list)
	{
		for (const TestCase& tc : registry()) std::printf("%-8s %s\n", tc.kind == Kind::Normal ? "test" : (tc.kind == Kind::Bench ? "bench" : "probe"), tc.name);
		return 0;
	}

	start_watchdog();

	int passed = 0, failed = 0, infos = 0;
	std::vector<std::string> failedNames;
	struct Row { std::string name; const char* result; double ms; };
	std::vector<Row> rows;
	const auto allStart = std::chrono::steady_clock::now();

	for (const TestCase& tc : registry())
	{
		if (!selected(tc)) continue;

		TestCtx ctx;
		g_ctx = &ctx;
		std::printf("[ RUN    ] %s\n", tc.name);
		std::fflush(stdout);
		const auto t0 = std::chrono::steady_clock::now();
		g_testStartMs = now_ms();
		g_runningTest = tc.kind == Kind::Bench ? nullptr : tc.name;
		try
		{
			tc.fn(ctx);
		}
		catch (const std::exception& e)
		{
			ctx.failures.push_back(std::string("exception: ") + e.what());
		}
		g_runningTest = nullptr;
		const double ms = seconds_since(t0) * 1000.0;

		const bool failedNow = !ctx.failures.empty();
		if (!opt.quiet)
		{
			const size_t limit = opt.verbose ? ctx.log.size() : (failedNow ? 120 : 40);
			for (size_t i = 0; i < ctx.log.size() && i < limit; i++) std::printf("    | %s\n", ctx.log[i].c_str());
			if (ctx.log.size() > limit) std::printf("    | ... %zu more log lines (-v to show all)\n", ctx.log.size() - limit);
		}
		for (const std::string& line : ctx.info) std::printf("    * %s\n", line.c_str());
		for (const std::string& f : ctx.failures) std::printf("    ! %s\n", f.c_str());

		const bool isInfo = tc.kind != Kind::Normal;
		const char* result = failedNow ? "FAIL" : (isInfo ? "INFO" : "PASS");
		std::printf("[ %-6s ] %s  (%.3f ms)\n\n", result, tc.name, ms);
		if (failedNow) { failed++; failedNames.push_back(tc.name); }
		else if (isInfo) infos++;
		else passed++;
		rows.push_back({tc.name, result, ms});
		g_ctx = nullptr;
	}

	std::printf("==================== summary ====================\n");
	for (const Row& r : rows) std::printf("  %-6s %-52s %10.3f ms\n", r.result, r.name.c_str(), r.ms);
	std::printf("-------------------------------------------------\n");
	std::printf("  %d passed, %d failed, %d info   (%.1f ms total)\n", passed, failed, infos, seconds_since(allStart) * 1000.0);
	if (failed)
	{
		std::printf("  failed:");
		for (const std::string& n : failedNames) std::printf(" %s", n.c_str());
		std::printf("\n");
	}
	if (!opt.probe && !opt.bench)
	{
		std::printf("  (--probe shows what your implementation does for the open decisions, --bench measures tick() speed)\n");
	}
#ifndef NDEBUG
	if (opt.bench) std::printf("  note: NDEBUG is not defined, benchmark numbers are for a debug build\n");
#endif
	return failed ? 1 : 0;
}
