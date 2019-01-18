#ifndef __MAIN_H
#define __MAIN_H

#include <cassert>

//#define DEBUG
//#define RT_SIMULATOR
#define INTRA_SCHED
#define NS 1
#define US 1000
#define S_MS 1000000
#define SEC 1000000000
#define INST_UNIT 1 // Each instruction takes 1 execution cycles (=1 CPI)
#define N_DECIMAL_POINTS_PRECISION 1000.0 // To extracting three decimal points

#define tasks_num 5
#define patterns_num 50
#define EVAL_CNT_START 100 // Start to use the profiling data from #100 instance

#define TSK_SET_1_COM_NUM 3
#define TSK_SET_1_TSK_NUM 5

#define TSK_NONDVFS_FOR_START_NUM 1
const unsigned int NonDVFS_For_Start[TSK_NONDVFS_FOR_START_NUM] = {4};

//#define PATTERN_GEN

//#define AVERAGE_INTERFERENCE_EVAL

/*
 cur_freq (MHz)
 cur_time (us)
*/
typedef struct Sys_Clk {
	double cur_freq; // The current execution speed for the system
	double cur_time;  // The current system tick (elapsed time so far) 
	int time_unit; // Unit of time (ms)
} sys_clk_t;

enum {
	WORST   = 1, // The worst case
	AVERAGE = 2, // The average case
	BEST    = 3  // The best case
};

enum {
	ATTRIBUTES_ITEM = 0x01, // "{}" for task attributes
	TASK_ATTRIBUTES = 0x02, // For contents of task attributes
	SUCCESSORS_ITEM = 0x03, // "{}" for its successors
	SUCCESSORS	= 0x04  // For contents of its successors
};

enum {
	TASK_ID = 0x05,
	WCEC    = 0x06,
	ACEC    = 0x07,
	BCEC    = 0x08
};

enum {
	LOOP      = 0x01, // Belonging to a certain loop nest
	NON_LOOP  = 0x02, // Not inside any loop nest
	LOOP_EXIT = 0x03  // The exit point of a certain loop nest
};

enum {
	NOT_TAKEN     = 1, // If branch instruction was not taken
	TAKEN         = 3  // If branch instruction was taken
};

enum {
	NORMAL    = 1, // Wihout DVFS ability
	H_PREDICT = 2, // High Predicatability
	L_POWER   = 3, // Low Power 
	L_JITTER  = 4  // Low Absolute Finish Time Jitter demand
};

enum {
	START_TIME      	= 0x01,
	FREQ_SCALING_POINT      = 0x02,
	PREEMPTED_POINT 	= 0x03,
	RESUME_POINT    	= 0x04,
	COMPLETION_TIME 	= 0x05,
	SYS_POWER_EVAL_POINT 	= 0x06
};

enum {
	NonDVFS_sim        = 1, // The settings without DVFS
	DVFS_sim           = 2, // The settings of DVFS regardless transition overhead
	DVFSOverhead_sim   = 3, // The settings of DVFS regarding to transition overhead
	DVFSAlphaBound_sim = 4, // The settings of DVFS with bounded Alpha
	DVFSAvgResp_sim    = 5  // The settings of DVFS with dynamic Alpha basing on means of actual response time
};

typedef int exeTime_info[3];

#endif // __MAIN_H

