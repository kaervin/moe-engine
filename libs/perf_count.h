#include <stdint.h>

uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

#define START_TIME Debug_Timer dbg_timer__FUNCTION__ = time_function_start(__COUNTER__, (char*)__FUNCTION__);

#define END_TIME time_function_end(dbg_timer__FUNCTION__);

typedef struct Debug_Timer {
	struct timespec start_time;
	uint64_t start_rdtsc;
	char *fnc_name;
	int counter;
} Debug_Timer;

typedef struct Perf_Measurement {
	double time;
	uint64_t rdtsc_stamp;
	char *fnc_name;
	uint64_t hit;
} Perf_Measurement;

#ifdef PERF_MAIN
Perf_Measurement perf_measurements[];
#endif

#ifdef PERF_GAMESTEP
Perf_Measurement *perf_measurements_gamestep;
#endif

Debug_Timer time_function_start(int cnt, char *fnc_name) {
	//printf("start %s %i\n", fnc_name, cnt);
	Debug_Timer dbg_timer;
	dbg_timer.fnc_name = fnc_name;
	dbg_timer.counter = cnt;
	dbg_timer.start_rdtsc = rdtsc();
	clock_gettime(CLOCK_MONOTONIC, &dbg_timer.start_time);
	return dbg_timer;
}

static void time_function_end(Debug_Timer dbg_timer) {
	struct timespec end_time;
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	double time_nano = end_time.tv_nsec - dbg_timer.start_time.tv_nsec;
	double time_milli = time_nano / 1000000.0f;
	time_t time_sec = end_time.tv_sec - dbg_timer.start_time.tv_sec;
	//printf("end %s %f %li\n", dbg_timer.fnc_name, time_milli, time_sec);
	Perf_Measurement p;
	p.fnc_name = dbg_timer.fnc_name;
	
	uint64_t delta_rdtsc = rdtsc() - dbg_timer.start_rdtsc;
	//printf(":: %s %i\n", dbg_timer.fnc_name, dbg_timer.counter);
#ifdef PERF_MAIN
	p.hit = perf_measurements[dbg_timer.counter].hit + 1;
	p.time = perf_measurements[dbg_timer.counter].time + time_milli;
	//p.rdtsc_stamp = perf_measurements[dbg_timer.counter].rdtsc_stamp + delta_rdtsc;
	perf_measurements[dbg_timer.counter] = p;
#endif
#ifdef PERF_GAMESTEP
	p.hit = perf_measurements_gamestep[dbg_timer.counter].hit + 1;
	p.time = perf_measurements_gamestep[dbg_timer.counter].time + time_milli;
	//p.rdtsc_stamp = perf_measurements_gamestep[dbg_timer.counter].rdtsc_stamp + delta_rdtsc;
	perf_measurements_gamestep[dbg_timer.counter] = p;
	//printf("end gmstep: %s %i\n", dbg_timer.fnc_name, dbg_timer.counter);
#endif
}