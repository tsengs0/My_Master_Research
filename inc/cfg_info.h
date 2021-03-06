#ifndef __CFG_INFO_H
#define __CFG_INFO_H

#include <iostream>
#include <vector>
#include <cmath>
#include "jitter_info.h"
#include "main.h"
#include "timer.h"

using namespace std;

typedef vector<char> char_str;
typedef vector<int> array_int_element;

class Basic_block {
	private:
		int block_index; 
		int execution_cycles[3]; // index_0 : WCEC, index_1 : ACEC, index_3 : BCEC

	public:
		vector<int> succ;
		//unsigned char loop_exit;
		int get_index(void);
		int get_cycles(int case_t);
		int get_succ(int succ_index);
		int B_checkpoint_en; // For normally branch instruction
		int L_checkpoint_en[2]; // For loop nest, first element means the index of loop nest; 
					// Second element means the address of minining table
		Basic_block(int curr_index, vector<int> &succ_index, vector<int> &cycles);
		~Basic_block(void);
};	

// The tuple of mining tablele
typedef struct B_Mining_table {
		int n_taken_rwcec; // The remaining worst-case executin cycles from current checkpoint, if the branch instruction was not taken
		int taken_rwcec; // The remaining worst-case executin cycles from current checkpoint, if the branch instruction was taken
		int successors[2];
} B_mining_table_t;

typedef struct L_Mining_table {
	vector<int> n_taken_rwcec;
	vector<int> taken_rwcec;
	int successors[2];
} L_mining_table_t;

typedef vector< vector<int> > L_checkpoints_t;
typedef vector<int> B_checkpoints_t;
typedef vector<L_checkpoints_t> P_checkpoints_t;
typedef vector<int> L_loop_iteration_t;
typedef vector< vector<int> > P_loop_iteration_t;
// The set of various types of Checkpoints
typedef struct Checkpoints {
	B_checkpoints_t B_checkpoints;
	L_checkpoints_t L_checkpoints;
	P_checkpoints_t P_checkpoints;
	L_loop_iteration_t L_loop_iteration; // Recording the number of iteration every loop nest have run so far
	P_loop_iteration_t P_loop_iteration;
} checkpoints_t;

typedef struct RWCEC_Trace {
	int B_RWCEC_t[1][4];
	int L_RWCEC_t[1][2][8];
} RWCEC_Trace_in;

typedef struct isr_context{
	float act_exe_time;
	int act_cycles;
} isr_context_t;

class Src_CFG {
	private:
		vector<char_str> task_id;
		char_str task_id_temp;
		vector<char_str> wcec, acec, bcec;
		char_str wcec_temp, acec_temp, bcec_temp;

		vector<array_int_element> task_succ;
		vector<int> succ;
		int succ_int_temp;
	
	public:
		vector<Basic_block> CFG_path;
		int execution_cycles[3]; // index_0 : WCEC, index_1 : ACEC, index_3 : BCEC
		float max_freq_t;
		float min_freq_t;
		float default_freq_t;
		char dvfs_en;

// Multitask scheduling information
		float response_time;
		float max_response;  // For evaluating the finish time jitter
		float min_response;  // For evaluating the finish time jitter
		double response_acc; // For evaluating the finish time jitter 
		double exe_acc;      // For evaluating the finish time jitter 	
		Src_CFG *next_task;
		Src_CFG *pre_task;
		vector< vector<int> > exe_path;
		
		int cycle_acc;
		vector<float> exe_case;
		vector<float> response_case;
		float response_SampleVariance;
		float RFJ, AFJ; // Relative finishing time jitter; Absolute finishing time jitter
		float exe_var;  // Recording variation on actual execution time
		float tar_diff; // The difference between actual execution time and target execution time
		jitter_constraint jitter_config;
		vector<B_mining_table_t> B_mining_table;
		vector< vector<L_mining_table_t> > L_mining_table;

	//public:
		void traverse_spec_path(int &case_id, int case_t, float releast_time_new, float start_time_new, float Deadline, char DVFS_en);
		float get_cur_speed(void);

		// Intra-task DVFS attributes
		void checkpoints_placement(checkpoints_t &checkpoints_temp);
		void mining_table_gen(void);
		void exe_cycle_tracing(int *WCET_INFO, RWCEC_Trace_in *cycle_in_temp);
		void B_Intra_task_checkpoint(int cur_block_index, int succ_block_index);
		void L_Intra_task_checkpoint(int cur_block_index, int succ_block_index);
		float discrete_handle(float new_freq, int rwcec);
		void checkpoint_operation(int block_index, int case_t);
		void exe_speed_config(void); // For determining all DVFS availability firstly
		void jitter_init(void); // Configuration of jitter constraints
		void exe_speed_scaling(float new_speed);
		void global_param_init(void);
		void constraint_update(void);
		void power_init(void);
		void power_eval(void);

		// The functions for final evaluation
		void global_param_eval(void);
		void output_result(char *case_msg);
		int cycles_cnt;
		
		//Configuration of Time Management 
		sys_clk_t *sys_clk;
		Time_Management *time_management; // Designate which time-management rules this task
		void timer_config(Time_Management *&timer);		

		// Test Case
		void pattern_init(vector< vector<int> > test_case);

		// Interrupt Timer and Preemption
		// Context register for interrupt timer and preemption
		isr_context_t isr_driven_cfg(int case_t, char DVFS_en);
		void dispatch_cfg(int &case_id, int case_t, float release_time_new, float start_time_new, float Deadline, char DVFS_en);
		int rem_wcec;		
		int executed_cycles;
		int cur_block_cycles;
		int cur_block_index;
		int cur_case_id;
		isr_context_t context_reg;

		// target control flow information
		Src_CFG(
			char *file_name, 
			Time_Management *&timer, 
			checkpoints_t &checkpoints_t, 
			RWCEC_Trace_in *cycle_in_temp,
			int *WCET_INFO, 
			vector< vector<int> > exe_path
		);
		~Src_CFG(void);

		B_checkpoints_t B_checkpoints;
		L_checkpoints_t L_checkpoints;
		P_checkpoints_t P_checkpoints;
		RWCEC_Trace_in *cycle_trace_in;
		int L_loop_cnt;
		int P_loop_cnt;
		vector<int> loop_bound;
		vector<int> L_loop_iteration;
		P_loop_iteration_t P_loop_iteration;
		vector<int> L_loop_exit;

// Multitask scheduling information
		void completion_config(void);
		float start_time;
		float release_time;
		/*Perhaps no need*/char  state; // READY, WAIT, IDLE, RUN, TERMINATE
		float abs_dline; // Absolute Deadline
		float rel_dline; // Relative Deadline
		int dline_miss;
		float wcet; 
		float bcet;
		bool completion_flag; // True: just completed; False: haven't completed or arrived yet	
		/*Perhaps no need*/float period;
		/*Perhaps no need*/char  prt; // Task Priority

// Energy/Power Evaluation parameters
		float energy_acc;
		float pre_eval_time;
};

/*
Sample Variance
(s^2) = sum from {i = 1} to {n} { [(x_i - x')^2] / (n - 1) }
*/
static float sample_variance(vector<float> &a) 
{
	int i; float acc = 0.0, acc_1 = 0.0; 

	for(i = 0; i < a.size(); i++) acc += a[i]; 
	acc = acc / a.size(); 
	for(i = 0; i < a.size(); i++) acc_1 += ((a[i] - acc) * (a[i] - acc));
	acc_1 = acc_1 / (a.size() - 1);

	return acc_1; 
}

#endif // __CFG_INFO_H
