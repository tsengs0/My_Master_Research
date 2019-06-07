#include "../inc/sched.h"
#include "../inc/main.h"

using std::cout;
using std::cin;
using std::endl;
using std::vector;
//extern int tasks_num;
extern int sim_mode;

Task_State_Bus::Task_State_Bus(Time_Management *timer, task_info_t *src_inter, vector<Src_CFG> &src_intra)
{
	time_management = timer;

	inter_tasks = src_inter;
	intra_tasks = src_intra;
	
	for(int i = 0; i < tasks_num; i++) {
		cout << "=====================================================" << endl;
		cout << "task_" << i << ":" << endl;
		cout << "Release time: " << src_inter[i].release_time << "ns" << endl;
		cout << "Priority: " << src_inter[i].prt << endl;
		cout << "Relative Deadline: " << src_inter[i].rel_dline << "ns" << endl;
		cout << "WCET: " << src_inter[i].wcet << "ns" << endl;
		cout << "BCET: " << src_inter[i].bcet << "ns" <<endl;
		cout << "Period: " << src_inter[i].period << "ns" << endl;
		cout << "Operating State: " << (int) (src_inter[i].state) << endl;
		cout << "Default WCRT: " << inter_tasks[i].wcrt << "ns" << endl;
		cout << "Default BCRT: " << inter_tasks[i].bcrt << "ns" << endl;
		cout << "=====================================================" << endl;
	}
}

Task_State_Bus::~Task_State_Bus(void)
{

}

void Task_State_Bus::scheduling_point_assign(int task_id, int case_t, int dvfs_en)
{
	// Passing new release_time and start_time to Intra-task CFG
	intra_tasks[task_id].traverse_spec_path(
			case_t, 
			WORST, 
			inter_tasks[task_id].release_time, 
			time_management -> sys_clk -> cur_time, 
			inter_tasks[task_id].rel_dline, 
			dvfs_en
	); 
}

void Task_State_Bus::start_new_task_config(
	int new_task_id, 
	int case_id, 
	int case_t, 
	double release_time_new,  
	double slack,
	double worst_interference_in,
	double avg_interference_in,
	double Deadline, 
	int DVFS_en
)
{
	double TimeAvailable_init, wcec_t, wcet_new, default_freq_new, f_new_lb, f_new_ub; 
	bool DVFS_Disable_Start = false;	
	intra_tasks[new_task_id].cycles_cnt = 0;	
	intra_tasks[new_task_id].cur_block_index = 0;
	intra_tasks[new_task_id].dvfs_en = DVFS_en;
	intra_tasks[new_task_id].exe_speed_config();
	intra_tasks[new_task_id].slack = slack;
/*
#ifdef AVERAGE_INTERFERENCE_EVAL
	intra_tasks[new_task_id].interference = avg_interference_in;
#else
	intra_tasks[new_task_id].interference = worst_interference_in - slack;
#endif
*/
	if(intra_tasks[new_task_id].sys_mode == (int) L_JITTER)
		intra_tasks[new_task_id].interference = (double) avg_interference_in;
	else
		intra_tasks[new_task_id].interference = (double) worst_interference_in;

	// Disable the DVFS at start point of designated task(s)
	for(unsigned int i = 0; i < (unsigned int) TSK_NONDVFS_FOR_START_NUM; i++) {
		if(NonDVFS_For_Start[i] == (unsigned int) new_task_id) {
			DVFS_Disable_Start = true;
			break;
		}
	}		

	if(
		(DVFS_Disable_Start == false) && (
		(intra_tasks[new_task_id].sys_mode != (int) NORMAL && intra_tasks[new_task_id].sys_mode != (int) L_JITTER) ||
		(intra_tasks[new_task_id].sys_mode == (int) L_JITTER && intra_tasks[new_task_id].response_case.size() >= (unsigned int) EVAL_CNT_START)
		)
	) { 
		if(slack < 0) {
			cout << "slack: " << slack << endl;	
			cout << "Wrong Calculation, slack may not be negative" << endl;
			exit(1);
		}

		else if(slack == 0.0) {/*cout << "Previously preemption task didn't leave any slack time" << endl;*/}	

		else if(intra_tasks[new_task_id].sys_mode == (int) H_PREDICT || intra_tasks[new_task_id].sys_mode == (int) L_POWER) { 
			TimeAvailable_init = intra_tasks[new_task_id].jitter_config.fin_time_target - 
					     intra_tasks[new_task_id].interference;
					      
			wcec_t = (double) (intra_tasks[new_task_id].execution_cycles[WORST - 1]);
			default_freq_new = intra_tasks[new_task_id].discrete_handle(
							// GHz -> MHz
							((1000 * wcec_t) / TimeAvailable_init), 
							(int) wcec_t, 
							TimeAvailable_init,
							(intra_tasks[new_task_id].wcrt + intra_tasks[new_task_id].release_time - time_management -> sys_clk -> cur_time)
			);
			double freq_temp = (double) intra_tasks[new_task_id].default_freq_t; // Only use for exporting log file
			intra_tasks[new_task_id].default_freq_t = default_freq_new;
			intra_tasks[new_task_id].exe_speed_scaling(default_freq_new);
#ifdef DEBUG		
			char *msg_slack = new char[200];
			sprintf(msg_slack, "echo \"(Start Point) curTime: %.05fns, TskID: %d, Slack:%.05f, PrevFreq: %.05f, UpdateFreq: %.05f\" >> slack_reclaim.start.sim_%d.txt", 
					time_management -> sys_clk -> cur_time,
					new_task_id,
					slack,
					freq_temp,
					default_freq_new,
					sim_mode
			);
			system(msg_slack);
			delete [] msg_slack;
#endif
		}
		else { // intra_tasks[new_task_id].sys_mode == (int) L_JITTER) 
			wcec_t = (double) (intra_tasks[new_task_id].execution_cycles[WORST - 1]);

			double response_temp = (intra_tasks[new_task_id].response_case.size() == 0) ? inter_tasks[new_task_id].wcrt : intra_tasks[new_task_id].average_response;
			//double response_temp = intra_tasks[new_task_id].interference + (wcec_t / intra_tasks[new_task_id].default_freq_t); 
			if(response_temp < intra_tasks[new_task_id].min_response) { // To avoid slack time lead to newly minimal response time among all instances
				f_new_ub = (1000 * wcec_t) / (intra_tasks[new_task_id].min_response - intra_tasks[new_task_id].interference);
				//f_new_ub = (1000 * wcec_t) / (intra_tasks[new_task_id].min_response - interference_in + slack);
				f_new_lb = (1000 * wcec_t) / (intra_tasks[new_task_id].max_response - intra_tasks[new_task_id].interference);
				//f_new_lb = (1000 * wcec_t) / (intra_tasks[new_task_id].max_response - interference_in + slack);
				/*default_freq_new = intra_tasks[new_task_id].discrete_handle(
								// GHz -> MHz
								f_new_ub,
								(int) wcec_t, 
								intra_tasks[new_task_id].min_response - interference_in + slack,
								(intra_tasks[new_task_id].average_response + intra_tasks[new_task_id].release_time - time_management -> sys_clk -> cur_time)
				);*/
				default_freq_new = intra_tasks[new_task_id].discrete_handle_SelectBound(f_new_ub, (int) LOWER_BOUND);
				
			}
			else {
				default_freq_new = (double) intra_tasks[new_task_id].default_freq_t; 
			}
			double freq_temp = (double) intra_tasks[new_task_id].default_freq_t; // Only use for exporting log file
			intra_tasks[new_task_id].default_freq_t = default_freq_new;
			intra_tasks[new_task_id].exe_speed_scaling(default_freq_new);
#ifdef DEBUG		
			char *msg_slack = new char[200];
			sprintf(msg_slack, "echo \"(Start Point) curTime: %.05fns, TskID: %d(inst.:%d), Slack:%.05f, PrevFreq: %.05f, UpdateFreq: %.05f(LB: %.05f, UB: %.05f)\" >> slack_reclaim.start.sim_%d.txt", 
					time_management -> sys_clk -> cur_time,
					new_task_id,
					intra_tasks[new_task_id].response_case.size() + 1,
					slack,
					freq_temp,
					default_freq_new,
					f_new_lb,
					f_new_ub,
					sim_mode
			);
			system(msg_slack);
			delete [] msg_slack;
#endif
		}
	}
//--------------------------------------------------------------------------------//
// Setting the release time, start time, absolute deadline and relative deadline
	// The release time of current(new) instance 
	intra_tasks[new_task_id].release_time  = time_management -> time_unit_config(release_time_new);
	
	// The start time of current(new) instance
	intra_tasks[new_task_id].start_time = time_management -> sys_clk -> cur_time;  
	//intra_tasks[new_task_id].pre_eval_time = intra_tasks[new_task_id].start_time;
	//cout << "pre_eval_time updating (1)" << endl;
	
	intra_tasks[new_task_id].rel_dline = time_management -> time_unit_config(Deadline);
	intra_tasks[new_task_id].abs_dline = intra_tasks[new_task_id].release_time + intra_tasks[new_task_id].rel_dline;

	time_management -> update_cur_time(intra_tasks[new_task_id].start_time);
//--------------------------------------------------------------------------------//
#ifdef DEBUG
	char SysMode_msg[25];
	if(intra_tasks[new_task_id].dvfs_en == (int) NonDVFS_sim) sprintf(SysMode_msg, "NonDVFS_sim");
	else if(intra_tasks[new_task_id].dvfs_en == (int) DVFS_sim) sprintf(SysMode_msg, "DVFS_sim");
	else if(intra_tasks[new_task_id].dvfs_en == (int) DVFSOverhead_sim) sprintf(SysMode_msg, "DVFSOverhead_sim");
	else if(intra_tasks[new_task_id].dvfs_en == (int) DVFSAlphaBound_sim) sprintf(SysMode_msg, "DVFSAlphaBound_sim");
	else if(intra_tasks[new_task_id].dvfs_en == (int) DVFSAvgResp_sim) sprintf(SysMode_msg, "DVFSAvgResp_sim");
	cout << "==================================================================" << endl;
	printf("#	The %dth instance\r\n", intra_tasks[new_task_id].response_case.size()); 
	printf("#	The System Mode: %s\r\n", SysMode_msg); 
	printf("#	Release time: %.05f ns, Start time: %.05f ns\r\n", 
				intra_tasks[new_task_id].release_time, 
				intra_tasks[new_task_id].start_time
	);
	printf("#	Target Response Time: %.05f ns\r\n", intra_tasks[new_task_id].jitter_config.fin_time_target); 
	printf("#	Worst Case Response Time: %.05f ns\r\n", intra_tasks[new_task_id].wcrt); 
	printf("#	Average Response Time: %.05f ns\r\n", intra_tasks[new_task_id].average_response); 
	printf("#	Best Case Response Time: %.05f ns\r\n", intra_tasks[new_task_id].bcrt); 
	printf("#	Propagated Slack Time: %.05f ns\r\n", intra_tasks[new_task_id].slack); 
	printf("#	Absolute Deadline: %.05f ns\r\n", intra_tasks[new_task_id].abs_dline); 
	printf("#	max: %.02f MHz, min: %.02f MHz, Default speed: %.02f MHz\r\n", 
				intra_tasks[new_task_id].max_freq_t, 
				intra_tasks[new_task_id].min_freq_t, 
				time_management -> sys_clk -> cur_freq
	);
	printf("#	Jitter constraint: BCET + (WCET - BCET) * %.02f%\r\n", intra_tasks[new_task_id].jitter_config.alpha * 100);
	cout << "==================================================================" << endl;
#endif
	
	intra_tasks[new_task_id].executed_cycles = 0;
	intra_tasks[new_task_id].cur_case_id = case_id;
	intra_tasks[new_task_id].rem_wcec = intra_tasks[new_task_id].execution_cycles[WORST - 1];
	intra_tasks[new_task_id].timeline_curBlock = intra_tasks[new_task_id].CFG_path.front().get_index();
	for(unsigned int i = 0; i < intra_tasks[new_task_id].L_loop_iteration_preload.size(); i++) {
	  intra_tasks[new_task_id].L_loop_iteration.at(i) = intra_tasks[new_task_id].L_loop_iteration_preload[i][case_id];
	}
#ifdef DEBUG
	cout << endl << time_management -> sys_clk -> cur_time << " ns(RWCEC:" << intra_tasks[new_task_id].rem_wcec << ")\t\t";
#endif
}

void Task_State_Bus::time_driven_cfg(int new_task_id, int case_t)
{
	double time_temp;
	int case_id = intra_tasks[new_task_id].cur_case_id;
	int cur_block_index = intra_tasks[new_task_id].cur_block_index; 
	if(intra_tasks[new_task_id].exe_path[case_id][cur_block_index] != intra_tasks[new_task_id].exe_path[case_id].back()) {
#ifdef DEBUG
		int temp = intra_tasks[new_task_id].CFG_path[intra_tasks[new_task_id].exe_path[case_id][cur_block_index] - 1].get_index();
		if(temp != intra_tasks[new_task_id].timeline_curBlock) {
			// Record remaining worst-case execution cycles after one Basic Block has already been passed through
			//intra_tasks[new_task_id].rem_wcec -= intra_tasks[new_task_id].CFG_path[intra_tasks[new_task_id].timeline_curBlock - 1].get_cycles(case_t); 
			for(int j = 0; j < 15; j++) cout << "-"; 
			for(int j = 0; j < 8*new_task_id; j++) cout << "-"; 
			cout << "|" << new_task_id << "|" << endl;
			printf("[Cur_Freq: %.01f MHz]", time_management -> sys_clk -> cur_freq);
			cout << "(1)Block_" << intra_tasks[new_task_id].timeline_curBlock << " -> ";
			printf("\r\n%fns(RWCEC:%d cycle(s))\t\t", time_management -> sys_clk -> cur_time, intra_tasks[new_task_id].rem_wcec);
			intra_tasks[new_task_id].timeline_curBlock = temp;
		}
#endif
		intra_tasks[new_task_id].context_reg = intra_tasks[new_task_id].isr_driven_cfg((int) WORST, intra_tasks[new_task_id].dvfs_en);
		//cout << "power_eval (0)" << endl;
		intra_tasks[new_task_id].power_eval();			
	}
	else {
/*#ifdef DEBUG
		int temp = intra_tasks[new_task_id].CFG_path[intra_tasks[new_task_id].exe_path[case_id][cur_block_index] - 1].get_index();
		if(temp != intra_tasks[new_task_id].timeline_curBlock) {
			// Record remaining worst-case execution cycles after one Basic Block has already been passed through
			//intra_tasks[new_task_id].rem_wcec -= intra_tasks[new_task_id].CFG_path[intra_tasks[new_task_id].timeline_curBlock - 1].get_cycles(case_t); 
			for(int j = 0; j < 15; j++) cout << "-"; 
			for(int j = 0; j < 8*new_task_id; j++) cout << "-"; 
			cout << "|" << new_task_id << "|" << endl;
			printf("[Cur_Freq: %.01f MHz]", time_management -> sys_clk -> cur_freq);
			cout << "(2)Block_" << intra_tasks[new_task_id].timeline_curBlock << " -> ";
			cout << endl << time_management -> sys_clk -> cur_time << " ns(RWCEC:" << intra_tasks[new_task_id].rem_wcec << ")\t\t";
			intra_tasks[new_task_id].timeline_curBlock = temp;
		}
#endif*/
		intra_tasks[new_task_id].context_reg = intra_tasks[new_task_id].isr_driven_cfg((int) WORST, intra_tasks[new_task_id].dvfs_en);
		//cout << "power_eval (1)" << endl;
		intra_tasks[new_task_id].power_eval();			

		if((cur_block_index + 1) == intra_tasks[new_task_id].exe_path[case_id].size() && intra_tasks[new_task_id].executed_cycles == 0x7FFFFFFF) {
			intra_tasks[new_task_id].executed_cycles = 0; // Remove the temporarily dummy signal
#ifdef DEBUG
			// Record remaining worst-case execution cycles after one Basic Block has already been passed through
			//intra_tasks[new_task_id].rem_wcec -= intra_tasks[new_task_id].CFG_path[intra_tasks[new_task_id].timeline_curBlock - 1].get_cycles(case_t); 
			for(int j = 0; j < 15; j++) cout << "-"; 
			for(int j = 0; j < 8*new_task_id; j++) cout << "-"; 
			cout << "|" << new_task_id << "|" << endl;
			printf("[Cur_Freq: %.01f MHz]", time_management -> sys_clk -> cur_freq);
			cout << "(2)Block_" << intra_tasks[new_task_id].timeline_curBlock << " -> ";
			printf("\r\n%fns(RWCEC:%d cycle(s))\r\n", time_management -> sys_clk -> cur_time, intra_tasks[new_task_id].rem_wcec);
			cout << "End" << endl << endl;
			cout << endl << "Completion Time: " << time_management -> sys_clk -> cur_time << " ns\t\t";
			cout << endl << "Evaluation of Task_" << new_task_id << ":" << endl;		
#endif
			//cout << "power_eval (2)" << endl;
			//intra_tasks[new_task_id].power_eval();
			intra_tasks[new_task_id].global_param_eval();
			intra_tasks[new_task_id].completion_config();
			cout << endl;
		}
	}
}
