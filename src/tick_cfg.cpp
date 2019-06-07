#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../inc/cfg_info.h"
#include "../inc/dvfs_info.h"
#include "../inc/main.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

extern double in_alpha;
//extern double in_default_speed[tasks_num];
extern double *in_default_speed;
extern int sim_cnt;
//extern double energy_ref;
extern double ISR_TIME_SLICE;
int cur_TskID;

/**
  * @brief Executing/simulating the sub-portion of control flow graph in terms of 
  *	   interrupt timer's period(preloaded cycle count). For sake of simulating 
  *        preemptive behaiour
  * @param case_t: designating the estimated 1)worst-case, 2)average-case, 
  *        3)best-case to entire execution
  * @param DVFS_en: enable or disable DVFS mechanism  
**/   
isr_context_t Src_CFG::isr_driven_cfg(int case_t, int DVFS_en)
{
	double time_temp;
	int isr_time_cycles, temp;
	int rem_cycles_timer; // Remaining exeuction cycles which still can be executed within a interrupt timer period
	dvfs_en = DVFS_en;
	isr_context_t context_reg;
//----------------------------------------------------------------------------------------------------------------//
	// Canonical number of execution cycles which will be taken within one period of interrupt timer
	// us -> ns	
	isr_time_cycles = (int) INST_UNIT;//(int) (((double) ISR_TIME_SLICE) * ((double) (time_management -> sys_clk -> cur_freq))) / 1000;
	context_reg.act_cycles = isr_time_cycles; 
	context_reg.act_exe_time = ((double) isr_time_cycles * 1000) / time_management -> sys_clk -> cur_freq;
	//printf("ISR_TIME_SLICE: %.05fns, act_exe_time: %.05fns, isr_time_cycles: %d\r\n", ISR_TIME_SLICE, context_reg.act_exe_time, isr_time_cycles);
	/*
	// Case 1) If Canonical number of exeuction cycles within on interrupt timer's period is the multiple of any instuction's CPI (assume all types of instructions take same execution cycle(s));
	// Case 2) If Canonaical number of execution cycles is not dividable for a instruction's CPI, then this must be one instruction which make system (interrupt timer) wait for, i.e., some additional 
	//         execution cycles/time would be required for flushing out such last instruction. For instance, when Canonical number is 10 cycles and CPI is 3 cycles, so there are totally 4 instructions
	//         may be fetched within a interrupt timer's period. However, whilst system runs out former 3 instructions and be executing the last 4th instruction, interrupt timer will finish one tick/
	//         countdown period and still remain 2 exeuction cycles' time haven't been run belonging to 4th instruction. So in this case, timer need to wait for this two exeuction cycles's time, that is,
	//         completely finish 4th instruction, instead of invoke the interruption immediately.
	rem_cycles_timer = (isr_time_cycles % (int) INST_UNIT == 0) ? isr_time_cycles : 
								     ((isr_time_cycles / (int) INST_UNIT) + (isr_time_cycles % (int) INST_UNIT)) *
							             ((int) INST_UNIT);	// Ex. => [(10 / 3) + (10 % 3)] * 3 = (3 + 1) * 3 = 12
	*/
	rem_cycles_timer = isr_time_cycles;
//printf("rem_cycles_timer: %d cycle(s), curBlock: %d, curBlock.get_cycle: %d, executed_cycles: %d\r\n", rem_cycles_timer,CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index(), CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t), executed_cycles);
	while(rem_cycles_timer > 0) {
		// When Current Basic Block is not able to be completed within an interrupt timer's period
		if((CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles) > rem_cycles_timer) {
			executed_cycles += rem_cycles_timer;
			rem_wcec -= rem_cycles_timer;
			rem_cycles_timer = 0; // End the current timer's period 
			// Since current Basic Block hasn't been run out yet, keeping the rest of portion for next upcoming timer's period/preloading
			// cur_block_index = cur_block_index; 
 	
			//cycles_cnt += context_reg.act_cycles;
			time_temp = time_management -> time_unit_config(
				// us -> ns
	                	((double) context_reg.act_cycles * 1000) / time_management -> sys_clk -> cur_freq   
       		        );

			//printf("(%.05f + %.05f) = ", time_temp, time_management -> sys_clk -> cur_time);
               		time_temp += time_management -> sys_clk -> cur_time;
			//printf("%.05f ", time_temp);
			time_management -> update_cur_time(time_temp);
			// (abolish) power_eval();
			//printf("%.05f, ", time_management -> sys_clk -> cur_time);
		}
		// When Current Basic Block is able to be completed before or at the end of an interrupt timer's period,
		// and also it is still remain some exeuction time until the end of interrupt timer"s countdown.
		// In this case, crossing multiple Basic Block within one timer's period is possible
		else if((CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles) < rem_cycles_timer) {
			cycles_cnt += CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t); // Accumulate the cycles count as soon as current BB just run out
	                time_temp = time_management -> time_unit_config(
				// us -> ns
	                	((double) (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles) * 1000) / time_management -> sys_clk -> cur_freq   
       		        );
               	
			time_temp += time_management -> sys_clk -> cur_time;
			time_management -> update_cur_time(time_temp);
			// (abolish) power_eval();
			
			context_reg.act_cycles -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles); 
			// us -> ns
			context_reg.act_exe_time -= (((double) (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles)) * 1000 / time_management -> sys_clk -> cur_freq);

			// After checkpoint, going to next successive Basic Block according to the given Test Pattern set
			rem_cycles_timer -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles);
			rem_wcec         -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles);
			executed_cycles = 0;
			if((cur_block_index + 1) != exe_path[cur_case_id].size()) {
				cur_block_index += 1;
			}
			else {
				cur_block_index = cur_block_index;
				rem_cycles_timer = 0;
				executed_cycles = 0x7FFFFFFF; // Dummy signal to let system now it's the end of current Test Pattern
			}
#ifdef DEBUG
			temp = CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index();
			if(temp != timeline_curBlock) {
				// Record remaining worst-case execution cycles after one Basic Block has already been passed through
				//rem_wcec -= CFG_path[timeline_curBlock - 1].get_cycles(case_t); 
				for(int j = 0; j < 15; j++) cout << "-"; 
				for(int j = 0; j < 8*TskID; j++) cout << "-"; 
				cout << "|" << TskID << "|" << endl;
				printf("[Cur_Freq: %.01f MHz]", time_management -> sys_clk -> cur_freq);
				cout << "(3)Block_" << timeline_curBlock <<  " -> ";
				printf("\r\n%fns(RWCEC:%d cycle(s))\t\t", time_management -> sys_clk -> cur_time, rem_wcec);
				timeline_curBlock = temp;
			}
#endif
		        // Before really crossing to next Basic Block, checking if current BB is a checkpoint 
			if(
				(sys_mode != (int) NORMAL && sys_mode != (int) L_JITTER) ||
				(sys_mode == (int) L_JITTER && response_case.size() >= (unsigned int) EVAL_CNT_START)
			) { 
				// Invoking the operation of B-type checkpoint
				if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].B_checkpoint_en != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system starts to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) {
#ifdef DEBUG
				cout << "CH(3)" << endl;
#endif
					B_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID 
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according to the indicated execution path case
					);		
				}
				// Invoking the operation of L-type checkpoint
				else if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].L_checkpoint_en[0] != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system starts to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) { 
#ifdef DEBUG
				cout << "CH(3)" << endl;
#endif
					L_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according ot the indicated execution path case
					);
				}			
				else if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].P_checkpoint_en != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system start to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) { 
#ifdef DEBUG	
				cout << "CH(3)" << endl;
#endif
					P_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID 
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according to the indicated execution path case
					);		
				}
				else {
				}	
			}	
		}
		// When Current Basic Block is able to be completed at exactly the end of an interrupt timer's period,
		else {
			cycles_cnt += CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t); // Accumulate the cycles count as soon as current BB just run out
	                time_temp = time_management -> time_unit_config(
				// us -> ns
	                	((double) (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles) * 1000) / time_management -> sys_clk -> cur_freq   
       		        );
			time_temp += time_management -> sys_clk -> cur_time;
               		time_management -> update_cur_time(time_temp); 
                	// (abolish) power_eval();
			context_reg.act_cycles -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles); 
			context_reg.act_exe_time -= ((double) (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles) * 1000 / time_management -> sys_clk -> cur_freq);
			
			// After checkpoint, going to next successive Basic Block according to the given Test Pattern set
			rem_cycles_timer -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles);
			rem_wcec         -= (CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_cycles(case_t) - executed_cycles);
			executed_cycles = 0;
			if(rem_cycles_timer != 0) {
				cout << endl << "=============================================" << endl;
				cout << "(1)rem_cycles_timer suppose be zero, however it is " << rem_cycles_timer << endl;
				cout << "=============================================" << endl;
				exit(1);
			}
			if((cur_block_index + 1) != exe_path[cur_case_id].size()) {
				cur_block_index += 1;
			}
			else {
				cur_block_index = cur_block_index;
				rem_cycles_timer = 0;
				executed_cycles = 0x7FFFFFFF; // Dummy signal to let system now it's the end of current Test Pattern
			}
#ifdef DEBUG
			temp = CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index();
			if(temp != timeline_curBlock) {
				// Record remaining worst-case execution cycles after one Basic Block has already been passed through
				//rem_wcec -= CFG_path[timeline_curBlock - 1].get_cycles(case_t); 
				for(int j = 0; j < 15; j++) cout << "-"; 
				for(int j = 0; j < 8*TskID; j++) cout << "-"; 
				cout << "|" << TskID << "|" << endl;
				printf("[Cur_Freq: %.01f MHz]", time_management -> sys_clk -> cur_freq);
				cout << "(4)Block_" << timeline_curBlock <<  " -> ";
				printf("\r\n%fns(RWCEC:%d cycle(s))\t\t", time_management -> sys_clk -> cur_time, rem_wcec);
				timeline_curBlock = temp;
			}
#endif
			if(
				(sys_mode != (int) NORMAL && sys_mode != (int) L_JITTER) ||
				(sys_mode == (int) L_JITTER && response_case.size() >= (unsigned int) EVAL_CNT_START)
			) { 
				// Invoking the operation of B-type checkpoint
				if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].B_checkpoint_en != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system start to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) {
#ifdef DEBUG
				cout << "CH(4)" << endl;
#endif
					B_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID 
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according to the indicated execution path case
					);		
				}
				// Invoking the operation of L-type checkpoint
				else if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].L_checkpoint_en[0] != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system start to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) { 
#ifdef DEBUG
				cout << "CH(4)" << endl;
#endif
					L_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according ot the indicated execution path case
					);
				}			
				else if(
					CFG_path[ exe_path[cur_case_id][cur_block_index - 1] - 1 ].P_checkpoint_en != 0x7FFFFFFF &&
					// When system finishes second BB from the sink, cur_block_index won't be incremented by 1,
					// in order to avoid the situation that whilst system start to run last BB and it still go to check the 
					// checkpoint of previous stage again
					executed_cycles != 0x7FFFFFFF // When system finishes second BB from the sink, cur_block_index won't be incremented by 1
				) { 
#ifdef DEBUG
				cout << "CH(4)" << endl;
#endif
					P_Intra_task_checkpoint(
						exe_path[cur_case_id][cur_block_index - 1],    // Cast current Basic Block ID 
						exe_path[cur_case_id][cur_block_index] // Cast its successive Basic Block ID according to the indicated execution path case
					);		
				}
				else {
				}	
			}	
		}
	}

	// Returning the actual execution time within this period of interrupt period(including overhead)
	if(rem_cycles_timer != 0) {
		cout << endl << "=============================================" << endl;
		cout << "(2)rem_cycles_timer suppose be zero, however it is " << rem_cycles_timer << endl;
		cout << "=============================================" << endl;
		exit(1);
	}
	return context_reg;
}

/**
  * @brief For temporal test of function isr_driven_cfg() 
**/
void Src_CFG::dispatch_cfg(int &case_id, int case_t, double release_time_new, double start_time_new, double Deadline, int DVFS_en)
{
	double time_temp;
	dvfs_en = DVFS_en;
	isr_context_t context_reg;
	exe_speed_config();

//--------------------------------------------------------------------------------//
// Setting the release time, start time, absolute deadline and relative deadline
	// The release time of current(new) instance 
	release_time  = time_management -> time_unit_config(release_time_new);
	
	// The start time of current(new) instance
	start_time = time_management -> time_unit_config(start_time_new);  
	
	rel_dline = time_management -> time_unit_config(Deadline);
	abs_dline = release_time + rel_dline;

	time_management -> update_cur_time(start_time);
//--------------------------------------------------------------------------------//
#ifndef DEBUG
	cout << "==================================================================" << endl;
	printf("#	Release time: %.05f us, Start time: %.05f us\r\n", release_time, start_time);
	printf("#       Absolute Deadline: %.05f us\r\n", abs_dline);
	printf("#	max: %.02f MHz, min: %.02f MHz, Default speed: %.02f MHz\r\n", max_freq_t, min_freq_t, sys_clk -> cur_freq);
	printf("#	Jitter constraint: BCET + (WCET - BCET) * %.02f%\r\n", jitter_config.alpha * 100);
	cout << "==================================================================" << endl;
//	cout << "Start -> " << endl; cout << "current time: " << sys_clk -> cur_time << endl;
#endif

	executed_cycles = 0;
	cur_case_id = case_id;
	for(cur_block_index = 0; exe_path[case_id][cur_block_index-1] != CFG_path.back().get_index(); ) {
#ifdef DEBUG
		cout << "Block_" << CFG_path[ exe_path[case_id][cur_block_index] - 1 ].get_index() << " -> ";
#endif
		context_reg = isr_driven_cfg((int) WORST, (int) DVFS_en);
		time_temp = time_management -> time_unit_config(
			context_reg.act_exe_time
		); 
		cycles_cnt += context_reg.act_cycles; 
#ifdef DEBUG	
		printf("\r\n#actual Execution cycles withing this period of interrupt timer: %d cycles\r\n", context_reg.act_cycles);
		printf("#Actual Execution time within this period of interrupt timer: %.05f us\r\n", time_temp);
#endif	

#ifdef DEBUG
		printf("Current Time: %.05f us\r\n", time_management -> sys_clk -> cur_time);	
#endif	
		//power_eval();
	}
#ifndef DEBUG
	cout << "End" << endl << endl;
#endif
	//power_eval();
	global_param_eval();
	completion_config();
}

void Src_CFG::ErrMsg(int case_t)
{
	if(
		CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() > CFG_path.back().get_index() || 
		CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() < CFG_path.front().get_index()
	) {
		printf("%d Current traversed Basic Block: %d, which is beyond the scope of this task's CFG\r\n", case_t, CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index());
		printf("exe_path[*][i]: %d, exe_path[*][i+1]: %d\r\n", exe_path[cur_case_id][cur_block_index], exe_path[cur_case_id][cur_block_index+1]);		
		exit(1);
	}
}
