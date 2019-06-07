#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "../inc/cfg_info.h"
#include "../inc/main.h"
#include "../inc/dvfs_info.h"
//#include "../inc/rt_simulator.h"
#include "../inc/sched.h"
#include "../inc/timer.h"
#include "../inc/inter_bus.h"
#include "../inc/checkpoint_info.h"
#include "../Parser/parser.h"
#include "../inc/pattern_gen.h"
#include "../inc/result_export.h"

using std::cout;
using std::cin;
using std::endl;
using std::vector;

//-----------------------------------------------------------------------------------------//
// Functions
void system_init(void);
void wcet_info_config(void);
void checkpoint_config(char *FileName);
void TestPattern_config(void);
void array_int_cpy(vector<int> &Dst, int *Src);
void simulation_exe_path(Src_CFG &task, int path, double release_time, double start_time, int dvfs_en);
void func_gen(void *inout);
void export_result(void);
void verify_TestPattern_Consistency(void);
void export_TestPattern(unsigned int i);
void export_TestPattern_LaIteration(unsigned int i);
void SimSchedule(char env);
double show_minNRT(void* TskSched_in);
void verify_ParsedTestPattern(int tsk_id);
void verify_ParsedLaIterationPattern(int tsk_id);
//void priority_assignment(void);
//-----------------------------------------------------------------------------------------//
//Input parameters
#define TSK_SET_SIM 1
double PeriodSet_TskSet_1[TSK_SET_1_COM_NUM][TSK_SET_1_TSK_NUM] = {
	// {bs.c, compress.c, cfg_1, matmul.c, ludcmp.c}
	{75582.0, 173189.0, 164546.0, 9110699.0, 84239.0},
	{162500, 35949, 35951, 37807900, 121349},		
	{84239.0, 75582.0, 164546.0, 9110699.0, 173189.0}
};

int PrtSet_TskSet_1[TSK_SET_1_COM_NUM][TSK_SET_1_TSK_NUM] = {
	{0, 3, 2, 4, 1},
	{3, 0, 1, 4, 2},
	{1, 0, 2, 4, 3}
};
double in_alpha[tasks_num];
double *in_default_speed;
double TskSet_default_speed[TSK_SET_1_COM_NUM][TSK_SET_1_TSK_NUM] = {
	// {bs.c, compress.c, cfg_1, matmul.c, ludcmp.c}
	 {1000.0, 1000.0, 1000.0, 1000.0, 1000.0},
	 {1000.0, 1000.0, 1000.0, 1000.0, 1000.0},
	 {720.0, 800.0, 600.0, 1000.0, 1000.0}
};
//double energy_ref;
//double energy_ref_overhead;
char msg[41];
Parser parsing;
const char *Checkpoint_FileName = "../checkpoint/checkpoint.txt";
//-----------------------------------------------------------------------------------------//
//Parameters for simulation
ExePath_set TestPatternSet_inout;
int **LaIterationPatternSet_inout;
int sim_cnt;
sys_clk_t Sys_Clk_NonDVFS;
sys_clk_t Sys_Clk;
sys_clk_t Sys_Clk_overhead;
sys_clk_t Sys_Clk_AlphaBound;
sys_clk_t Sys_Clk_AvgResp;
Time_Management *time_management_NonDVFS;
Time_Management *time_management;
Time_Management *time_management_overhead;
Time_Management *time_management_AlphaBound;
Time_Management *time_management_AvgResp;
Task_State_Bus *inter_intra_bus_NonDVFS;
Task_State_Bus *inter_intra_bus;
Task_State_Bus *inter_intra_bus_overhead;
Task_State_Bus *inter_intra_bus_AlphaBound;
Task_State_Bus *inter_intra_bus_AvgResp;
Task_Scheduler *task_sched_NonDVFS;
Task_Scheduler *task_sched;
Task_Scheduler *task_sched_overhead;
Task_Scheduler *task_sched_AlphaBound;
Task_Scheduler *task_sched_AvgResp;
extern double ISR_TIME_SLICE;
extern int cur_TskID;
//int tasks_num =3 ; // The number of tasks 
//int patterns_num = 5;
vector<Src_CFG> src_intra_NonDVFS;
vector<Src_CFG> src_intra;
vector<Src_CFG> src_intra_overhead;
vector<Src_CFG> src_intra_AlphaBound;
vector<Src_CFG> src_intra_AvgResp;
task_info_t *src_inter_NonDVFS;
task_info_t *src_inter;
task_info_t *src_inter_overhead;
task_info_t *src_inter_AlphaBound;
task_info_t *src_inter_AvgResp;
CSVWriter **writer; // Creating an object of CSVWriter
int Tsk_SysMode_NonDVFS_sim[tasks_num] = {(int) NORMAL, (int) NORMAL, (int) NORMAL, (int) NORMAL, (int) NORMAL/*, (int) NORMAL, (int) NORMAL*/};
int Tsk_SysMode_DVFS_sim[tasks_num] = {(int) H_PREDICT, (int) H_PREDICT, (int) NORMAL, (int) L_POWER, (int) NORMAL/*, (int) h_PREDICT, (int) H_PREDICT*/};
int Tsk_SysMode_overhead_sim[tasks_num] = {(int) L_JITTER, (int) L_JITTER, (int) NORMAL, (int) L_POWER, (int) NORMAL/*, (int) NORMAL, (int) NORMAL*/};
int Tsk_SysMode_AlphaBound_sim[tasks_num] = {(int) L_JITTER, (int) NORMAL, (int) NORMAL, (int) NORMAL, (int) NORMAL/*, (int) NORMAL, (int) NORMAL*/};
int Tsk_SysMode_AvgResp_sim[tasks_num] = {(int) L_JITTER, (int) NORMAL, (int) NORMAL, (int) NORMAL, (int) NORMAL/*, (int) NORMAL, (int) NORMAL*/};
int sim_mode; // For recognising what's the file name of each stage should be, according to different simulation environments

//-----------------------------------------------------------------------------------------//
//Temporary test cases
typedef vector<int> ExePath_case;
typedef vector<ExePath_case> ExePath_set;
//-----------------------------------------------------------------------------------------//
//Checkpoints Objects
void checkpoint_config();
exeTime_info task_wcet_info_t[tasks_num] = {
        { // Tsk 0 (bs.c)
         9750, // The worst-case execution cycle(s)
         0    , // The average-case execution cycle(s)
         1000   // The best-case exeucution cycle(s)
        },

        { // Tsk 1 (compress.c)
         11950, // The worst-case execution cycle(s)
         0    , // The average-case execution cycle(s)
         52     // The best-case exeucution cycle(s)
        },

        { // Tsk 2 (cfg_1)
         1810, // The worst-case execution cycle(s)
         0   , // The average-case execution cycle(s)
         260   // The best-case exeucution cycle(s)
        },

        { // Tsk 3 (matmult.c)
         1890395, // The worst-case execution cycle(s)
         0      , // The average-case execution cycle(s)
         95       // The best-case exeucution cycle(s)
        },
        
	{ // Tsk 4 (ludcmp.c)
         27546, // The worst-case execution cycle(s)
         0    , // The average-case execution cycle(s)
         338    // The best-case exeucution cycle(s)
        }/*,
        
	{ // Tsk 5 (st.c)
         44769, // The worst-case execution cycle(s)
         0    , // The average-case execution cycle(s)
         1293   // The best-case exeucution cycle(s)
        },
        
	{ // Tsk 6 (crc.c)
         74626, // The worst-case execution cycle(s)
         0    , // The average-case execution cycle(s)
         448    // The best-case exeucution cycle(s)
        }*/
};
exeTime_info *task_wcet_info;
checkpoint_num *checkpoint_num_t; // The size will be known after parsing
RWCEC_Trace_in *cycle_trace; // The real size will be defined after parsing
checkpoints_label *checkpointLabel; // The label of checkpoints at each task's Basic Block(s)
ExePath_set *exe_path; // The set of exeuction-path set for tasks
// Three instaneces for each of those two tasks
int instance_case[2][3] = {
        {5, 3, 0}, // Task 1: exe_path_5(first instance), exe_path_3(second instance), exe_path_0(third instance)
        {5, 3, 1}  // Task 2: exe_path_5(first instance), exe_path_3(second instance), exe_path_1(third instance) 
};
//-----------------------------------------------------------------------------------------//
int main(int argc, char **argv)
{
#ifndef PATTERN_GEN
	if(argc != (2 + tasks_num)) {
		cout << "Input Format: Task_1's alpha, Task_2's alpha, ... Task_N's alpha, default operating frequency" << endl;
		exit(1);
	}
	else {
		for(unsigned int i = 0; i < tasks_num; i++) in_alpha[i] = ((double) atoi(argv[1 + i])) / 100;
		//in_default_speed = (double) atoi(argv[1 + tasks_num]);
	}
#else
	system("rm ../TestPattern/*Pattern*.txt");
	for(unsigned int i = 0; i < tasks_num; i++) in_alpha[i] = (double) 1.0; // dummy assignment
	cout << "The number of Test Pattern is: " << patterns_num << endl;
	//in_default_speed = (double) atoi(argv[1 + tasks_num]);
#endif
//=======================================================================================================================================================//
// Settings of each task's CFG information
	cout << "Preset 1) Starting to initialise system" << endl;
	system_init();

	cout << "Preset 2) Configure Benchmark Programs' CFGs, etc." << endl;	
	Src_CFG task1_cfg(
		(char*) "../cfg/bs.cfg", 
		time_management, 
		&checkpointLabel[0], 
		&cycle_trace[0], 
		&checkpoint_num_t[0], 
		task_wcet_info[0], 
		in_alpha[0], 
		Tsk_SysMode_NonDVFS_sim[0],
		(int) 0
	); cout << "\tPreset 2.1) Finished configuring task1" << endl;
	Src_CFG task2_cfg(
		(char*) "../cfg/compress.cfg", 
		time_management, 
		&checkpointLabel[1], 
		&cycle_trace[1], 
		&checkpoint_num_t[1], 
		task_wcet_info[1], 
		in_alpha[1], 
		Tsk_SysMode_NonDVFS_sim[1],
		(int) 1
	); cout << "\tPreset 2.1) Finished configuring task2" << endl;
	Src_CFG task3_cfg(
		(char*) "../cfg/cfg_1.cfg", 
		time_management, 
		&checkpointLabel[2], 
		&cycle_trace[2], 
		&checkpoint_num_t[2], 
		task_wcet_info[2], 
		in_alpha[2],
		Tsk_SysMode_NonDVFS_sim[2], 
		(int) 2
	); cout << "\tPreset 2.1) Finished configuring task3" << endl;
	Src_CFG task4_cfg(
		(char*) "../cfg/matmult.cfg", 
		time_management, 
		&checkpointLabel[3], 
		&cycle_trace[3], 
		&checkpoint_num_t[3], 
		task_wcet_info[3], 
		in_alpha[3],
		Tsk_SysMode_NonDVFS_sim[3], 
		(int) 3
	); cout << "\tPreset 2.1) Finished configuring task4" << endl;
	Src_CFG task5_cfg(
		(char*) "../cfg/ludcmp.cfg", 
		time_management, 
		&checkpointLabel[4], 
		&cycle_trace[4], 
		&checkpoint_num_t[4], 
		task_wcet_info[4], 
		in_alpha[4],
		Tsk_SysMode_NonDVFS_sim[4], 
		(int) 4
	); cout << "\tPreset 2.1) Finished configuring task5" << endl;
/*
	Src_CFG task6_cfg(
		(char*) "../cfg/st.cfg", 
		time_management, 
		&checkpointLabel[5], 
		&cycle_trace[5], 
		&checkpoint_num_t[5], 
		task_wcet_info[5], 
		in_alpha[5],
		Tsk_SysMode_NonDVFS_sim[5], 
		(int) 5
	); cout << "\tPreset 2.1) Finished configuring task3" << endl;
	Src_CFG task7_cfg(
		(char*) "../cfg/crc.cfg", 
		time_management, 
		&checkpointLabel[6], 
		&cycle_trace[6], 
		&checkpoint_num_t[6], 
		task_wcet_info[6], 
		in_alpha[6],
		Tsk_SysMode_NonDVFS_sim[6], 
		(int) 6
	); cout << "\tPreset 2.1) Finished configuring task3" << endl;
*/
	//in_default_speed = (double) atoi(argv[1 + tasks_num]);
	task1_cfg.global_param_init();
	task2_cfg.global_param_init(); task3_cfg.global_param_init();
	task4_cfg.global_param_init(); task5_cfg.global_param_init();
	src_intra.push_back(task1_cfg); src_intra.push_back(task2_cfg); src_intra.push_back(task3_cfg);
	src_intra.push_back(task4_cfg); src_intra.push_back(task5_cfg); 
	
	//in_default_speed = 800.0; 
	task1_cfg.timer_config(time_management_NonDVFS); task1_cfg.global_param_init();  
	task2_cfg.timer_config(time_management_NonDVFS); task2_cfg.global_param_init();  
	task3_cfg.timer_config(time_management_NonDVFS); task3_cfg.global_param_init();  
	task4_cfg.timer_config(time_management_NonDVFS); task4_cfg.global_param_init();  
	task5_cfg.timer_config(time_management_NonDVFS); task5_cfg.global_param_init();  
	src_intra_NonDVFS.push_back(task1_cfg); src_intra_NonDVFS.push_back(task2_cfg); src_intra_NonDVFS.push_back(task3_cfg);
	src_intra_NonDVFS.push_back(task4_cfg); src_intra_NonDVFS.push_back(task5_cfg); 
	
	//in_default_speed = (double) atoi(argv[1 + tasks_num]);
	task1_cfg.timer_config(time_management_overhead); task1_cfg.global_param_init();  
	task2_cfg.timer_config(time_management_overhead); task2_cfg.global_param_init();  
	task3_cfg.timer_config(time_management_overhead); task3_cfg.global_param_init();  
	task4_cfg.timer_config(time_management_overhead); task4_cfg.global_param_init();  
	task5_cfg.timer_config(time_management_overhead); task5_cfg.global_param_init();  
	src_intra_overhead.push_back(task1_cfg); src_intra_overhead.push_back(task2_cfg); src_intra_overhead.push_back(task3_cfg);
	src_intra_overhead.push_back(task4_cfg); src_intra_overhead.push_back(task5_cfg); 
	
	//in_default_speed = (double) atoi(argv[1 + tasks_num]);
	task1_cfg.timer_config(time_management_AlphaBound); task1_cfg.global_param_init();  
	task2_cfg.timer_config(time_management_AlphaBound); task2_cfg.global_param_init();  
	task3_cfg.timer_config(time_management_AlphaBound); task3_cfg.global_param_init();  
	task4_cfg.timer_config(time_management_AlphaBound); task4_cfg.global_param_init();  
	task5_cfg.timer_config(time_management_AlphaBound); task5_cfg.global_param_init();  
	src_intra_AlphaBound.push_back(task1_cfg); src_intra_AlphaBound.push_back(task2_cfg); src_intra_AlphaBound.push_back(task3_cfg);
	src_intra_AlphaBound.push_back(task4_cfg); src_intra_AlphaBound.push_back(task5_cfg); 

	//in_default_speed = (double) atoi(argv[1 + tasks_num]);
	task1_cfg.timer_config(time_management_AvgResp); task1_cfg.global_param_init();  
	task2_cfg.timer_config(time_management_AvgResp); task2_cfg.global_param_init();  
	task3_cfg.timer_config(time_management_AvgResp); task3_cfg.global_param_init();  
	task4_cfg.timer_config(time_management_AvgResp); task4_cfg.global_param_init();  
	task5_cfg.timer_config(time_management_AvgResp); task5_cfg.global_param_init();  
	src_intra_AvgResp.push_back(task1_cfg); src_intra_AvgResp.push_back(task2_cfg); src_intra_AvgResp.push_back(task3_cfg);
	src_intra_AvgResp.push_back(task4_cfg); src_intra_AvgResp.push_back(task5_cfg); 

	cout << "\tPreset 2.2) Verify the correctness among CFGs file and Checkpoint Annotations " <<  endl;
	cout << "\t\tThe number of Intra-Source: " << src_intra.size() << endl;
	if(tasks_num != src_intra.size()) {
		cout << "\t\t\tIn the setting of Non-DVFS Intra-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	if(tasks_num != src_intra_NonDVFS.size()) {
		cout << "\t\t\tIn the setting of DVFS Intra-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	if(tasks_num != src_intra_overhead.size()) {
		cout << "\t\t\tIn the setting of DVFS Intra-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	if(tasks_num != src_intra_AlphaBound.size()) {
		cout << "\t\t\tIn the setting of DVFS Intra-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	if(tasks_num != src_intra_AvgResp.size()) {
		cout << "\t\t\tIn the setting of DVFS Intra-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	cout << "Preset 3) Starting to configure test patterns for all tasks" << endl;
	TestPattern_config();
	cout << "\tFinished configuring settings of test patterns" << endl;
//=======================================================================================================================================================//
// Settings of Inter-task	
	cout << "Preset 4) Starting to configure Inter tasks" << endl;
	Ready_Queue que_NonDVFS, que, que_overhead, que_AlphaBound, que_AvgResp;
	task_info_t *task = new task_info_t[tasks_num];
	task[0] = { // bs.c
			     0.0, // Release Time
			     0.0, // Start Time 
			     PrtSet_TskSet_1[TSK_SET_SIM][0]  , // Priority
			     PeriodSet_TskSet_1[TSK_SET_SIM][0], // Relative Deadline
			     task1_cfg.wcet,//task1.wcet, // WCET
			     task1_cfg.bcet,//task1.bcet, // BCET
		             PeriodSet_TskSet_1[TSK_SET_SIM][0], // Period
			     false, 
			     (char) ZOMBIE, // Default Task State
			     task1_cfg.wcet, // Default WCRT
			     task1_cfg.bcet, // Default BCRT
			     (unsigned int) 0
	};

	task[1] = { // compress.c
			     0.0, // Release Time
			     0.0, // Start Time 
			     PrtSet_TskSet_1[TSK_SET_SIM][1]  , // Priority
			     PeriodSet_TskSet_1[TSK_SET_SIM][1], // Relative Deadline
			     task2_cfg.wcet, // WCET
			     task2_cfg.bcet, // BCET
		             PeriodSet_TskSet_1[TSK_SET_SIM][1], // Period
			     false, 
			     (char) ZOMBIE, // Default Task State
			     task2_cfg.wcet, // Default WCRT
			     task2_cfg.bcet, // Default BCRT
			     (unsigned int) 0
	};
	
	task[2] = { // cfg_1
			     0.0, // Release Time
			     0.0, // Start Time 
			     PrtSet_TskSet_1[TSK_SET_SIM][2]  , // Priority
			     PeriodSet_TskSet_1[TSK_SET_SIM][2], // Relative Deadline
			     task3_cfg.wcet, // WCET
			     task3_cfg.bcet, // BCET
		             PeriodSet_TskSet_1[TSK_SET_SIM][2], // Period
			     false, 
			     (char) ZOMBIE, // Default Task State
			     task3_cfg.wcet, // Default WCRT
			     task3_cfg.bcet, // Default BCRT
			     (unsigned int) 0
	};
	task[3] = { // matmul.c
			     0.0, // Release Time
			     0.0, // Start Time 
			     PrtSet_TskSet_1[TSK_SET_SIM][3]  , // Priority
			     PeriodSet_TskSet_1[TSK_SET_SIM][3], // Relative Deadline
			     task4_cfg.wcet, // WCET
			     task4_cfg.bcet, // BCET
		             PeriodSet_TskSet_1[TSK_SET_SIM][3], // Period
			     false, 
			     (char) ZOMBIE, // Default Task State
			     task4_cfg.wcet, // Default WCRT
			     task4_cfg.bcet, // Default BCRT
			     (unsigned int) 0
	};
	task[4] = { // ludcmp.c
			     0.0, // Release Time
			     0.0, // Start Time 
			     PrtSet_TskSet_1[TSK_SET_SIM][4]  , // Priority
			     PeriodSet_TskSet_1[TSK_SET_SIM][4], // Relative Deadline
			     task5_cfg.wcet, // WCET
			     task5_cfg.bcet, // BCET
		             PeriodSet_TskSet_1[TSK_SET_SIM][4], // Period
			     false, 
			     (char) ZOMBIE, // Default Task State
			     task5_cfg.wcet, // Default WCRT
			     task5_cfg.bcet, // Default BCRT
			     (unsigned int) 0
	};
	//src_inter.push_back(task[0]); src_inter.push_back(task[1]); src_inter.push_back(task[2]);
	//src_inter_NonDVFS.push_back(task[0]); src_inter_NonDVFS.push_back(task[1]); src_inter_NonDVFS.push_back(task[2]);
	src_inter_NonDVFS          = new task_info_t[tasks_num];
	src_inter                  = new task_info_t[tasks_num]; 
	src_inter_overhead         = new task_info_t[tasks_num]; 
	src_inter_AlphaBound       = new task_info_t[tasks_num]; 
	src_inter_AvgResp          = new task_info_t[tasks_num]; 
	memcpy(&src_inter_NonDVFS[0], &task[0], sizeof(task[0])); 
	memcpy(&src_inter_NonDVFS[1], &task[1], sizeof(task[1])); 
	memcpy(&src_inter_NonDVFS[2], &task[2], sizeof(task[2]));
	memcpy(&src_inter_NonDVFS[3], &task[3], sizeof(task[3]));
	memcpy(&src_inter_NonDVFS[4], &task[4], sizeof(task[4]));
	memcpy(&src_inter[0], &task[0], sizeof(task[0])); 
	memcpy(&src_inter[1], &task[1], sizeof(task[1])); 
	memcpy(&src_inter[2], &task[2], sizeof(task[2]));
	memcpy(&src_inter[3], &task[3], sizeof(task[3]));
	memcpy(&src_inter[4], &task[4], sizeof(task[4]));
	memcpy(&src_inter_overhead[0], &task[0], sizeof(task[0])); 
	memcpy(&src_inter_overhead[1], &task[1], sizeof(task[1])); 
	memcpy(&src_inter_overhead[2], &task[2], sizeof(task[2]));
	memcpy(&src_inter_overhead[3], &task[3], sizeof(task[3]));
	memcpy(&src_inter_overhead[4], &task[4], sizeof(task[4]));
	memcpy(&src_inter_AlphaBound[0], &task[0], sizeof(task[0])); 
	memcpy(&src_inter_AlphaBound[1], &task[1], sizeof(task[1])); 
	memcpy(&src_inter_AlphaBound[2], &task[2], sizeof(task[2]));
	memcpy(&src_inter_AlphaBound[3], &task[3], sizeof(task[3]));
	memcpy(&src_inter_AlphaBound[4], &task[4], sizeof(task[4]));
	memcpy(&src_inter_AvgResp[0], &task[0], sizeof(task[0])); 
	memcpy(&src_inter_AvgResp[1], &task[1], sizeof(task[1])); 
	memcpy(&src_inter_AvgResp[2], &task[2], sizeof(task[2]));
	memcpy(&src_inter_AvgResp[3], &task[3], sizeof(task[3]));
	memcpy(&src_inter_AvgResp[4], &task[4], sizeof(task[4]));
	/*if(tasks_num != src_inter.size()) {
		cout << "In the setting of Non-DVFS Inter-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}
	if(tasks_num != src_inter_NonDVFS.size()) {
		cout << "In the setting of DVFS Inter-task, the initial settings does not match Task-set pattern from input file" << endl;
		exit(1);	
	}*/
	cout << "\tFinished configuring Inter tasks" << endl;
//=======================================================================================================================================================//
// Settings of Intra- and Inter-task communication Bus and Task Management
	cout << "Preset 5) Starting to bind each Intra task(Control Flow Information) to their corresponding Inter task" << endl;
	inter_intra_bus_NonDVFS = new Task_State_Bus(time_management_NonDVFS, src_inter_NonDVFS, src_intra_NonDVFS);
	inter_intra_bus = new Task_State_Bus(time_management, src_inter, src_intra);
	inter_intra_bus_overhead = new Task_State_Bus(time_management_overhead, src_inter_overhead, src_intra_overhead);
	inter_intra_bus_AlphaBound = new Task_State_Bus(time_management_AlphaBound, src_inter_AlphaBound, src_intra_AlphaBound);
	inter_intra_bus_AvgResp = new Task_State_Bus(time_management_AvgResp, src_inter_AvgResp, src_intra_AvgResp);
	for(int i = 0; i < tasks_num; i++) {
		inter_intra_bus_NonDVFS    -> intra_tasks[i].dvfs_config((char) NonDVFS_sim);
		inter_intra_bus            -> intra_tasks[i].dvfs_config((char) DVFS_sim);
		inter_intra_bus_overhead   -> intra_tasks[i].dvfs_config((char) DVFSOverhead_sim);
		inter_intra_bus_AlphaBound -> intra_tasks[i].dvfs_config((char) DVFSAlphaBound_sim);
		inter_intra_bus_AvgResp    -> intra_tasks[i].dvfs_config((char) DVFSAvgResp_sim);
	}
	cout << "\tFinished binding Intra tasks with Inter tasks" << endl;
//=======================================================================================================================================================//
// Settings of Task Scheduler
	cout << "Preset 6) Starting to configure Task Scheduler" << endl;
	task_sched_NonDVFS = new Task_Scheduler(time_management_NonDVFS, src_inter_NonDVFS, &que_NonDVFS, (char) RM, inter_intra_bus_NonDVFS);
	task_sched = new Task_Scheduler(time_management, src_inter, &que, (char) RM, inter_intra_bus);
	task_sched_overhead = new Task_Scheduler(time_management_overhead, src_inter_overhead, &que_overhead, (char) RM, inter_intra_bus_overhead);
	task_sched_AlphaBound = new Task_Scheduler(time_management_AlphaBound, src_inter_AlphaBound, &que_AlphaBound, (char) RM, inter_intra_bus_AlphaBound);
	task_sched_AvgResp = new Task_Scheduler(time_management_AvgResp, src_inter_AvgResp, &que_AvgResp, (char) RM, inter_intra_bus_AvgResp);
	for(int i = 0; i < tasks_num; i++) {
		printf("Intra-Task_%d.WCRT: %.05f ns, Intra-Task_%d.BCRT: %.05f ns\r\n", i, inter_intra_bus_NonDVFS -> intra_tasks[i].wcrt, i, inter_intra_bus_NonDVFS -> intra_tasks[i].bcrt);
		printf("Inter-Task_%d.WCRT: %.05f ns, Inter-Task_%d.BCRT: %.05f ns\r\n\r\n", i, inter_intra_bus_NonDVFS -> inter_tasks[i].wcrt, i, inter_intra_bus_NonDVFS -> inter_tasks[i].bcrt);
	}
	cout << "\tFinished configuration of Task Scheduler" << endl;
//=======================================================================================================================================================//
// Setting the Jitter constraints
	cout << "Preset 7) Starting to set jitter contraints" << endl;
	for(int i = 0; i < tasks_num; i++) {
		inter_intra_bus_NonDVFS    -> intra_tasks[i].SysMode_reconfig(Tsk_SysMode_NonDVFS_sim[i]); 	
		inter_intra_bus            -> intra_tasks[i].SysMode_reconfig(Tsk_SysMode_DVFS_sim[i]); 	
		inter_intra_bus_overhead   -> intra_tasks[i].SysMode_reconfig(Tsk_SysMode_overhead_sim[i]); 	
		inter_intra_bus_AlphaBound -> intra_tasks[i].SysMode_reconfig(Tsk_SysMode_AlphaBound_sim[i]); 	
		inter_intra_bus_AvgResp    -> intra_tasks[i].SysMode_reconfig(Tsk_SysMode_AvgResp_sim[i]); 	
	}
	cout << "Finished setting jitter constraints" << endl;

	//for(unsigned int i = 0; i < tasks_num; i++)
	//		printf("WCRT(%d): %.05f ns, BCRT(%d): %.05f ns\r\n", i, inter_intra_bus -> intra_tasks[i].wcrt, i, inter_intra_bus -> intra_tasks[i].bcrt);
//=======================================================================================================================================================//
#ifndef PATTERN_GEN
	// NonDVFS
	//SimSchedule((char) NonDVFS_sim); cout << "Fininshed simulation of Non-DVFS environment" << endl;
	
	// StaticDVFS with user-specified target response times
	//SimSchedule((char) DVFS_sim); cout << "Fininshed simulation of DVFS environment" << endl;
	
	// ProfileDVFS with profile-based target response times
	SimSchedule((char) DVFSOverhead_sim); cout << "Fininshed simulation of DVFS environment with consideration of Transition Overhead" << endl;
	
	// Reserved setting 1
	//SimSchedule((char) DVFSAlphaBound_sim); cout << "Fininshed simulation of DVFS environment with bounded Alpha" << endl;

	// Reserved setting 2
	//SimSchedule((char) DVFSAvgResp_sim); cout << "Fininshed simulation of DVFS environment with average alpha" << endl;
#endif
	delete time_management_NonDVFS;
	delete time_management;
	delete time_management_overhead;
	delete time_management_AlphaBound;
	delete time_management_AvgResp;
	return 0;
}

void func_gen(void *inout)
{
        ExePath_case &out = *(ExePath_case*) inout;

        for(int i = 0; i < 10; i++)
        out.push_back(i);
}
void system_init(void)
{
	in_default_speed = TskSet_default_speed[TSK_SET_SIM];
        srand((unsigned) time(0));
        //energy_ref = 0.0;

//--------------------------------------------------------------------------------//        
// Configure the System Tick(Timer) for both Non-DVFS and DVFS environments
	Sys_Clk_NonDVFS.cur_freq = 0.0; // Initially none of task is running
        Sys_Clk_NonDVFS.cur_time  = 0.0;
        Sys_Clk_NonDVFS.time_unit = (int) NS;
        time_management_NonDVFS = new Time_Management(Sys_Clk_NonDVFS);
        
	Sys_Clk.cur_freq = 0.0; // Initially none of task is running
        Sys_Clk.cur_time  = 0.0;
        Sys_Clk.time_unit = (int) NS;
        time_management = new Time_Management(Sys_Clk);

	Sys_Clk_overhead.cur_freq = 0.0; // Initially none of task is running
        Sys_Clk_overhead.cur_time  = 0.0;
        Sys_Clk_overhead.time_unit = (int) NS;
        time_management_overhead = new Time_Management(Sys_Clk_overhead);

	Sys_Clk_AlphaBound.cur_freq = 0.0; // Initially none of task is running
        Sys_Clk_AlphaBound.cur_time  = 0.0;
        Sys_Clk_AlphaBound.time_unit = (int) NS;
        time_management_AlphaBound = new Time_Management(Sys_Clk_AlphaBound);
	
	Sys_Clk_AvgResp.cur_freq = 0.0; // Initially none of task is running
        Sys_Clk_AvgResp.cur_time  = 0.0;
        Sys_Clk_AvgResp.time_unit = (int) NS;
        time_management_AvgResp = new Time_Management(Sys_Clk_AvgResp);
	
	// Initially, assigning every task's Executed Time annotation as value of "0"
	// Since none of task is executed at very beginning
	time_management_NonDVFS    -> ExecutedTime = new double[tasks_num];
	time_management            -> ExecutedTime = new double[tasks_num];
	time_management_overhead   -> ExecutedTime = new double[tasks_num];
	time_management_AlphaBound -> ExecutedTime = new double[tasks_num];
	time_management_AvgResp    -> ExecutedTime = new double[tasks_num];
	for(int i = 0; i < tasks_num; i++) {
		time_management_NonDVFS    -> ExecutedTime[i] = (double) 0.0;
		time_management            -> ExecutedTime[i] = (double) 0.0;
		time_management_overhead   -> ExecutedTime[i] = (double) 0.0;
		time_management_AlphaBound -> ExecutedTime[i] = (double) 0.0;
		time_management_AvgResp    -> ExecutedTime[i] = (double) 0.0;
	}
	time_management_NonDVFS    -> UpdatePoint = 0; 
	time_management            -> UpdatePoint = 0;
	time_management_overhead   -> UpdatePoint = 0; 
	time_management_AlphaBound -> UpdatePoint = 0; 
	time_management_AvgResp    -> UpdatePoint = 0; 
//--------------------------------------------------------------------------------//        
        checkpoint_config((char*) Checkpoint_FileName);
        wcet_info_config();
/*
// Pre-create the .CSV file for each Task's experimental result
	writer = new CSVWriter*[tasks_num];
	for(int i = 0; i < tasks_num; i++) {
		writer[i] = new CSVWriter[3];
	  	char in_msg[30], in_msg1[30], in_msg2[30];
		sprintf(in_msg, "NonDVFS.%d.csv", i);
	  	sprintf(in_msg1, "DVFS.NonOverhead.%d.csv", i);
	  	sprintf(in_msg2, "DVFS.Overhead.%d.csv", i);
		writer[i][0].ExportFile_config(in_msg);
		writer[i][1].ExportFile_config(in_msg1);
		writer[i][2].ExportFile_config(in_msg2);
	}
*/
}

void checkpoint_config(char *FileName) {
        cycle_trace      = new RWCEC_Trace_in[tasks_num];
        checkpoint_num_t = new checkpoint_num[tasks_num];
        checkpointLabel  = new checkpoints_label[tasks_num];
        parsing.checkpoint_in(
			FileName,			       // The name of checkpoint-configuration file 
                        tasks_num,                             // The number of tasks 
                        (RWCEC_Trace_in*) cycle_trace,         // The cycle tracing information for building Mining Table
                        (checkpoint_num*) checkpoint_num_t,    // The number of eacc type heckpoints 
                        (checkpoints_label*) checkpointLabel   // The label of checkpoints at each task's Basic Block(s)
        );
/*
        for(int i = 0; i < tasks_num; i++) {
                for(int j = 0; j < checkpointLabel[i].B_checkpoints.size(); j++) 
                        cout << "B_checkpoint[" << j << "]: " << checkpointLabel[i].B_checkpoints[j] << endl;
                for(int k = 0; k < checkpointLabel[i].L_checkpoints.size(); k++)  
                        for(int m = 0; m < checkpointLabel[i].L_checkpoints[k].size(); m++) 
                                cout << "L_checkpoint[" << k << "][" << m << "]: " << checkpointLabel[i].L_checkpoints[k][m] << endl;
                for(int l = 0; l < checkpointLabel[i].P_checkpoints.size(); l++) 
                        cout << "P_checkpoint[" << l << "]: " << checkpointLabel[i].P_checkpoints[l] << endl;
                for(int n = 0; n < checkpointLabel[i].L_loop_bound.size(); n++)
                        cout << "L_bound[" << n << "]: " << checkpointLabel[i].L_loop_bound[n] << endl; 
                for(int n = 0; n < checkpointLabel[i].P_loop_bound.size(); n++)
                        cout << "P_bound[" << n << "]: " << checkpointLabel[i].P_loop_bound[n] << endl; 
                cout << endl << endl;
        }
*/
}

void wcet_info_config() {
        task_wcet_info = new exeTime_info[tasks_num];
        memcpy(task_wcet_info, task_wcet_info_t, tasks_num * 3 * sizeof(int));
}

void TestPattern_config()
{
	exe_path = new ExePath_set[tasks_num];
        for(unsigned int i = 0; i < tasks_num; i++) {
         src_intra_NonDVFS[i].P_loop_LaIteration    = new int*[checkpointLabel[i].P_loop_bound.size()];
         src_intra[i].P_loop_LaIteration            = new int*[checkpointLabel[i].P_loop_bound.size()];
         src_intra_overhead[i].P_loop_LaIteration   = new int*[checkpointLabel[i].P_loop_bound.size()];
         src_intra_AlphaBound[i].P_loop_LaIteration = new int*[checkpointLabel[i].P_loop_bound.size()];
         src_intra_AvgResp[i].P_loop_LaIteration    = new int*[checkpointLabel[i].P_loop_bound.size()];
         for(unsigned int j = 0; j < checkpointLabel[i].P_loop_bound.size(); j++) {
          src_intra_NonDVFS[i].P_loop_LaIteration[j]    = new int[patterns_num];
          src_intra[i].P_loop_LaIteration[j]            = new int[patterns_num];
          src_intra_overhead[i].P_loop_LaIteration[j]   = new int[patterns_num];
          src_intra_AlphaBound[i].P_loop_LaIteration[j] = new int[patterns_num];
          src_intra_AvgResp[i].P_loop_LaIteration[j]    = new int[patterns_num];
	 }
#ifdef PATTERN_GEN // To generate set of test patterns only
         rand_ExePath_gen (
                 src_intra[i].CFG_path,         // Pass each task's corrsponding Src_CFG
                 patterns_num,                  // The number of test patterns demanded to be generated
                 checkpointLabel[i],            // The label of checkpoints' corresponding Basic Block ID
                 (ExePath_set*) (&exe_path[i]), // The output of generated set of test patterns
                 (int**) src_intra_NonDVFS[i].P_loop_LaIteration,
                 (int**) src_intra[i].P_loop_LaIteration,
                 (int**) src_intra_overhead[i].P_loop_LaIteration,
                 (int**) src_intra_AlphaBound[i].P_loop_LaIteration,
                 (int**) src_intra_AvgResp[i].P_loop_LaIteration
         );
	export_TestPattern(i);
	export_TestPattern_LaIteration(i);

	 vector<int> temp;
         for(unsigned int j = 0; j < checkpointLabel[i].P_loop_bound.size(); j++) {
           for(unsigned int k = 0; k < patterns_num; k++) {
	    temp.push_back(src_intra[i].P_loop_LaIteration[j][k] + 1);
	   } 
	}
	verify_TestPattern_Consistency();

#else  // Instead of generating set of test patterns, reading the prior generated set of test patterns
       // Read the generated Set of Test Patterns and configure it/them
	vector<int> temp;
	char msg_pattern1[300];
	sprintf(msg_pattern1, "../TestPattern/TestPattern_Tsk%d.txt", i);
	parsing.TestPattern_in(msg_pattern1, (int) patterns_num, (ExePath_set*) (&exe_path[i]));//&TestPatternSet_inout);
	//verify_ParsedTestPattern(i);
	TestPatternSet_inout.clear();

	char msg_pattern2[300]; 
	sprintf(msg_pattern2, "../TestPattern/LaIterationPattern_Tsk%d.txt", i);
	LaIterationPatternSet_inout =  new int* [patterns_num];
	for(int pattern_cnt = 0; pattern_cnt < patterns_num; pattern_cnt++) 
		LaIterationPatternSet_inout[pattern_cnt] = new int [ checkpointLabel[i].P_checkpoints.size() ]; // Allocate the size of LaIteration annotations in light of the value of PChNumSet[i]
	parsing.LaIterationPattern_in(msg_pattern2, (int) patterns_num, LaIterationPatternSet_inout, checkpointLabel[i]);
	//verify_ParsedLaIterationPattern(i);
         for(unsigned int j = 0; j < checkpointLabel[i].P_loop_bound.size(); j++) {
           for(unsigned int k = 0; k < patterns_num; k++) {
	    temp.push_back(LaIterationPatternSet_inout[k][j] + 1);
	   } 
	  }
#endif
	 src_intra_NonDVFS[i].L_loop_iteration_preload.push_back(temp); 
	 src_intra[i].L_loop_iteration_preload.push_back(temp); 
	 src_intra_overhead[i].L_loop_iteration_preload.push_back(temp); 
	 src_intra_AlphaBound[i].L_loop_iteration_preload.push_back(temp); 
	 src_intra_AvgResp[i].L_loop_iteration_preload.push_back(temp); 
	 vector<int>().swap(temp); 
         src_intra_NonDVFS[i].pattern_init(exe_path[i]); // Configure a test pattern to NonDVFS environment 
	 src_intra[i].pattern_init(exe_path[i]); // Configure a test pattern to DVFS environment
         src_intra_overhead[i].pattern_init(exe_path[i]); // Configure a test pattern to NonDVFS environment 
         src_intra_AlphaBound[i].pattern_init(exe_path[i]); // Configure a test pattern to NonDVFS environment 
         src_intra_AvgResp[i].pattern_init(exe_path[i]); // Configure a test pattern to NonDVFS environment 

	 // Fill dummy values for getting memory space
	  for(unsigned int addr = 0;  addr < src_intra[i].L_loop_iteration_preload.size(); addr++) {
         	src_intra_NonDVFS[i].L_loop_iteration.push_back(0); 
	 	src_intra[i].L_loop_iteration.push_back(0);
         	src_intra_overhead[i].L_loop_iteration.push_back(0);
         	src_intra_AlphaBound[i].L_loop_iteration.push_back(0);
         	src_intra_AvgResp[i].L_loop_iteration.push_back(0);
	  }
        }

#ifdef PATTERN_GEN
for(unsigned int i = 0; i < tasks_num; i++) {
       // Instead of generating set of test patterns, reading the prior generated set of test patterns
       // Read the generated Set of Test Patterns and configure it/them
	vector<int> temp;
	char msg_pattern1[300];
	sprintf(msg_pattern1, "../TestPattern/TestPattern_Tsk%d.txt", i);
	parsing.TestPattern_in(msg_pattern1, (int) patterns_num, (ExePath_set*) &TestPatternSet_inout);
	verify_ParsedTestPattern(i);
	TestPatternSet_inout.clear();

	char msg_pattern2[300]; 
	sprintf(msg_pattern2, "../TestPattern/LaIterationPattern_Tsk%d.txt", i);
	LaIterationPatternSet_inout =  new int* [patterns_num];
	for(int pattern_cnt = 0; pattern_cnt < patterns_num; pattern_cnt++) 
		LaIterationPatternSet_inout[pattern_cnt] = new int [ checkpointLabel[i].P_checkpoints.size() ]; // Allocate the size of LaIteration annotations in light of the value of PChNumSet[i]
	parsing.LaIterationPattern_in(msg_pattern2, (int) patterns_num, LaIterationPatternSet_inout, checkpointLabel[i]);
	verify_ParsedLaIterationPattern(i);
         for(unsigned int j = 0; j < checkpointLabel[i].P_loop_bound.size(); j++) {
           for(unsigned int k = 0; k < patterns_num; k++) {
	    temp.push_back(LaIterationPatternSet_inout[k][j] + 1);
	   } 
	  }
}
	cout << "Verify Done (Parsing)" << endl;
	exit(1);
#endif
}

void array_int_cpy(vector<int> &Dst, int *Src)
{
        int a = 0;
        for(a = 0; Src[a] != 0x7FFFFFFF; a++) Dst.push_back(Src[a]);
}

void export_result(int env)
{
	double csv_col_temp[3]; // RFJ, AFJ, Energy
	for(int i = 0; i < tasks_num; i++) {
	  char in_msg[50];
	  if(env == (int) NonDVFS_sim) {
		sprintf(in_msg, "NonDVFS.%d", i);
	  	inter_intra_bus_NonDVFS -> intra_tasks[i].output_result(in_msg);
		/*csv_col_temp[0] = inter_intra_bus_NonDVFS -> intra_tasks[i].RFJ;
		csv_col_temp[1] = inter_intra_bus_NonDVFS -> intra_tasks[i].AFJ;
		csv_col_temp[2] = inter_intra_bus_NonDVFS -> intra_tasks[i].energy_acc;
		writer[i][0].addDatainRow(csv_col_temp, csv_col_temp + sizeof(csv_col_temp) / sizeof(double));
	  	*/
	  }
	  else if(env == (int) DVFS_sim) {
	  	sprintf(in_msg, "DVFS.NonOverhead.%d", i);
	  	inter_intra_bus -> intra_tasks[i].output_result(in_msg);
		/*
		csv_col_temp[0] = inter_intra_bus_NonDVFS -> intra_tasks[i].RFJ;
		csv_col_temp[1] = inter_intra_bus_NonDVFS -> intra_tasks[i].AFJ;
		csv_col_temp[2] = inter_intra_bus_NonDVFS -> intra_tasks[i].energy_acc;
		writer[i][1].addDatainRow(csv_col_temp, csv_col_temp + sizeof(csv_col_temp) / sizeof(double));
		*/
	  }
	  else if(env == (int) DVFSOverhead_sim) {
	  	sprintf(in_msg, "DVFS.Overhead.%d", i);
	  	inter_intra_bus_overhead -> intra_tasks[i].output_result(in_msg);
		/*
		csv_col_temp[0] = inter_intra_bus_NonDVFS -> intra_tasks[i].RFJ;
		csv_col_temp[1] = inter_intra_bus_NonDVFS -> intra_tasks[i].AFJ;
		csv_col_temp[2] = inter_intra_bus_NonDVFS -> intra_tasks[i].energy_acc;
		writer[i][2].addDatainRow(csv_col_temp, csv_col_temp + sizeof(csv_col_temp) / sizeof(double));
		*/
	  }
	  else if(env == (int) DVFSAlphaBound_sim) {
	  	sprintf(in_msg, "DVFS.AlphaBound.%d", i);
	  	inter_intra_bus_AlphaBound -> intra_tasks[i].output_result(in_msg);
		/*
		csv_col_temp[0] = inter_intra_bus_NonDVFS -> intra_tasks[i].RFJ;
		csv_col_temp[1] = inter_intra_bus_NonDVFS -> intra_tasks[i].AFJ;
		csv_col_temp[2] = inter_intra_bus_NonDVFS -> intra_tasks[i].energy_acc;
		writer[i][2].addDatainRow(csv_col_temp, csv_col_temp + sizeof(csv_col_temp) / sizeof(double));
		*/
	  }
	  else if(env == (int) DVFSAvgResp_sim) {
	  	sprintf(in_msg, "DVFS.AvgResp.%d", i);
	  	inter_intra_bus_AvgResp -> intra_tasks[i].output_result(in_msg);
		/*
		csv_col_temp[0] = inter_intra_bus_NonDVFS -> intra_tasks[i].RFJ;
		csv_col_temp[1] = inter_intra_bus_NonDVFS -> intra_tasks[i].AFJ;
		csv_col_temp[2] = inter_intra_bus_NonDVFS -> intra_tasks[i].energy_acc;
		writer[i][2].addDatainRow(csv_col_temp, csv_col_temp + sizeof(csv_col_temp) / sizeof(double));
		*/
	  }
	  else {
		cout << "There is no environment option match the demand" << endl;
		exit(1);
	  }
	}
}

void verify_TestPattern_Consistency(void)
{	
	for(int i = 0; i < (int) tasks_num; i++) {
	// Check the consistency among all environments' set of Test Pattern
	  if(src_intra_NonDVFS[i].exe_path.size() != src_intra[i].exe_path.size()) {
	  	cout << "Wrong configuration" << endl
		     << "The number of Test Pattern between Non DVFS and DVFS environment are not consistent" << endl;
	  	exit(1);
	  }
	  else if(src_intra_NonDVFS[i].exe_path.size() != src_intra_overhead[i].exe_path.size()) {
	  	cout << "Wrong configuration" << endl
		     << "The number of Test Pattern between Non DVFS and DVFSOverhead environment are not consistent" << endl;
	  	exit(1);
	  }
	  else if(src_intra[i].exe_path.size() != src_intra_overhead[i].exe_path.size()) {
	  	cout << "Wrong configuration" << endl
		     << "The number of Test Pattern between DVFS and DVFSOverhead environment are not consistent" << endl;
	  	exit(1);
	  }
	  else if(src_intra_AlphaBound[i].exe_path.size() != src_intra_overhead[i].exe_path.size()) {
	  	cout << "Wrong configuration" << endl
		     << "The number of Test Pattern between DVFS and DVFSAlpahBound environment are not consistent" << endl;
	  	exit(1);
	  }
	  else if(src_intra_AvgResp[i].exe_path.size() != src_intra_AlphaBound[i].exe_path.size()) {
	  	cout << "Wrong configuration" << endl
		     << "The number of Test Pattern between DVFS and DVFSAvgResp environment are not consistent" << endl;
	  	exit(1);
	  }
	  else {
		  for(int j = 0; j < src_intra[i].exe_path.size(); j++) {
	 	    for(int k = 0; k < src_intra[i].exe_path[j].size(); k++) 
			if(src_intra_NonDVFS[i].exe_path[j][k] != src_intra[i].exe_path[j][k]) {
			  cout << "Wrong configuration" << endl
			       << "The #" << j << "'s " << k << "th BlockID of Task_" << i << "in both NonDVFS environment and DVFS environment are not consistent" << endl;
			  exit(1);
			}
			else if(src_intra_NonDVFS[i].exe_path[j][k] != src_intra_overhead[i].exe_path[j][k]) {
			  cout << "Wrong configuration" << endl
			       << "The #" << j << "'s " << k << "th BlockID of Task_" << i << "in both NonDVFS environment and DVFSOverhead environment are not consistent" << endl;
			  exit(1);
			}
			else if(src_intra[i].exe_path[j][k] != src_intra_overhead[i].exe_path[j][k]) {
			  cout << "Wrong configuration" << endl
			       << "The #" << j << "'s " << k << "th BlockID of Task_" << i << "in both DVFS environment and DVFSOverhead environment are not consistent" << endl;
			  exit(1);
			}
			else if(src_intra[i].exe_path[j][k] != src_intra_AlphaBound[i].exe_path[j][k]) {
			  cout << "Wrong configuration" << endl
			       << "The #" << j << "'s " << k << "th BlockID of Task_" << i << "in both DVFS environment and DVFSAlphaBound environment are not consistent" << endl;
			  exit(1);
			}
			else if(src_intra_AvgResp[i].exe_path[j][k] != src_intra_AlphaBound[i].exe_path[j][k]) {
			  cout << "Wrong configuration" << endl
			       << "The #" << j << "'s " << k << "th BlockID of Task_" << i << "in both DVFS environment and DVFSAvgResp environment are not consistent" << endl;
			  exit(1);
			}
		  }
	  }

	// Check if Wrong Test Pattern had been generated 
	  for(unsigned int cnt = 0; cnt < src_intra_NonDVFS[i].exe_path.size(); cnt++) {
	    for(unsigned int cnt2 = 0; cnt2 < src_intra_NonDVFS[i].exe_path[cnt].size(); cnt2++) {
		if(
			src_intra_NonDVFS[i].exe_path[cnt][cnt2] > src_intra_NonDVFS[i].CFG_path.back().get_index() ||
			src_intra_NonDVFS[i].exe_path[cnt][cnt2] < src_intra_NonDVFS[i].CFG_path.front().get_index() 
		 ) {
			cout << "In NonDVFS Env." << endl;
		 	printf("Tsk%d's ExePath_set[%d][%d]: %d, such Index of Basic Block doesn't exist in this Task's CFG\r\n", i, cnt, cnt2, src_intra_NonDVFS[i].exe_path[cnt][cnt2]);
			exit(1);
		 }
	    }
	  }
	  for(unsigned int cnt = 0; cnt < src_intra[i].exe_path.size(); cnt++) {
	    for(unsigned int cnt2 = 0; cnt2 < src_intra[i].exe_path[cnt].size(); cnt2++) {
		if(
			src_intra[i].exe_path[cnt][cnt2] > src_intra[i].CFG_path.back().get_index() ||
			src_intra[i].exe_path[cnt][cnt2] < src_intra[i].CFG_path.front().get_index() 
		 ) {
			cout << "In DVFS Env." << endl;
		 	printf("Tsk%d's ExePath_set[%d][%d]: %d, such Index of Basic Block doesn't exist in this Task's CFG\r\n", i, cnt, cnt2, src_intra[i].exe_path[cnt][cnt2]);
			exit(1);
		 }
	    }
	  }
	  for(unsigned int cnt = 0; cnt < src_intra_overhead[i].exe_path.size(); cnt++) {
	    for(unsigned int cnt2 = 0; cnt2 < src_intra_overhead[i].exe_path[cnt].size(); cnt2++) {
		if(
			src_intra_overhead[i].exe_path[cnt][cnt2] > src_intra_overhead[i].CFG_path.back().get_index() ||
			src_intra_overhead[i].exe_path[cnt][cnt2] < src_intra_overhead[i].CFG_path.front().get_index() 
		 ) {
			cout << "In DVFSOverhead Env." << endl;
		 	printf("Tsk%d's ExePath_set[%d][%d]: %d, such Index of Basic Block doesn't exist in this Task's CFG\r\n", i, cnt, cnt2, src_intra_overhead[i].exe_path[cnt][cnt2]);
			exit(1);
		 }
	    }
	  }
	  for(unsigned int cnt = 0; cnt < src_intra_AlphaBound[i].exe_path.size(); cnt++) {
	    for(unsigned int cnt2 = 0; cnt2 < src_intra_AlphaBound[i].exe_path[cnt].size(); cnt2++) {
		if(
			src_intra_AlphaBound[i].exe_path[cnt][cnt2] > src_intra_AlphaBound[i].CFG_path.back().get_index() ||
			src_intra_AlphaBound[i].exe_path[cnt][cnt2] < src_intra_AlphaBound[i].CFG_path.front().get_index() 
		 ) {
			cout << "In DVFSAlphaBound Env." << endl;
		 	printf("Tsk%d's ExePath_set[%d][%d]: %d, such Index of Basic Block doesn't exist in this Task's CFG\r\n", i, cnt, cnt2, src_intra_AlphaBound[i].exe_path[cnt][cnt2]);
			exit(1);
		 }
	    }
	  }
	  for(unsigned int cnt = 0; cnt < src_intra_AvgResp[i].exe_path.size(); cnt++) {
	    for(unsigned int cnt2 = 0; cnt2 < src_intra_AvgResp[i].exe_path[cnt].size(); cnt2++) {
		if(
			src_intra_AvgResp[i].exe_path[cnt][cnt2] > src_intra_AvgResp[i].CFG_path.back().get_index() ||
			src_intra_AvgResp[i].exe_path[cnt][cnt2] < src_intra_AvgResp[i].CFG_path.front().get_index() 
		 ) {
			cout << "In DVFSAvgResp Env." << endl;
		 	printf("Tsk%d's ExePath_set[%d][%d]: %d, such Index of Basic Block doesn't exist in this Task's CFG\r\n", i, cnt, cnt2, src_intra_AvgResp[i].exe_path[cnt][cnt2]);
			exit(1);
		 }
	    }
	  }
	
	}
	cout << "Verify Done!" << endl;
}

void verify_ParsedTestPattern(int tsk_id)
{
// Verify that the Set of Test Pattern parsed from prior generated Set of Test Pattern is consistent
	if(!(src_intra[tsk_id].exe_path.size() == TestPatternSet_inout.size() && src_intra[tsk_id].exe_path.size() == (unsigned int) patterns_num)) {
		cout << "The size of generated TestPatternSet is not equal to the size of parsed TestPatternSet" << endl;
		cout << "src_intra[0].exe_path.size(): " << src_intra[tsk_id].exe_path.size() << " "
		     << "TestPatternSet_inout.size(): " << TestPatternSet_inout.size() << endl;
		exit(1);
	}
	for(unsigned int i = 0; i < (unsigned int) patterns_num; i++) {
		for(unsigned int j = 0; j < (unsigned int) TestPatternSet_inout[i].size(); j++) {
			if(TestPatternSet_inout[i][j] != src_intra[tsk_id].exe_path[i][j]) {
				cout << "TestPatternSet_inout[" << i << "][" << j << "]: " << TestPatternSet_inout[i][j] << "!= "
				     << "src_intra[" << tsk_id << "].exe_path[" << i << "][" << j << "]: " << src_intra[tsk_id].exe_path[i][j] << endl;
				exit(1);
			}
		}
	}	 
}


void verify_ParsedLaIterationPattern(int tsk_id)
{
// Verify that the Set of LaIteration Pattern parsed from prior generated Set of LaIteration Pattern is consistent
	/*if(!(src_intra[tsk_id].P_loop_LaIteration.size() == LaIterationPatternSet_inout.size() && src_intra[tsk_id].P_loop_LaIteration.size() == (unsigned int) patterns_num)) {
		cout << "The size of generated LaIterationPatternSet is not equal to the size of parsed LaIterationPatternSet" << endl;
		cout << "src_intra[0].exe_path.size(): " << src_intra[tsk_id].P_loop_LaIteration.size() << " "
		     << "TestPatternSet_inout.size(): " << LaIterationPatternSet_inout.size() << endl;
		exit(1);
	}*/
	for(unsigned int i = 0; i < (unsigned int) checkpointLabel[tsk_id].P_loop_bound.size(); i++) {
		for(unsigned int j = 0; j < (unsigned int) patterns_num; j++) {
			cout << "LaIterationPatternSet_inout[" << j << "][" << i << "]: " << LaIterationPatternSet_inout[j][i] << "=> "
			     << "src_intra[" << tsk_id << "].P_loop_LaIteration[" << i << "][" << j << "]: " << src_intra[tsk_id].P_loop_LaIteration[i][j] << endl;
			if(LaIterationPatternSet_inout[j][i] != src_intra[tsk_id].P_loop_LaIteration[i][j]) {
				cout << "LaIterationPatternSet_inout[" << j << "][" << i << "]: " << LaIterationPatternSet_inout[j][i] << "!= "
				     << "src_intra[" << tsk_id << "].P_loop_LaIteration[" << i << "][" << j << "]: " << src_intra[tsk_id].P_loop_LaIteration[i][j] << endl;
				exit(1);
			}
		}
	}	 
}

void export_TestPattern(unsigned int i)
{
	char *msg_TestPattern1 = new char[500];
	unsigned int PatternSet_size = exe_path[i].size();
	cout << "Tsk[" << i << "].size of exe_path_set: " << exe_path[i].size() << endl;
	sprintf(msg_TestPattern1, "echo \"PATTERN_SET_SIZE: %d\" > ../TestPattern/TestPattern_Tsk%d.txt", PatternSet_size, i);
	system(msg_TestPattern1); //delete [] msg_TestPattern1;
	for(unsigned int j = 0; j < PatternSet_size; j++) {
	  unsigned int PatternCase_size = exe_path[i][j].size();
	  char *msg_TestPattern2 = new char[500];
	  sprintf(msg_TestPattern2, "echo \"CASE: %d\r\nSIZE: %d\" >> ../TestPattern/TestPattern_Tsk%d.txt", j, PatternCase_size, i);
	  system(msg_TestPattern2); //delete [] msg_TestPattern2;	
	  for(unsigned int k = 0; k < PatternCase_size; k++) {
	  	char *msg_TestPattern3 = new char[50];
	  	sprintf(msg_TestPattern3, "echo \"%d\" >> ../TestPattern/TestPattern_Tsk%d.txt", exe_path[i][j][k], i);
	  	system(msg_TestPattern3); //delete [] msg_TestPattern3;	
	  }
	}
	char *msg_TestPattern_End = new char[50];
	sprintf(msg_TestPattern_End, "echo \"#\" >> ../TestPattern/TestPattern_Tsk%d.txt", i);
	system(msg_TestPattern_End); //delete [] msg_TestPattern_End;	
}

void export_TestPattern_LaIteration(unsigned int i)
{
	char *msg_TestPattern1 = new char[500];
	unsigned int PatternSet_size = (unsigned int) patterns_num;
	sprintf(msg_TestPattern1, "echo \"PATTERN_SET_SIZE: %d\" > ../TestPattern/LaIterationPattern_Tsk%d.txt", PatternSet_size, i);
	system(msg_TestPattern1); //delete [] msg_TestPattern1;
	for(unsigned int j = 0; j < PatternSet_size; j++) {
	  unsigned int PatternCase_size = checkpointLabel[i].P_checkpoints.size(); // The number of LaIterations are all the same regardless which case it is
	  char *msg_TestPattern2 = new char[500];
	  sprintf(msg_TestPattern2, "echo \"CASE: %d\r\nSIZE: %d\" >> ../TestPattern/LaIterationPattern_Tsk%d.txt", j, PatternCase_size, i);
	  system(msg_TestPattern2); //delete [] msg_TestPattern2;	
	  for(unsigned int k = 0; k < PatternCase_size; k++) {
	  	char *msg_TestPattern3 = new char[50];
	  	sprintf(msg_TestPattern3, "echo \"%d\" >> ../TestPattern/LaIterationPattern_Tsk%d.txt", src_intra[i].P_loop_LaIteration[k][j], i);
	  	system(msg_TestPattern3); //delete [] msg_TestPattern3;	
	 }
	}
	char *msg_TestPattern_End = new char[50];
	sprintf(msg_TestPattern_End, "echo \"#\" >> ../TestPattern/LaIterationPattern_Tsk%d.txt", i);
	system(msg_TestPattern_End); //delete [] msg_TestPattern_End;	
}

void SimSchedule(char env)
{
	double cur_time; 
	bool isEndSim = false;
	if(env == (char) NonDVFS_sim) {
		sim_mode = (int) 1;
		cout << "==================================================" << endl;
		cout << "\t\t";
		for(int i = 0; i < tasks_num; i++) cout << "task_" << i << "\t";
		cout << "Hyperperiod: " << task_sched_NonDVFS -> hyperperiod << "(us)" << endl;
		cout << endl << "--------------------------------------------------" << endl;
		time_management_NonDVFS -> update_cur_time(0.0);
		task_sched_NonDVFS -> sched_arbitration(0.000);
		cur_time = time_management_NonDVFS -> sys_clk -> cur_time;
		//for(unsigned int i = 0; i < tasks_num; i++) 
		//	isEndSim = (inter_intra_bus_NonDVFS -> intra_tasks[i].response_case.size() >= (unsigned int) EVAL_CNT_START) ? true : false;
		//for(; isEndSim == false;) {
		//for(; time_management_NonDVFS -> sys_clk -> cur_time <= task_sched_NonDVFS -> hyperperiod/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
		for(; time_management_NonDVFS -> sys_clk -> cur_time <= 51770000*4/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
		//for(; task_sched_NonDVFS -> task_list[3].completion_cnt < 5; ) {
			task_sched_NonDVFS -> sched_arbitration(cur_time);
			for(int i = 0; i < tasks_num; i++) { 
				if(task_sched_NonDVFS -> task_list[i].state == (char) RUN) {
					cur_TskID = i;
					inter_intra_bus_NonDVFS -> time_driven_cfg(i, (int) WORST);
				}
			} 
			cur_time = time_management_NonDVFS -> sys_clk -> cur_time;
			// Two situations:
			// 1) One task just completed its work regardless of Ready state of other tasks
			// 2) No task is in the Ready Queue
			if(task_sched_NonDVFS -> IsIdle() == true && task_sched_NonDVFS -> ready_queue -> IsEmpty() == true) {
				cur_time = show_minNRT((Task_Scheduler*) task_sched_NonDVFS);
				time_management_NonDVFS -> update_cur_time(cur_time);
			}
			else cur_time += 0.1;	
			//cout << "cur_time: " << time_management_NonDVFS -> sys_clk -> cur_time << "ns" << endl;
		}
		cout << "==================================================" << endl;
		export_result(env);
		for(int i = 0; i < tasks_num; i++)
		 cout << "(NonDVFS)Task_" << i << " has already run " << task_sched_NonDVFS -> show_SimPatternCnt(i) << " numbers of execution paths(test pattern)" << endl;
	}
	else if(env == (char) DVFS_sim) {
		sim_mode = (int) 2;
		cout << "==================================================" << endl;
		cout << "\t\t";
		for(int i = 0; i < tasks_num; i++) cout << "task_" << i << "\t";
		cout << "Hyperperiod: " << task_sched -> hyperperiod << "(us)" << endl;
		cout << endl << "--------------------------------------------------" << endl;
		time_management -> update_cur_time(0.0);
		task_sched -> sched_arbitration(0.000);
		cur_time = time_management -> sys_clk -> cur_time;
		//for(unsigned int i = 0; i < tasks_num; i++) 
		//	isEndSim = (inter_intra_bus -> intra_tasks[i].response_case.size() >= (unsigned int) EVAL_CNT_START * 2) ? true : false;
		//for(; isEndSim == false;) {
		//for(; time_management -> sys_clk -> cur_time <= task_sched -> hyperperiod; ) {
		for(; time_management -> sys_clk -> cur_time <= 51770000*4/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
		//for(; task_sched -> task_list[3].completion_cnt < 5; ) {
			task_sched -> sched_arbitration(cur_time);
			for(int i = 0; i < tasks_num; i++) { 
				if(task_sched -> task_list[i].state == (char) RUN) {
					cur_TskID = i;
					inter_intra_bus -> time_driven_cfg(i, (int) WORST);
				}
			} 
			cur_time = time_management -> sys_clk -> cur_time;
			// Two situations:
			// 1) One task just completed its work regardless of Ready state of other tasks
			// 2) No task is in the Ready Queue
			if(task_sched -> IsIdle() == true && task_sched -> ready_queue -> IsEmpty() == true) {
				cur_time = show_minNRT((Task_Scheduler*) task_sched);
				time_management -> update_cur_time(cur_time);
			}
			else cur_time += 0.1;	
		}
		cout << "==================================================" << endl;
		export_result(env);
		for(int i = 0; i < tasks_num; i++)
			cout << "Task_" << i << " has already run " << task_sched -> show_SimPatternCnt(i) << " numbers of execution paths(test pattern)" << endl;
	}
	else if(env == (char) DVFSOverhead_sim) {
		sim_mode = (int) 3;
		cout << "==================================================" << endl;
		cout << "\t\t";
		for(int i = 0; i < tasks_num; i++) cout << "task_" << i << "\t";
		cout << "Hyperperiod: " << task_sched_overhead -> hyperperiod << "(us)" << endl;
		cout << endl << "--------------------------------------------------" << endl;
		time_management_overhead -> update_cur_time(0.0);
		task_sched_overhead -> sched_arbitration(0.000);
		cur_time = time_management_overhead -> sys_clk -> cur_time;
		//for(unsigned int i = 0; i < tasks_num; i++) 
		//	isEndSim = (inter_intra_bus_overhead -> intra_tasks[i].response_case.size() >= 100) ? true : false;
		//for(; isEndSim == false;) {
		//for(; time_management_overhead -> sys_clk -> cur_time <= task_sched_overhead -> hyperperiod/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
		for(; time_management_overhead -> sys_clk -> cur_time <= 51770000*4/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
		//for(; task_sched_overhead -> task_list[3].completion_cnt < 5; ) {
			task_sched_overhead -> sched_arbitration(cur_time);
			for(int i = 0; i < tasks_num; i++) { 
				if(task_sched_overhead -> task_list[i].state == (char) RUN) {
					cur_TskID = i;
					inter_intra_bus_overhead -> time_driven_cfg(i, (int) WORST);
				}
			} 
			cur_time = time_management_overhead -> sys_clk -> cur_time;
			// Two situations:
			// 1) One task just completed its work regardless of Ready state of other tasks
			// 2) No task is in the Ready Queue
			if(task_sched_overhead -> IsIdle() == true && task_sched_overhead -> ready_queue -> IsEmpty() == true) {
				cur_time = show_minNRT((Task_Scheduler*) task_sched_overhead);
				time_management_overhead -> update_cur_time(cur_time);
			}
			else cur_time += 0.1;	
			//cout << "cur_time: " << time_management_NonDVFS -> sys_clk -> cur_time << "ns" << endl;
		}
		cout << "==================================================" << endl;
		export_result(env);
		for(int i = 0; i < tasks_num; i++)
			cout << "Task_" << i << " has already run " << task_sched_overhead -> show_SimPatternCnt(i) << " numbers of execution paths(test pattern)" << endl;
	}
	else if(env == (char) DVFSAlphaBound_sim) {
		sim_mode = (int) 4;
		cout << "==================================================" << endl;
		cout << "\t\t";
		for(int i = 0; i < tasks_num; i++) cout << "task_" << i << "\t";
		cout << "Hyperperiod: " << task_sched_AlphaBound -> hyperperiod << "(us)" << endl;
		cout << endl << "--------------------------------------------------" << endl;
		time_management_AlphaBound -> update_cur_time(0.0);
		task_sched_AlphaBound -> sched_arbitration(0.000);
		cur_time = time_management_AlphaBound -> sys_clk -> cur_time;
		for(unsigned int i = 0; i < tasks_num; i++) 
			isEndSim = (inter_intra_bus_AlphaBound -> intra_tasks[i].response_case.size() >= 100) ? true : false;
		for(; isEndSim == false;) {
		//for(; time_management_AlphaBound -> sys_clk -> cur_time <= task_sched_AlphaBound -> hyperperiod/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
			task_sched_AlphaBound -> sched_arbitration(cur_time);
			for(int i = 0; i < tasks_num; i++) { 
				if(task_sched_AlphaBound -> task_list[i].state == (char) RUN) {
					cur_TskID = i;
					inter_intra_bus_AlphaBound -> time_driven_cfg(i, (int) WORST);
				}
			} 
			cur_time = time_management_AlphaBound -> sys_clk -> cur_time;
			// Two situations:
			// 1) One task just completed its work regardless of Ready state of other tasks
			// 2) No task is in the Ready Queue
			if(task_sched_AlphaBound -> IsIdle() == true && task_sched_AlphaBound -> ready_queue -> IsEmpty() == true) {
				cur_time = show_minNRT((Task_Scheduler*) task_sched_AlphaBound);
				time_management_AlphaBound -> update_cur_time(cur_time);
			}
			else cur_time += 0.1;	
			//cout << "cur_time: " << time_management_NonDVFS -> sys_clk -> cur_time << "ns" << endl;
		}
		cout << "==================================================" << endl;
		export_result(env);
		for(int i = 0; i < tasks_num; i++)
			cout << "Task_" << i << " has already run " << task_sched_AlphaBound -> show_SimPatternCnt(i) << " numbers of execution paths(test pattern)" << endl;
	}
	else if(env == (char) DVFSAvgResp_sim) {
		sim_mode = (int) 5;
		cout << "==================================================" << endl;
		cout << "\t\t";
		for(int i = 0; i < tasks_num; i++) cout << "task_" << i << "\t";
		cout << "Hyperperiod: " << task_sched_AvgResp -> hyperperiod << "(us)" << endl;
		cout << endl << "--------------------------------------------------" << endl;
		time_management_AvgResp -> update_cur_time(0.0);
		task_sched_AvgResp -> sched_arbitration(0.000);
		cur_time = time_management_AvgResp -> sys_clk -> cur_time;
		for(unsigned int i = 0; i < tasks_num; i++) 
			isEndSim = (inter_intra_bus_AvgResp -> intra_tasks[i].response_case.size() >= 100) ? true : false;
		for(; isEndSim == false;) {
		//for(; time_management_AvgResp -> sys_clk -> cur_time <= task_sched_AvgResp -> hyperperiod/*task_sched_NonDVFS -> task_list[2].completion_cnt < 20*/; ) {
			task_sched_AvgResp -> sched_arbitration(cur_time);
			for(int i = 0; i < tasks_num; i++) { 
				if(task_sched_AvgResp -> task_list[i].state == (char) RUN) {
					cur_TskID = i;
					inter_intra_bus_AvgResp -> time_driven_cfg(i, (int) WORST);
				}
			} 
			cur_time = time_management_AvgResp -> sys_clk -> cur_time;
			// Two situations:
			// 1) One task just completed its work regardless of Ready state of other tasks
			// 2) No task is in the Ready Queue
			if(task_sched_AvgResp -> IsIdle() == true && task_sched_AvgResp -> ready_queue -> IsEmpty() == true) {
				cur_time = show_minNRT((Task_Scheduler*) task_sched_AvgResp);
				time_management_AvgResp -> update_cur_time(cur_time);
			}
			else cur_time += 0.1;	
			//cout << "cur_time: " << time_management_NonDVFS -> sys_clk -> cur_time << "ns" << endl;
		}
		cout << "==================================================" << endl;
		export_result(env);
		for(int i = 0; i < tasks_num; i++)
			cout << "Task_" << i << " has already run " << task_sched_AvgResp -> show_SimPatternCnt(i) << " numbers of execution paths(test pattern)" << endl;
	}
	else {
		printf("Demand: %d\r\n", env);
		cout << "There is no environment option match the demand" << endl;
		exit(1);
	}
}

double show_minNRT(void *TskSched_in)
{
	Task_Scheduler *TskSched = (Task_Scheduler*) TskSched_in;
	double min_t;
	min_t = TskSched -> hyperperiod;
	for(unsigned int i = 0; i < tasks_num; i++) {
		if(TskSched -> task_list[i].release_time < min_t)
			min_t = TskSched -> task_list[i].release_time;
	}
	return min_t;
}

/*
void priority_assignment(void)
{

double PrtSet_TskSet1[TSK_SET_1_COM_NUM][TSK_SET_1_TSK_NUM];
double PeriodSet_TskSet1[2][5] = {
double PrtSet_TskSet1[2][5];
	unsigned int prt = 0;
	// Initialise all parameters 
	for(unsigned int i = 0; i < (unsigned int) TSK_SET_1_COM_NUM; i++) {
	  for(unsigned int j = 0; j < (unsigned int) TSK_SET_1_TSK_NUM; j++) {
  	  	PrtSet_TskSet1[i][j] = (double) 0.0;
	  }
	}

	// Assign Priority according to Rate-Monotonic scheduling policy
	for(unsigned int i = 0; i < (unsigned int) TSK_SET_1_COM_NUM; i++) {
	  for(unsigned int j = 0; j < (unsigned int) TSK_SET_1_TSK_NUM; j++) {
  	  	
	  }
	}
}
*/
