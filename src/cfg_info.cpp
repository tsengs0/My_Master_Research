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

//extern double in_default_speed[tasks_num];
extern double *in_default_speed;
extern int sim_cnt;
extern int sim_mode;
//extern double energy_ref;
//extern double energy_ref_overhead;
double ISR_TIME_SLICE;

int Basic_block::get_cycles(int case_t)
{
	return execution_cycles[(int) (case_t - 1)];
}

int Basic_block::get_succ(int succ_index)
{
	return succ[succ_index];
}

int Basic_block::get_index(void)
{
	return block_index;
}

Basic_block::Basic_block(int curr_index, vector<int> &succ_index, vector<int> &cycles)
{
	int SuccNum = 0;
	int i;
	
	SuccNum = succ_index.size(); 

	for(i = 0; i < SuccNum; i++) succ.push_back(succ_index[i]);

	execution_cycles[0] = cycles[0];
	execution_cycles[1] = cycles[1];
	execution_cycles[2] = cycles[2];

	block_index = curr_index;
	B_checkpoint_en = 0x7FFFFFFF; // Disable itself as non-checkpoint 
	L_checkpoint_en[0] = 0x7FFFFFFF;
	L_checkpoint_en[1] = 0x7FFFFFFF;
	P_checkpoint_en = 0x7FFFFFFF;  
#ifdef DEBUG	
	cout << "Block_" << block_index << ", ";
	cout << "WCEC: " << execution_cycles[0] << " ACEC: " << execution_cycles[1] << " BCEC: " << execution_cycles[2] << endl;
	cout << "Successors: ";
	for(i = 0; i < SuccNum; i++)
		cout << succ[i] << " ";
	cout << endl << endl;
#endif
}

Basic_block::~Basic_block(void)
{

}

Src_CFG::Src_CFG(
	char *file_name,
	Time_Management *timer, 
	checkpoints_label *checkpoint_label_temp, 
	RWCEC_Trace_in *cycle_trace_temp,
	checkpoint_num *checkpointNum_temp,
	exeTime_info WCET_INFO,
	double alpha,
	int SysMode_in,
	int TskID_in
)
{
	FILE *fp;
	unsigned int c, i, j;
	unsigned char fsm_state = (unsigned char) ATTRIBUTES_ITEM;
	unsigned char fsm_state_internal = (unsigned char) TASK_ID;
	vector<int> succ_conv_temp;
	TskID = TskID_in;
	succ_int_temp = 0;
	fp = fopen(file_name, "r");
	while(1) {
		c = fgetc(fp); 
		if( feof(fp) ) break;
		else  {
			if(c == 0x0A || c == ' ') { while( c == 0x0A || c == ' ' ) c = fgetc(fp);  }
		//	printf("c = %c state:0x%02X 0x%02X\r\n", c, (unsigned char) fsm_state, (unsigned char) fsm_state_internal);	
			if( c == '{' || c == '}' ) {		
				switch(c)
				{
					case '{': // Identifying 1) Task Attributes or 2) Its successors list
						fsm_state = (fsm_state == (unsigned char) ATTRIBUTES_ITEM) ? (unsigned char) TASK_ATTRIBUTES : 
							    (fsm_state == (unsigned char) SUCCESSORS_ITEM) ? (unsigned char) SUCCESSORS      :
											                             fsm_state;
						break;
				
					case '}': 
						c = fgetc(fp);
						if(c == ',') 
							fsm_state = (unsigned char) SUCCESSORS_ITEM;
						else if(c == ';') {
							fsm_state = (unsigned char) ATTRIBUTES_ITEM;
							fsm_state_internal = (unsigned char) TASK_ID;
							wcec.push_back(wcec_temp); wcec_temp.clear();
							acec.push_back(acec_temp); acec_temp.clear();
							bcec.push_back(bcec_temp); bcec_temp.clear();
							
							// After last element, clearing is still needed
							succ.push_back(succ_int_temp); succ_int_temp = 0; 
							
							task_succ.push_back(succ); succ.clear();
							cout << endl;
						}
						else 
							cout << "Wrong Format, expecting either\",\" or \";\"" << endl;
						break;
	
					default:
						break;
				}
			}
			else {
				if(fsm_state == (unsigned char) TASK_ATTRIBUTES && fsm_state_internal == (unsigned char) TASK_ID) {
					if(c != ',') task_id_temp.push_back(c);
					else { task_id.push_back(task_id_temp); task_id_temp.clear(); fsm_state_internal += 0x01; } 
				}
				else if(fsm_state == (unsigned char) TASK_ATTRIBUTES && fsm_state_internal != (unsigned char) TASK_ID) {
					if     (fsm_state_internal == (unsigned char) WCEC && c != ',') wcec_temp.push_back(c);
					else if(fsm_state_internal == (unsigned char) ACEC && c != ',') acec_temp.push_back(c);
					else if(fsm_state_internal == (unsigned char) BCEC && c != '}') bcec_temp.push_back(c);
					else fsm_state_internal += 0x01; 
											   
				}
				else { // fsm_state = SUCCESSORS
					if(c != ',') {
						succ_int_temp = succ_int_temp * 10;
						succ_int_temp = (c != '#') ? succ_int_temp + ( c - 48 ) : 0;		
					}
					else { // c = ','
						succ.push_back(succ_int_temp);
						succ_int_temp = 0;
					}
				}
			}
		}
	}
	fclose(fp);
	for(i = 0; i < task_id.size(); i++) {
		int id_temp = 0; vector<int> cycles_temp;

		for(j=0;j<task_id[i].size();j++) {
			id_temp *= 10;
			id_temp += ( task_id[i][j] - 48 );
		}

		for(j=0;j<task_succ[i].size();j++) succ_conv_temp.push_back(task_succ[i][j]);//succ_conv_temp[j] = task_succ[i][j];

		cycles_temp.push_back((int) 0); cycles_temp.push_back( (int) 0 ); cycles_temp.push_back( (int) 0 );
		for(j=0;j<wcec[i].size();j++) {	
			cycles_temp[0] *= 10;
			cycles_temp[0] += ( wcec[i][j] - 48 );
		}
		
		for(j=0;j<acec[i].size();j++) {	
			cycles_temp[1] *= 10;
			cycles_temp[1] += ( acec[i][j] - 48 );
		}
		
		for(j=0;j<bcec[i].size();j++) {	
			cycles_temp[2] *= 10;
			cycles_temp[2] += ( bcec[i][j] - 48 );
		}
		
		CFG_path.push_back( 
					Basic_block( 
						(int) id_temp, // Task id
						      succ_conv_temp, // Enumeration of all successors
						      cycles_temp 
					) 
		);
		vector<int>().swap(cycles_temp); vector<int>().swap(succ_conv_temp);
	}


	sys_mode = SysMode_in;	
	in_alpha = alpha;
	
	// According to the input file
	exe_cycle_tracing(WCET_INFO, cycle_trace_temp, checkpointNum_temp);
	
	// Checkpoints insertion and generating its corresponding mining table
	checkpoints_placement(checkpoint_label_temp); 
	mining_table_gen(); 
	
	// Designating the system-clock/timer source
	timer_config(timer);
	
	global_param_init();
//	pattern_init(test_case);
	vector<int>().swap(succ_conv_temp); // Free vector's memory space
}

Src_CFG::~Src_CFG(void)
{
}

void Src_CFG::dvfs_config(int env) {
	dvfs_en = env;
}

void Src_CFG::timer_config(Time_Management *timer)
{
	time_management = timer;
	sys_clk = timer -> sys_clk;
}

void Src_CFG::global_param_init(void)
{
	// Setting the DVFS-available specification
	exe_speed_config(); 
	
	// Initialising some global parameters
	exe_var = 0.0;
	response_time = 0.0;
	max_response = 0.0;
	min_response = 0x7FFFFFFF;
	max_response_fb = 0.0;
	min_response_fb = 0x7FFFFFFF;
	AFJ = 0.0;
	RFJ = 0.0;
	average_response = 0.0;
	AvgResp_ref[0] = (double) 0.0;
	AvgResp_ref[1] = (double) 0.0;
	response_acc = 0.0;
	exe_acc = 0.0;
	cycle_acc = 0;

	// The unit of frequency is "MHz", in order to get the unit of time as "ns", 
	// Multiplying the result by 1000: us -> ns
	wcet = (double) ((1000 * execution_cycles[WORST - 1]) / default_freq_t);  
	bcet = (double) ((1000 * execution_cycles[BEST - 1]) / max_freq_t);
#ifdef DEBUG
	printf("Intra-Task_%d Global Parameter Configuration:\r\n", TskID);
	printf("WCET: %.05f ns\r\nBCET: %.05f ns\r\n\r\n", wcet, bcet);
#endif
	dline_miss = 0;
	completion_flag = false;
	rem_wcec = 0;
	cycles_cnt = 0;
	slack = (double) 0.0;
	
/* Abolish, jitter concept ought to depend on multitasking instead of single task      ---- 13 FEB, 2018
	// Setting the Jitter constraints
	jitter_init(); 
*/
	// Initialising the context register
	ISR_TIME_SLICE = (1000 * INST_UNIT) / default_freq_t; // Convert us to ns
	executed_cycles = 0;
	cur_block_cycles = 0;
	cur_case_id = 0;
	cur_block_index = 0;

	// Initialising energy/power evaluation parameters
	power_init();
}

void Src_CFG::exe_speed_config(void)
{
	// DVFS setting
	max_freq_t     = (double) MAX_speed;
	min_freq_t     = (double) MIN_speed;
	default_freq_t = (in_default_speed[TskID] > max_freq_t) ? max_freq_t : (in_default_speed[TskID] < min_freq_t) ? min_freq_t : in_default_speed[TskID];
	//default_freq_t = (default_freq_t != max_freq_t && default_freq_t != min_freq_t) ? discrete_handle(default_freq_t) : default_freq_t;
	time_management -> cur_freq_config(default_freq_t);
}

void Src_CFG::pattern_init(ExePath_set test_case)
{
	exe_path = test_case;
#ifdef DEBUG
	cout << "Task ";
	for(int i = task_id_temp.size() - 1; i >= 0; i--) cout << task_id_temp[i];
	cout << "'s test pattern:" << endl;
	for(int j = 0; j < exe_path.size(); j++) {
         cout << "Test Case " << j + 1 << ": " << endl;
	 for(int k = 0; k < exe_path[j].size(); k++) cout << "Block_" << exe_path[j][k] << endl;
	}
#endif	
//=======================================================================================================================================================//
}

void Src_CFG::global_param_eval(void)
{
	double Act_CET, response_pre, response_cur, response_diff;
 
	dline_miss = (time_management -> sys_clk -> cur_time > abs_dline) ? dline_miss + 1 : dline_miss;
	if(dline_miss != 0) {
#ifdef DEBUG
		char *msg_dline = new char[200];
		cout << endl << endl << endl;
		printf("Task_%d missed deadline, Current Time is %fns, but absolute deadline is %fns\r\n", TskID, time_management -> sys_clk -> cur_time, abs_dline);
		sprintf(msg_dline, "echo \"Tsk_%d missed deadline, curTime: %fns, Abs_dline: %fns\" >> dline_miss.Tsk_%d.sim_%d.txt", 
			TskID,
			time_management -> sys_clk -> cur_time,
			abs_dline,
			TskID,
			sim_mode
		);
		system(msg_dline);
		delete [] msg_dline;
#endif
		printf("Task_%d missed deadline, Current Time is %fns, but absolute deadline is %fns\r\n", TskID, time_management -> sys_clk -> cur_time, abs_dline);
		//exit(1);
	}
/*Need to be modified*/	Act_CET = (double) (time_management -> sys_clk -> cur_time - start_time); // Regardless of preemption
	cycle_acc += cycles_cnt;
	exe_acc += Act_CET;
	exe_case.push_back(Act_CET);
	response_pre = response_time;
	response_cur = time_management -> sys_clk -> cur_time - release_time;
	response_time = response_cur;
	response_diff = (response_pre == 0) ? 0: 
			(response_cur >= response_pre) ? (response_cur - response_pre) : (response_pre - response_cur);
	response_acc += response_cur;
	response_case.push_back(response_cur);
	average_response = response_acc / response_case.size(); 
	response_SampleVariance = sample_variance(response_case);
	response_SampleVarianceDeviation = sqrt(response_SampleVariance);

	tar_diff = jitter_config.fin_time_target - Act_CET;
	tar_diff = (tar_diff / jitter_config.fin_time_target) * 100;
	exe_var  = (((exe_acc / exe_case.size()) - bcet) / jitter_config.fin_jitter_bound) * 100;
	max_response = (max_response < response_cur) ? response_cur : max_response;
	min_response = (min_response > response_cur) ? response_cur : min_response;
	//printf("(Tsk_%d) Max_Respons: %f ns, Min_Respons: %f ns, AFJ: %f ns\r\n", TskID, max_response, min_response, max_response - min_response);
	//char aa; cin >> aa;

	if(response_case.size() > (unsigned int) (EVAL_CNT_START + 1) && sys_mode == (int) L_JITTER) {
		max_response_fb = (max_response_fb < response_cur) ? response_cur : max_response_fb;
		min_response_fb = (min_response_fb > response_cur) ? response_cur : min_response_fb;
		AFJ = max_response_fb - min_response_fb;
		RFJ = (RFJ < response_diff) ? response_diff : RFJ;
	}
	else if(response_case.size() == (unsigned int) (EVAL_CNT_START + 1) && sys_mode == (int) L_JITTER) {
		max_response = average_response;
		min_response = average_response;
		AFJ = (double) 0.0; //max_response - min_response;
		RFJ = (double) 0.0; //max_response - min_response;
	}
	else { // For NORMAL mode
		AFJ = max_response - min_response;
		RFJ = (RFJ < response_diff) ? response_diff : RFJ;
	}

#ifdef DEBUG
	cout << "Actual Execution Cycles: " << cycles_cnt << " cycles" << endl;
	printf("Relative Deadline: %.05f ns\r\n", rel_dline);
	printf("Worst Case Execution Time: %.05f ns\r\n", wcet);
	printf("Best Case Execution Time: %.05f ns\r\n", bcet);
	printf("Actual Execution Time: %.05f - interference time ns(%.02f%%)\r\n", Act_CET, ((Act_CET - bcet)  / jitter_config.fin_jitter_bound) * 100);  
	printf("Target Response Time: %.05f ns(%.02f%%)\r\n", jitter_config.fin_time_target, jitter_config.alpha * 100);
	printf("Actual Response Time: %.05f ns\r\n", response_cur);
	printf("Average Response Time: %.05f ns\r\n", average_response);
	printf("Absolute Deadline: %.05f ns\r\n", abs_dline);
	printf("Completion time: %.05f ns\r\n", sys_clk -> cur_time);
	printf("The number of Deadline miss: %d\r\n", dline_miss);
	//printf("The difference between actual and target execution time: %.02f\% \r\n", tar_diff);
	printf("The total exeuction cycles: %d cycles\r\n", cycle_acc);
//	printf("The variation on execution time: %.02f% \r\n", exe_var);
	printf("Sample Standard Deviation: %.05f ns(%.02f%%)\r\n", response_SampleVarianceDeviation, (sqrt(response_SampleVariance) / jitter_config.fin_jitter_bound) * 100);
	printf("Relative finishing time jitter(RFJ): %.05f ns.\r\n", RFJ);
	printf("Absolute finishing time jitter(AFJ): %.05f ns.\r\n", AFJ);
	printf("Energy consumption: %.05fnJ\r\n", energy_acc);
	
	if(max_response != AvgResp_ref[1] || min_response != AvgResp_ref[0]) {
		char *msg_afj = new char[200];
		sprintf(msg_afj, "echo \"Tsk_%d, (inst.: %d), MinResp: %fns, MaxResp: %fns, AFJ: %fns\" >> AFJUpdate.Tsk_%d.sim_%d.txt", 
			TskID,
			response_case.size(),
			min_response,
			max_response,
			AFJ,
			TskID,
			sim_mode
		);
		system(msg_afj);
		delete [] msg_afj;
		AvgResp_ref[1] = max_response;
		AvgResp_ref[0] = min_response;
	}
	char *msg_cycles = new char[200];
	sprintf(msg_cycles, "echo \"Tsk_%d, (inst.: %d), %d cycles\" >> cycle_cnt.Tsk_%d.sim_%d.txt", 
		TskID,
		response_case.size(),
		cycles_cnt,
		TskID,
		sim_mode
	);
	system(msg_cycles);
	delete [] msg_cycles;
#endif
}

void Src_CFG::output_result(char* case_msg) {
	assert(strlen(case_msg) > 0);
	char ExeVar_msg[500], csv_msg[500];
	int len = sprintf(
				ExeVar_msg, 

				"echo \"Task_%08d: Alpha:%14.05f, CycleAcc:%d(cycle), RFJ:%12.05f(ns), AFJ:%12.05f(ns), TargetResponse:%12.05f(ns), AverageResponse:%12.05f(ns), SampleStandardDeviation:%12.05f(ms), Energy:%16.05f(nJ), DeadlineMiss: %02d\" >> test_result_%s.txt", 
	
				TskID,
				in_alpha,
				cycle_acc,
				RFJ,
				AFJ,
				jitter_config.fin_time_target,
				average_response,
				response_SampleVarianceDeviation,		
				energy_acc,
				dline_miss,
				case_msg
	);
	assert(len == strlen(ExeVar_msg));
	system((char*) ExeVar_msg); 

	len = sprintf(csv_msg, "echo \"%12.05f\,%12.05f\,%12.05f\,%12.05f\,%12.05f\,%16.05f\,%02d\" >> %s.csv", 
		RFJ,
		AFJ,
		jitter_config.fin_time_target,
		average_response,
		response_SampleVarianceDeviation,
		energy_acc,
		dline_miss,
		case_msg
	);
	assert(len == strlen(csv_msg));
	system((char*) csv_msg);
}

void Src_CFG::power_init(void)
{
	energy_acc    = 0.0;
	pre_eval_time = 0.0;
	pre_eval_time_cnt = 0;
	pre_eval_UpdatePoint = 0.0;
}

void Src_CFG::power_eval(void)
{
#ifdef PROCESSOR_AM335x
	int i;
	
#ifdef DISCRETE_DVFS
	for(i = 0; time_management -> sys_clk -> cur_freq != freq_vol[i][0]; i++);
	//energy_acc += ((time_management -> sys_clk -> cur_time - pre_eval_time) * MPU_POWER[i]) / 1000; // pJ -> nJ
	if(
		(pre_eval_time != time_management -> sys_clk -> cur_time && pre_eval_UpdatePoint != time_management -> UpdatePoint) || 
		(pre_eval_time == 0.0 && pre_eval_UpdatePoint == 0.0)
	) {
		energy_acc += ((time_management -> sys_clk -> cur_time - time_management -> UpdatePoint ) * MPU_POWER[i]) / 1000; // pJ -> nJ
	}
	else {
		//printf("pre_eval_time = %f ns\r\n", pre_eval_time);
		//printf("curTime = %f ns\r\n", time_management -> sys_clk -> cur_time);
		//printf("pre_eval_UpdatePoint = %f ns\r\n", pre_eval_UpdatePoint);
		//printf("UpdatePoint = %f ns\r\n", time_management -> UpdatePoint);
		//exit(1);
	}
#endif
	//pre_eval_time = time_management -> sys_clk -> cur_time;
	/*if(pre_eval_time == time_management -> UpdatePoint && pre_eval_time_cnt > 1) {
		printf("TskID: %d, curTime: %f, UpdatePoint_%f has been handled more than once\r\n", 
				TskID,
				time_management -> sys_clk -> cur_time,
				pre_eval_time
		);
		exit(1);
	}
	pre_eval_time_cnt += 1;*/
	pre_eval_time = time_management -> sys_clk -> cur_time;	
	pre_eval_UpdatePoint = time_management -> UpdatePoint;
	time_management -> ExecutedTime_Accumulator((unsigned char) SYS_POWER_EVAL_POINT, (int) TskID);
	//energy_ref = (dvfs_en == (int) NonDVFS_sim) ? energy_acc : 
	//	     (dvfs_en == (int) DVFS_sim   ) ? energy_ref : energy_ref_overhead;
#endif
}

void Src_CFG::completion_config(void)
{
	completion_flag = true;
	
	// Since task has completed its current job, preloading the loop iteration counter by the values of their corresponding Loop Bounds
	//for(int i = 0; i < checkpointLabel -> L_checkpoints.size(); i++) L_loop_iteration.at(i) = checkpointLabel -> L_loop_bound[i] + 1;
	for(unsigned int i = 0; i < checkpointLabel -> P_checkpoints.size(); i++) P_loop_iteration.at(i) = checkpointLabel -> P_loop_bound[i];

	// Reset the runtime information
	cycles_cnt = 0;	
	rem_wcec = 0;

	// Reset the context register
	executed_cycles = 0;
	cur_block_cycles = 0;
	cur_case_id = 0;
	cur_block_index = 0;

	// Because the default speed may be change after frequency scaling operation of start point 
	// thus the default frequency have to be reset again
	exe_speed_config();
}

void Src_CFG::traverse_spec_path(int case_id, int case_t, double release_time_new, double start_time_new, double Deadline, int DVFS_en)
{
	int cur_index;
	double time_temp;
	dvfs_en = DVFS_en;
	exe_speed_config();
	cur_case_id = case_id; // Information for Lookahead P-type checkpoint test pattern 
//--------------------------------------------------------------------------------//
// Setting the release time, start time, absolute deadline and relative deadline
	// The release time of current(new) instance 
	release_time  = time_management -> time_unit_config(release_time_new);
	
	// The start time of current(new) instance
	start_time = time_management -> time_unit_config(start_time_new);  
	pre_eval_time = start_time; cout << "pre_eval_time updating (2)" << endl;
	
	rel_dline = time_management -> time_unit_config(Deadline);
	abs_dline = release_time + rel_dline;

	time_management -> update_cur_time(start_time);
//--------------------------------------------------------------------------------//
#ifdef DEBUG
	cout << "==================================================================" << endl;
	printf("#	Release time: %.05f ns, Start time: %.05f ns\r\n", release_time, start_time);
	printf("#	max: %.02f MHz, min: %.02f MHz, Default speed: %.02f MHz\r\n", max_freq_t, min_freq_t, sys_clk -> cur_freq);
	printf("#	Jitter constraint: BCET + (WCET - BCET) * %.02f%\r\n", jitter_config.alpha * 100);
	cout << "==================================================================" << endl;
	cout << "Start -> " << endl; cout << "current time: " << sys_clk -> cur_time << "ns" << endl;
#endif
	//for(cur_index = 0; exe_path[case_id][cur_index] != 0x7FFFFFFF; cur_index++) {
	for(cur_index = 0; exe_path[case_id][cur_index] != exe_path[case_id].back(); cur_index++) {
#ifdef DEBUG
		cout << "Block_" << CFG_path[ exe_path[case_id][cur_index] - 1 ].get_index() << " -> ";
#endif
//cout << "4 case_t: " << case_t << endl;
if(
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() > CFG_path.back().get_index() || 
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() < CFG_path.front().get_index()
) {
	printf("Current traversed Basic Block: %d, which is beyond the scope of this task's CFG\r\n", CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index());
	exit(1);
}	
		cycles_cnt += CFG_path[ exe_path[case_id][cur_index] - 1 ].get_cycles(case_t);

//	cout << "3 case_t: " << case_t << endl;
if(
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() > CFG_path.back().get_index() || 
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() < CFG_path.front().get_index()
) {
	printf("Current traversed Basic Block: %d, which is beyond the scope of this task's CFG\r\n", CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index());
	exit(1);
}	
		time_temp = time_management -> time_unit_config(
			// us -> ns
			(1000 * CFG_path[ exe_path[case_id][cur_index] - 1 ].get_cycles(case_t)) / time_management -> sys_clk -> cur_freq
		); 
		time_management -> update_cur_time(time_temp + sys_clk -> cur_time);	
		power_eval();

	if(
		sys_mode != (int) NORMAL 
	) {
		// Invoking the operation of B-type checkpoint
		if(CFG_path[ exe_path[case_id][cur_index] - 1 ].B_checkpoint_en != 0x7FFFFFFF) { 
			B_Intra_task_checkpoint(
				exe_path[case_id][cur_index],    // Cast current Basic Block ID 
				exe_path[case_id][cur_index + 1] // Cast its successive Basic Block ID according to the indicated execution path case
			);		
		}
		// Invoking the operation of L-type checkpoint
		else if(CFG_path[ exe_path[case_id][cur_index] - 1 ].L_checkpoint_en[0] != 0x7FFFFFFF) {
			L_Intra_task_checkpoint(
				exe_path[case_id][cur_index],    // Cast current Basic Block ID 
				exe_path[case_id][cur_index + 1] // Cast its successive Basic Block ID according to the indicated execution path case
			);
		}			
		// Invoking the operation of P-type checkpoint
		else if(CFG_path[ exe_path[case_id][cur_index] - 1 ].P_checkpoint_en != 0x7FFFFFFF) { 
			P_Intra_task_checkpoint(
				exe_path[case_id][cur_index],    // Cast current Basic Block ID 
				exe_path[case_id][cur_index + 1] // Cast its successive Basic Block ID according to the indicated execution path case
			);		
		}
		else {
		}	
	}	
		cout << "current time: " << sys_clk -> cur_time << "ns" << endl;

	}
#ifdef DEBUG
		cout << "Block_" << CFG_path[ exe_path[case_id][cur_index] - 1 ].get_index() << " -> ";
#endif
//		cout << "2 case_t: " << case_t << endl;
if(
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() > CFG_path.back().get_index() || 
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() < CFG_path.front().get_index()
) {
	printf("Current traversed Basic Block: %d, which is beyond the scope of this task's CFG\r\n", CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index());
	exit(1);
}	
		cycles_cnt += CFG_path[ exe_path[case_id][cur_index] - 1 ].get_cycles(case_t);

//		cout << "1 case_t: " << case_t << endl;
if(
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() > CFG_path.back().get_index() || 
	CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index() < CFG_path.front().get_index()
) {
	printf("Current traversed Basic Block: %d, which is beyond the scope of this task's CFG\r\n", CFG_path[ exe_path[cur_case_id][cur_block_index] - 1 ].get_index());
	exit(1);
}	
		time_temp = time_management -> time_unit_config(
			// us -> ns
			(1000 * CFG_path[ exe_path[case_id][cur_index] - 1 ].get_cycles(case_t)) / time_management -> sys_clk -> cur_freq
		); 
		time_management -> update_cur_time(time_temp + time_management -> sys_clk -> cur_time);		
		power_eval();


#ifdef DEBUG
	cout << "End at " << sys_clk -> cur_time << "ns" << endl << endl;
#endif
	
	power_eval();
	global_param_eval();
	completion_config();
}

void Src_CFG::constraint_update(void)
{
	double bcet_limit, wcet_limit, middle_t;

	if(sys_mode == (int) NORMAL) {
		jitter_config.alpha = 0.0;
	}
	else if(sys_mode == (int) L_JITTER) {
		jitter_config.alpha = in_alpha;
#ifdef DEBUG	
		cout << "constraint_update => alpha: " << jitter_config.alpha << endl;
#endif
	}
	else if(sys_mode == (int) H_PREDICT) {
		jitter_config.alpha = in_alpha;
#ifdef DEBUG	
		cout << "constraint_update => alpha: " << jitter_config.alpha << endl;
#endif
	}
	else if(sys_mode == (int) L_POWER) {
		jitter_config.alpha = 1.0;
	}
#ifdef DEBUG	
       cout << "constraint_update => Input value of alpha: " << in_alpha << endl;
       cout << "constraint_update => jitter_config.alpha: " << jitter_config.alpha << "%" << endl
	    << "constraint_update => bcet_limit: " << bcet_limit << "%" << endl
	    << "constraint_update => wcet_limit: " << wcet_limit << "%" << endl
	    << "constraint_update => jitter_config.alpha: " << jitter_config.alpha << "%" << endl; 
#endif
}
 
void Src_CFG::jitter_init(void)
{
	// Jitter constraint setting
#ifdef DEBUG
	printf("jitter_init => Tsk_%d.wcrt: %.05f ns\r\n", TskID, wcrt);
	printf("jitter_init => Tsk_%d.bcrt: %.05f ns\r\n", TskID, bcrt);
#endif	
	// Define Jitter Margin
	constant_delay = bcet + (wcrt - wcet); // to be conservative, so using BCET + worst-case interference instead of real BCRT 
	var_delay = wcrt - constant_delay;
	jitter_config.fin_jitter_bound = var_delay;
	constraint_update();
	if(sys_mode == (int) L_JITTER)
		jitter_config.fin_time_target = constant_delay + jitter_config.fin_jitter_bound * jitter_config.alpha;	
	else if(sys_mode == (int) H_PREDICT) { 
		jitter_config.fin_time_target = constant_delay + jitter_config.fin_jitter_bound * jitter_config.alpha;	
		//jitter_config.fin_time_target = average_response;//(response_case.size() == 0) ? 0.0 : response_case.back();
		//cout << "average_response: " << average_response << ", fin_time_target: " << jitter_config.fin_time_target << endl;	
	}
	else if(sys_mode == (int) L_POWER) {
		jitter_config.fin_time_target = wcrt;
	}
	else { // sys_mode == (int) NORMAL
		jitter_config.fin_time_target = 0.0;
	}
}

void Src_CFG::SysMode_reconfig(int SysMode) 
{
	sys_mode = SysMode;
#ifdef DEBUG
	cout << "SysMode_in:" << SysMode << ", SysMode:" << sys_mode << endl;
#endif	
	jitter_init();
#ifdef DEBUG
	printf("Constant Delay: %fns, Variable Delay: %fns\r\n", constant_delay, var_delay);
	printf("Jitter Margin: %fns\r\n", jitter_config.fin_jitter_bound);
#endif
}
void Src_CFG::show_TskMode(void)
{
	cout << "TskID:" << TskID << "is defined as ";
	if(sys_mode == (int) NORMAL) cout << "NORMAL mode, no DVFS ability" << endl; 
	else if(sys_mode == (int) H_PREDICT) cout << "High Predictability mode" << endl; 
	else if(sys_mode == (int) L_POWER) cout << "Low-Power mode" << endl; 
	else if(sys_mode == (int) L_JITTER) cout << "Low Absolute Finish Time Jitter  mode" << endl; 
	else {
		cout << "Wrong setting of this task's mode" << endl;
		exit(1);
	}
}
void Src_CFG::exe_speed_scaling(double new_speed)
{
#ifdef  PROCESSOR_AM335x
        bool freq_verify;
        freq_verify = false;
	for(int i = 0; i < 5; i++) {
		if(new_speed == freq_vol[i][0]) {
			freq_verify = true;
			break;
		}
	}
	if(freq_verify == false) {
		cout << "Wrong configuration of operating frequency, processor TI-AM335x cannot perform " << new_speed << " MHz" << endl;
		exit(1); 
	}	
#endif
	time_management -> cur_freq_config(new_speed);
	// us -> ns
	ISR_TIME_SLICE = ((1000 * INST_UNIT) / time_management -> sys_clk -> cur_freq);

	
#ifdef DVFS_OVERHEAD
// Conducting Transition Overhead
	if(dvfs_en == (int) DVFSOverhead_sim && sys_mode != (int) NORMAL) {
		time_management -> update_cur_time(
			sys_clk -> cur_time +  // The base of current time
			((double) OverheadTime) // The overhead of frequency-voltage scaling
		);	
		//cout << "power eval 5" << endl;
		// overhead_power_eval();
	}
	//energy_ref = (dvfs_en == (int) NonDVFS_sim) ? energy_acc : 
	//	     (dvfs_en == (int) DVFS_sim   ) ? energy_ref : energy_ref_overhead;
	
	// Accumulate the energy consumption with previous VF transition period
	//power_eval();
	time_management -> ExecutedTime_Accumulator((unsigned char) FREQ_SCALING_POINT, (int) TskID);
#endif

#ifdef DEBUG
	printf("Current speed: %.02f MHz\r\n\r\n", time_management -> sys_clk -> cur_freq);
	printf("The period of interrupt timer has been changed to %.08f ns\r\n", ISR_TIME_SLICE);	
#endif
}

double Src_CFG::get_cur_speed(void)
{
	return time_management -> sys_clk -> cur_freq;
}

void Src_CFG::B_Intra_task_checkpoint(int cur_block_index, int succ_block_index)
{
	double new_freq;
	double rep_time_target, resp_time_act;
/*For debugging*/	double elapsed_time; 
	double executed_time, time_available, local_deadline;
	int rwcec; // Remaining worst-case execution cycles from current basic block
	int branch_addr = CFG_path[cur_block_index - 1].B_checkpoint_en;
	double time_temp;
	int temp;
	local_deadline = wcrt + release_time - time_management -> sys_clk -> cur_time;
//===============================================================================================================//
// Look up the remaining worst-case exeuction cycles (RWCEC) from B-type mining table
	// Identify the actual branch
	if( succ_block_index == B_mining_table[branch_addr].successors[0] ) {
		rwcec = B_mining_table[branch_addr].n_taken_rwcec; // Point to Branch instruction's address, i.e., certain index of mining table
		rem_wcec = rwcec;
#ifdef DEBUG
		cout << endl << "not taken" << endl;
#endif
	}
	else {
		rwcec = B_mining_table[branch_addr].taken_rwcec;
		rem_wcec = rwcec;
#ifdef DEBUG
		cout << endl << "taken" << endl;
#endif
	}
#ifdef DEBUG
	printf("rwcec = %d, ", rwcec); 
#endif
//===============================================================================================================//
/*For debugging*/	elapsed_time = time_management -> sys_clk -> cur_time - release_time; 
        rep_time_target = jitter_config.fin_time_target;	
	executed_time = time_management -> ExecutedTime[TskID] + (time_management -> sys_clk -> cur_time - time_management -> UpdatePoint);
	time_available = rep_time_target - (wcrt - wcet) - executed_time;
	resp_time_act = executed_time + ((1000 * rwcec) / time_management -> sys_clk -> cur_freq) + interference;
//===============================================================================================================//
#ifdef DEBUG
		printf("cur_block: %d, succ_block: %d\r\n", cur_block_index, succ_block_index);
		printf("Current time: %f us, ", sys_clk -> cur_time);
		printf("Elapsed time = %f us, Executed Time = %f us, Target Response Time = %f us\r\n", 
					elapsed_time, 
					executed_time,
					rep_time_target 
		);
#endif
	if(sys_mode == (int) L_JITTER) {
#ifdef DEBUG
		printf("Act_RT: %fns, min_resp: %fns, max_resp: %fns, interference: %fns", resp_time_act, min_response, max_response, interference);
#endif	
		if(resp_time_act < min_response) {
			// MHz -> GHz
			double f_new_ub = (1000 * rwcec) / (min_response - executed_time - interference);
			new_freq = (f_new_ub < 0.0) ? max_freq_t : f_new_ub;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : 
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) LOWER_BOUND);
			}
#endif
		}
		else if(resp_time_act > max_response) {
			// MHz -> GHz
			double f_new_lb = (1000 * rwcec) / (max_response - executed_time - interference);
			new_freq = (f_new_lb < 0.0) ? max_freq_t : f_new_lb;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : 
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) UPPER_BOUND);
			}
#endif
		}
		else {
			new_freq = time_management -> sys_clk -> cur_freq;
		}
	}
	else {
#ifdef DEBUG
		printf("Available Time for Task_%d: %f us, Remaining Execution time until WCRT: %f us\r\n", TskID, time_available, local_deadline);
#endif	
		if(rep_time_target <= ((wcrt - wcet) + executed_time)) {
			new_freq = max_freq_t; 
		}
		else {
			new_freq = (time_available <= local_deadline) ?  (1000 * rwcec) / time_available : // Remaining time until WCRT won't lead to deadline miss
									 (1000 * rwcec) / local_deadline; // Deadline miss might occur
			// Just in case, if available time is still negative, changing the target to make actual response time fit the WCRT
			if(time_available < 0) new_freq = (1000 * rwcec) / local_deadline;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : new_freq;
		}
#ifdef DISCRETE_DVFS
		if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
			printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
			new_freq = discrete_handle(new_freq, rwcec, time_available, local_deadline);
		}
#endif
	}
#ifdef DVFS_OVERHEAD
// Conducting Transition Overhead
	if(dvfs_en == (int) DVFSOverhead_sim) {
		temp = (int) OverheadCycle_B;
		cycles_cnt += temp;
		time_temp = time_management -> time_unit_config(
			// us -> ns
			(1000 * temp) / time_management -> sys_clk -> cur_freq
		);
		time_management -> update_cur_time(
			sys_clk -> cur_time + // The base of current time
			time_temp             // The overhead of checkpoint operation
		);		
		energy_acc += ((double) OverheadEnergy);
	}
#endif
#ifdef DEBUG
	if(new_freq != time_management -> sys_clk -> cur_freq) {
		char *dvfs_msg = new char[200];
		sprintf(dvfs_msg, "echo \"TskID: %d, inst.: %d, B-ch: %.01f MHz -> %.01f MHz\" >> Bch_Tsk%d.sim_%d.txt",
			TskID,
			response_case.size(),
			time_management -> sys_clk -> cur_freq,
			new_freq,
			TskID,
			sim_mode
		);
		system(dvfs_msg);
		delete [] dvfs_msg;
	}
#endif
	exe_speed_scaling(new_freq);
}

void Src_CFG::L_Intra_task_checkpoint(int cur_block_index, int succ_block_index)
{
	double new_freq;
	double rep_time_target, resp_time_act;
/*For debugging*/	double elapsed_time; 	
	double executed_time, time_available, local_deadline;
	int rwcec; // Remaining worst-case execution cycles from current basic block
	int loop_index = CFG_path[cur_block_index - 1].L_checkpoint_en[0];
	int loop_addr  = CFG_path[cur_block_index - 1].L_checkpoint_en[1];
	double time_temp;
	int temp;
	local_deadline = wcrt + release_time - time_management -> sys_clk -> cur_time;
//===============================================================================================================//
// Look up the remaining worst-case exeuction cycles (RWCEC) from L-type mining table
	// Accumulate L-type iteration counter by -1 if task reached loop's exit currently
	if(cur_block_index == L_mining_table[loop_index].loop_entry) 
		L_loop_iteration.at(loop_index) = 
			((L_loop_iteration[loop_index] - 1) == -1) ? 0 : L_loop_iteration[loop_index] - 1;
#ifdef DEBUG
	// Identify the actual branch
	cout << endl << "taken_succ[" << loop_addr << "]: " << L_mining_table[loop_index].taken_succ[loop_addr] << endl;
	cout << "not-taken_succ[" << loop_addr << "]: " << L_mining_table[loop_index].n_taken_succ[loop_addr] << endl;
#endif

	if(L_loop_iteration[loop_index] == 0) 
		rwcec = L_mining_table[loop_index].succ_rwcec;
	else if( succ_block_index == L_mining_table[loop_index].taken_succ[loop_addr]) {
		rwcec = L_mining_table[loop_index].taken_rwcec[loop_addr] + 
			(L_loop_iteration[loop_index] - 1) * P_mining_table[loop_index].iteration_wcec +  
			L_mining_table[loop_index].succ_rwcec; 
#ifdef DEBUG
		cout << endl << "taken\t"; 
		cout << "remaining iteration is (" << loop_index << ")(" << cur_case_id << "): " << L_loop_iteration[loop_index] << endl;
#endif
	}
	else {
		rwcec = L_mining_table[loop_index].n_taken_rwcec[loop_addr] + 
			(L_loop_iteration[loop_index] - 1) * P_mining_table[loop_index].iteration_wcec +  
			L_mining_table[loop_index].succ_rwcec; 
#ifdef DEBUG
		cout << endl << "not taken\t"; 
		cout << "remaining iteration is (" << loop_index << ")(" << cur_case_id << "): " << L_loop_iteration[loop_index] << endl;
#endif
	}
	rem_wcec = rwcec;
#ifdef DEBUG
	printf("rwcec = %d, ", rwcec);
#endif
//===============================================================================================================//
/*For debugging*/	elapsed_time = time_management -> sys_clk -> cur_time - release_time; 
        rep_time_target = jitter_config.fin_time_target;	
	executed_time = time_management -> ExecutedTime[TskID] + (time_management -> sys_clk -> cur_time - time_management -> UpdatePoint);
	time_available = rep_time_target - (wcrt - wcet) - executed_time;// + slack;  
	resp_time_act = executed_time + ((1000 * rwcec) / time_management -> sys_clk -> cur_freq) + interference;
//===============================================================================================================//
#ifdef DEBUG
		printf("cur_block: %d, succ_block: %d\r\n", cur_block_index, succ_block_index);
		printf("Current time: %f ns, ", sys_clk -> cur_time);
		printf("Elapsed time = %f ns, Executed Time = %f ns, Target Response Time = %f ns\r\n", 
					elapsed_time, 
					executed_time,
					rep_time_target 
		);
#endif
	if(sys_mode == (int) L_JITTER) {
#ifdef DEBUG
		printf("Act_RT: %fns, min_resp: %fns, max_resp: %fns, interference: %fns", resp_time_act, min_response, max_response, interference);
#endif	
		if(resp_time_act < min_response) {
			// MHz -> GHz
			double f_new_ub = (1000 * rwcec) / (min_response - executed_time - interference);
			new_freq = (f_new_ub < 0.0) ? max_freq_t : f_new_ub;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : 
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) LOWER_BOUND);
			}
#endif
		}
		else if(resp_time_act > max_response) {
			// MHz -> GHz
			double f_new_lb = (1000 * rwcec) / (max_response - executed_time - interference);
			new_freq = (f_new_lb < 0.0) ? max_freq_t : f_new_lb;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : 
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) UPPER_BOUND);
			}
#endif
		}
		else {
			new_freq = time_management -> sys_clk -> cur_freq;
		}
	}
	else {
#ifdef DEBUG
		printf("Available Time for Task_%d: %f ns, Remaining Execution time until WCRT: %f ns\r\n", TskID, time_available, local_deadline);
#endif	
		if(rep_time_target <= ((wcrt - wcet) + executed_time)) {
			new_freq = max_freq_t; 
		}
		else {
			new_freq = (time_available <= local_deadline) ?  (1000 * rwcec) / time_available : // Remaining time until WCRT won't lead to deadline miss
									 (1000 * rwcec) / local_deadline; // Deadline miss might occur
			// Just in case, if available time is still negative, changing the target to make actual response time fit the WCRT
			if(time_available < 0) new_freq = (1000 * rwcec) / local_deadline;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : new_freq;
		}
#ifdef DISCRETE_DVFS
		if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
			printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
			new_freq = discrete_handle(new_freq, rwcec, time_available, local_deadline);
		}
#endif
	}
#ifdef DVFS_OVERHEAD
// Conducting Transition Overhead
	if(dvfs_en == (int) DVFSOverhead_sim) {
		temp = (int) OverheadCycle_B;
		cycles_cnt += temp;
		time_temp = time_management -> time_unit_config(
			// us -> ns
			(1000 * temp) / time_management -> sys_clk -> cur_freq
		);
		time_management -> update_cur_time(
			sys_clk -> cur_time + // The base of current time
			time_temp             // The overhead of checkpoint operation
		);		
		energy_acc += ((double) OverheadEnergy);
	}
#endif
#ifdef DEBUG
	if(new_freq != time_management -> sys_clk -> cur_freq) {
		char *dvfs_msg = new char[200];
		sprintf(dvfs_msg, "echo \"TskID: %d, inst.: %d, L-ch: %.01f MHz -> %.01f MHz\" >> Lch_Tsk%d.sim_%d.txt",
			TskID,
			response_case.size(),
			time_management -> sys_clk -> cur_freq,
			new_freq,
			TskID,
			sim_mode
		);
		system(dvfs_msg);
		delete [] dvfs_msg;
	}
#endif
	exe_speed_scaling(new_freq);
}

void Src_CFG::P_Intra_task_checkpoint(int cur_block_index, int succ_block_index)
{
	double new_freq;
	double rep_time_target, resp_time_act;
/*For debugging*/	double elapsed_time; 
	double executed_time, time_available, local_deadline;
	int rwcec; // Remaining worst-case execution cycles from current basic block
	int loop_addr  = CFG_path[cur_block_index - 1].P_checkpoint_en;
	double time_temp;
	int temp;
	local_deadline = wcrt + release_time - time_management -> sys_clk -> cur_time;
//===============================================================================================================//
// Look up the remaining worst-case exeuction cycles (RWCEC) from P-type mining table
	// Since actual loop iteration(s) have been known ahead of task really reach that loop in the near future,
	// the calculation of the remaining worst-case execution cycles is: RWCEC = iteration x Iteration_WCEC + WCEC_after_Loop
	rwcec = P_loop_LaIteration[loop_addr][cur_case_id] * P_mining_table[loop_addr].iteration_wcec + P_mining_table[loop_addr].succ_rwcec; 
	rem_wcec = rwcec;
#ifdef DEBUG
	cout << endl << endl << "============================================" << endl;
	printf("P-type checkpoint: \r\nblock_%d -> block_%d\r\nrwcec = %d(actual loop iteration: %d)\r\n\r\n, ", cur_block_index, succ_block_index, rwcec, P_loop_LaIteration[loop_addr][cur_case_id]); 
	cout << endl << endl << "============================================" << endl;
#endif
//===============================================================================================================//
/*For debugging*/	elapsed_time = time_management -> sys_clk -> cur_time - release_time; 
        rep_time_target = jitter_config.fin_time_target;	
	executed_time = time_management -> ExecutedTime[TskID] + (time_management -> sys_clk -> cur_time - time_management -> UpdatePoint);
#ifdef DEBUG
	printf("Task_%d, before checkpoint, executed_time:%f ns, updatePoint: %f ns\r\n", TskID, time_management -> ExecutedTime[TskID], time_management -> UpdatePoint);
#endif
	time_available = rep_time_target - (wcrt - wcet) - executed_time;  
	resp_time_act = executed_time + ((1000 * rwcec) / time_management -> sys_clk -> cur_freq) + interference;
//===============================================================================================================//
#ifdef DEBUG
		printf("cur_block: %d, succ_block: %d\r\n", cur_block_index, succ_block_index);
		printf("Current time: %f ns, ", sys_clk -> cur_time);
		printf("Elapsed time = %f ns, Executed Time = %f ns, Target Response Time = %f ns\r\n", 
					elapsed_time, 
					executed_time,
					rep_time_target 
		);
#endif
	if(sys_mode == (int) L_JITTER) {
#ifdef DEBUG
		printf("Act_RT: %fns, min_resp: %fns, max_resp: %fns, interference: %fns", resp_time_act, min_response, max_response, interference);
#endif	
		if(resp_time_act < min_response) {
			// MHz -> GHz
			double f_new_ub = (1000 * rwcec) / (min_response - executed_time - interference);
			new_freq = (f_new_ub < 0.0) ? max_freq_t : f_new_ub;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t :
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) LOWER_BOUND);
			}
#endif
		}
		else if(resp_time_act > max_response) {
			// MHz -> GHz
			double f_new_lb = (1000 * rwcec) / (max_response - executed_time - interference);
			new_freq = (f_new_lb < 0.0) ? max_freq_t : f_new_lb;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : 
				   (new_freq < 0.0       ) ? max_freq_t : new_freq; // To avoid R_max < (executed_time + Interference)
#ifdef DEBUG
			printf(", new_freq: %f MHz\r\n", new_freq);
#endif
#ifdef DISCRETE_DVFS
			if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
				printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
				new_freq = discrete_handle_SelectBound(new_freq, (int) UPPER_BOUND);
			}
#endif
		}
		else {
			new_freq = time_management -> sys_clk -> cur_freq;
		}
	}
	else {
#ifdef DEBUG
		printf("Availavle Time is: %f - (%f - %f) - %f\r\n", rep_time_target, wcrt, wcet, executed_time);
		printf("Available Time for Task_%d: %f ns, Remaining Execution time until WCRT: %f ns\r\n", TskID, time_available, local_deadline);
#endif	
		if(rep_time_target <= ((wcrt - wcet) + executed_time)) {
			new_freq = max_freq_t; 
		}
		else {
			new_freq = (time_available <= local_deadline) ?  (1000 * rwcec) / time_available : // Remaining time until WCRT won't lead to deadline miss
									 (1000 * rwcec) / local_deadline; // Deadline miss might occur
			// Just in case, if available time is still negative, changing the target to make actual response time fit the WCRT
			if(time_available < 0) new_freq = (1000 * rwcec) / local_deadline;
			new_freq = (new_freq > max_freq_t) ? max_freq_t : 
			           (new_freq < min_freq_t) ? min_freq_t : new_freq;
		}
#ifdef DISCRETE_DVFS
		if(new_freq != min_freq_t && new_freq != max_freq_t) {
#ifdef DEBUG
			printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
#endif	
			new_freq = discrete_handle(new_freq, rwcec, time_available, local_deadline);
		}
#endif
	}

#ifdef DVFS_OVERHEAD
// Conducting Transition Overhead
	if(dvfs_en == (int) DVFSOverhead_sim) {
		temp = (int) OverheadCycle_B;
		cycles_cnt += temp;
		time_temp = time_management -> time_unit_config(
			// us -> ns
			(1000 * temp) / time_management -> sys_clk -> cur_freq
		);
		time_management -> update_cur_time(
			sys_clk -> cur_time + // The base of current time
			time_temp             // The overhead of checkpoint operation
		);		
		energy_acc += ((double) OverheadEnergy); 
	}
#endif
#ifdef DEBUG
	if(new_freq != time_management -> sys_clk -> cur_freq) {
		char *dvfs_msg = new char[200];
		sprintf(dvfs_msg, "echo \"TskID: %d, inst.: %d, P-ch: %.01f MHz -> %.01f MHz\" >> Pch_Tsk%d.sim_%d.txt",
			TskID,
			response_case.size(),
			time_management -> sys_clk -> cur_freq,
			new_freq,
			TskID,
			sim_mode
		);
		system(dvfs_msg);
		delete [] dvfs_msg;
	}
#endif
	exe_speed_scaling(new_freq);
}

double Src_CFG::discrete_handle(double new_freq, int rwcec, double AvailableTime, double local_deadline)
{
	int i;
	double max_diff, min_diff;

	if(new_freq <= min_freq_t) return min_freq_t;
	else if(new_freq >= max_freq_t) return max_freq_t;
	else {	
		for(i = 0; new_freq >= freq_vol[i+1][0]; i++);
#ifdef DEBUG
		printf("Discrete Bound: %.02f MHz, %.02f MHz, %.02f MHz\r\n", freq_vol[i][0], new_freq, freq_vol[i+1][0]);
#endif
		// us -> ns
		min_diff = ((1000 * rwcec) / freq_vol[i][0]) - AvailableTime;
		max_diff = AvailableTime - ((1000 * rwcec)  / freq_vol[i+1][0]);
		//return (new_freq >= max_freq_t) ? max_freq_t : 
		//       (new_freq <= min_freq_t) ? min_freq_t : freq_vol[i+1][0];
		//    us -> ns
		//       return (min_diff < max_diff && (((1000 * rwcec) / freq_vol[i][0]) <= local_deadline)) ? freq_vol[i][0] : freq_vol[i+1][0];	
		return freq_vol[i+1][0];
	}
}

double Src_CFG::discrete_handle_SelectBound(double new_freq, int sel_bound) {
	int i;
	if(new_freq <= min_freq_t) return min_freq_t;
	else if(new_freq >= max_freq_t) return max_freq_t;
	else {	
		for(i = 0; new_freq >= freq_vol[i+1][0]; i++);
		return (sel_bound == (int) UPPER_BOUND) ? freq_vol[i+1][0] : freq_vol[i][0];
	}
}

/**
  * @brief Execution traces obtained via a cycle-level simulation.

  * @procedure
	   1) Assign task's worst-/average-/best-case execution cycles
	   2) Configure data structure of  remaining execution cycles for each defined checkpoint
	   3) Configure data structure of each B-/L-/P-type checkpoints' numbers
**/
void Src_CFG::exe_cycle_tracing(exeTime_info WCET_INFO, RWCEC_Trace_in *cycle_trace_temp, checkpoint_num *checkpointNum_temp)
{
	// Procedure_1: assign task's worst-/average-/best-case execution cycles
	execution_cycles[WORST - 1]   = WCET_INFO[0]; cout << "WCEC: " << execution_cycles[WORST -1] << endl;
	execution_cycles[AVERAGE - 1] = WCET_INFO[1]; cout << "ACEC: " << execution_cycles[AVERAGE - 1] << endl;
	execution_cycles[BEST - 1]    = WCET_INFO[2]; cout << "BCEC: " << execution_cycles[BEST - 1] << endl;

	// Procedure_2: configure data structure of  remaining execution cycles for each defined checkpoint
	cycle_trace_in = cycle_trace_temp;

	// Procedure_3: configure data structure of each B-/L-/P-type checkpoints' numbers
	checkpointNum = checkpointNum_temp;
}

void Src_CFG::mining_table_gen(void)
{
	unsigned int B_checkpoints_cnt, L_loops_cnt, L_checkpoints_cnt,  P_checkpoints_cnt;

// Creating the correspoinding miniing table in order to record the remaining worst-case execution cycles at each basic block (especially every branch)
// For B-type checkpoints
	B_checkpoints_cnt = checkpointLabel -> B_checkpoints.size();
	for(unsigned int i = 0; i < B_checkpoints_cnt; i++) 
		B_mining_table.push_back( 
			(B_mining_table_t) 
			{
				checkpointLabel -> B_checkpoints[i],        // Block ID
				cycle_trace_in  -> B_RWCEC_t[i][NOT_TAKEN], // RWCEC if it's not taken
				cycle_trace_in  -> B_RWCEC_t[i][TAKEN],     // RWCEC if it's taken  
				cycle_trace_in  -> B_RWCEC_t[i][0],         // Successor 1 
				cycle_trace_in  -> B_RWCEC_t[i][2]          // Successor 2
			} 
		);
	
#ifdef DEBUG
	cout << "B-Type Mining Table" << endl;
	cout << "------------------------------------------------" << endl;
	cout << "|WCEC(cycle): " << execution_cycles[WORST - 1] << "\t\t\t\t|" << endl;
	cout << " -----------------------------------------------" << endl;
	cout << "| Address\t|" << "\t\tRWCEC(cycle)\t|" << endl;
	cout << "\t\t -------------------------------" << endl;
	cout << "|\t\t|\t" << "n_taken\t|\ttaken\t|" << endl;
	cout << " -----------------------------------------------" << endl;
	for(unsigned int i = 0; i < B_checkpoints_cnt; i++) {
		cout << "| Branch_" 
		     << i << "(Block " << B_mining_table[i].block_id << ")" << "\t|\t" 
		     << B_mining_table[i].n_taken_rwcec 
		     << "(" << B_mining_table[i].successors[0] 
		     << ")\t|\t" 
		     << B_mining_table[i].taken_rwcec 
		     << "(" << B_mining_table[i].successors[1]
	             << ")\t|" << endl; 
		cout << " -----------------------------------------------" << endl;	
	}
	cout << endl;
#endif 	
// For L-type checkpoints
	L_loops_cnt = checkpointLabel -> L_checkpoints.size();
	for(unsigned int i = 0; i < L_loops_cnt; i++) { // The #th Loop-nest
		L_mining_table_t *loop_t = new L_mining_table_t;
		loop_t -> loop_entry = checkpointLabel -> L_checkpoints[i];       // Block ID of Loop Entry
		loop_t -> loop_bound = cycle_trace_in -> L_RWCEC_t[i].loop_bound; // Loop Bound
		loop_t -> succ_rwcec = cycle_trace_in -> L_RWCEC_t[i].rwcec_AfterLoop; // The RWCEC after loop
		L_checkpoints_cnt = cycle_trace_in -> L_RWCEC_t[i].branch_num;
		for(unsigned int j = 0; j < L_checkpoints_cnt; j++) { // The checkpoint/branch inside #th Loop-nest
			loop_t -> block_id.push_back     (cycle_trace_in -> L_RWCEC_t[i].branch[j][0]); // The current branch's Block ID
			loop_t -> taken_succ.push_back   (cycle_trace_in -> L_RWCEC_t[i].branch[j][1]); // The successor_1's Block ID
			loop_t -> taken_rwcec.push_back  (cycle_trace_in -> L_RWCEC_t[i].branch[j][2]); // The WCEC from successor_1 until loop exit/entry
			loop_t -> n_taken_succ.push_back (cycle_trace_in -> L_RWCEC_t[i].branch[j][3]); // The successor_2's Block ID
			loop_t -> n_taken_rwcec.push_back(cycle_trace_in -> L_RWCEC_t[i].branch[j][4]); // The WCEC from successor_2 until loop exit/entry
		}
		L_mining_table.push_back(*loop_t);
		delete loop_t;
	}
#ifdef DEBUG
for(unsigned int i = 0; i < L_loops_cnt; i++) {	
	L_checkpoints_cnt = cycle_trace_in -> L_RWCEC_t[i].branch_num;
	cout << "L-Type Mining Table_" << i + 1 << endl;
	cout << "------------------------------------------------" << endl;
	cout << "Loop Entry/Exit(Basic Block): " << L_mining_table[i].loop_entry << endl;
	cout << "Loop Bound(iteration): " << L_mining_table[i].loop_bound << endl;
	cout << "The Remaining Worst-Case Execution Cycle(cycle): " << L_mining_table[i].succ_rwcec << endl;
	for( unsigned int j = 0; j < L_checkpoints_cnt; j++ ) {
		cout << "Address_" << j + 1 << "(Block_" << L_mining_table[i].block_id[j] << "): " << endl; 
		cout << "Taken(Block_" << L_mining_table[i].taken_succ[j] << ") =>  WCEC: " << L_mining_table[i].taken_rwcec[j] << " cycle(s)" << endl;
		cout << "Not-Taken(Block_" << L_mining_table[i].n_taken_succ[j] << ") =>  WCEC: " << L_mining_table[i].n_taken_rwcec[j] << " cycle(s)" << endl;
	}
	cout << "------------------------------------------------" << endl;
	cout << endl << endl;
}
#endif 	

// For P-type checkpoints
	P_checkpoints_cnt = checkpointLabel -> P_checkpoints.size();
	for(unsigned int i = 0; i < P_checkpoints_cnt; i++) {
		bool dummy = false;
		for(unsigned int j = 0; j < checkpointLabel -> P_loop_dummy.size(); j++) {
		  dummy = (checkpointLabel -> P_checkpoints[i] == checkpointLabel -> P_loop_dummy[j]) ? true : false;
		}
		if(dummy == false) 
		 P_mining_table.push_back( 
	 		 (P_mining_table_t) 
 			 {
				 checkpointLabel -> P_checkpoints[i], // Block ID
				 cycle_trace_in  -> P_RWCEC_t[i][0],  // Loop Bound
				 cycle_trace_in  -> P_RWCEC_t[i][1],  // WCEC within each iteration 
				 cycle_trace_in  -> P_RWCEC_t[i][2]   // Remaining worst-case execution cycles after loop
			 } 
		 );
	}
#ifdef DEBUG
	cout << "P-Type Mining Table" << endl;
	cout << " ------------------------------------------------------------------------------" << endl;
	cout << "|WCEC(cycle): " << execution_cycles[WORST - 1] << "\t\t\t\t|" << endl;
	cout << " ------------------------------------------------------------------------------" << endl;
	cout << "| Address\t|" << "\tLoop Bound\t|\tWCEC of each iteratoin\t|\tWCEC apart from Loop\t|" << endl;
	cout << " ------------------------------------------------------------------------------" << endl;
	for(unsigned int i = 0; i < P_checkpoints_cnt; i++) {
		cout << "| Loop_" 
		     << i << "(Block " << P_mining_table[i].block_id << ")" << "\t|\t" 
		     << P_mining_table[i].loop_bound 
		     << "\t|\t" 
		     << P_mining_table[i].iteration_wcec 
		     << "\t|\t" 
		     << P_mining_table[i].succ_rwcec 
	             << "\t|" << endl; 
		cout << " ------------------------------------------------------------------------------" << endl;
	}
#endif
	// Fill dummy values for getting memory space
	for(unsigned int i = 0; i < L_loop_iteration_preload.size(); i++)
	  L_loop_iteration.push_back(0);
}
		
/**
  * @brief According to the given checkpoint labels from input file,
	   enabling certain basic blocks as B-/L-/P-type checkpoints
	   which have been designated as checkpoints.

  * @procedure
	   1) Preload/setup value of the given loop bound to each L- and P-type checkpoints' counter
	   2) Enable the corresponding Basic Block as B-/L-/P-type checkpoint
**/
void Src_CFG::checkpoints_placement(checkpoints_label *&checkpoint_label_temp)
{	
	unsigned int B_cnt, L_cnt, P_cnt;
	checkpointLabel = checkpoint_label_temp;
	B_cnt = checkpointLabel -> B_checkpoints.size();
	L_cnt = checkpointLabel -> L_checkpoints.size();
	P_cnt = checkpointLabel -> P_checkpoints.size();
	
	// Preload/setup the counter of L- and P-type iteration
	//for(unsigned int i = 0; i < L_cnt; i++) L_loop_iteration.push_back(checkpointLabel -> L_loop_bound[i]);
	for(unsigned int i = 0; i < P_cnt; i++) P_loop_iteration.push_back(checkpointLabel -> P_loop_bound[i]);
	
	// Enable the corresponding Basic Block as B-/L-/P-type checkpoint
	for(unsigned int i = 0; i < B_cnt; i++ ) CFG_path[ checkpointLabel -> B_checkpoints[i] - 1 ].B_checkpoint_en = i; 
	for(unsigned int i = 0; i < L_cnt; i++ ) {
	 for(unsigned int j = 0; j <  (unsigned int) (cycle_trace_in -> L_RWCEC_t[i].branch_num); j++) {
	  CFG_path[cycle_trace_in -> L_RWCEC_t[i].branch[j][0] - 1].L_checkpoint_en[0] = i;
	  CFG_path[cycle_trace_in -> L_RWCEC_t[i].branch[j][0] - 1].L_checkpoint_en[1] = j;
	 }
	}
	for(unsigned int i = 0; i < P_cnt; i++ ) {
		bool dummy = false;
		for(unsigned int j = 0; j < checkpointLabel -> P_loop_dummy.size(); j++) {
		  dummy = (checkpointLabel -> P_checkpoints[i] == checkpointLabel -> P_loop_dummy[j]) ? true : false;
		}
		if(dummy == false) CFG_path[ checkpointLabel -> P_checkpoints[i] - 1 ].P_checkpoint_en = i; 
	}
	
/*
	for( index_temp = 0; index_temp < L_loop_cnt; index_temp++ ) {
		L_cnt = checkpoint_label_temp.L_checkpoints[index_temp].size();
		L_checkpoints.push_back( checkpoint_label_temp.L_checkpoints[index_temp] );
		for(i = 0; i < L_cnt; i++) {
			CFG_path[ checkpoint_label_temp.L_checkpoints[index_temp][i] - 1 ].L_checkpoint_en[0] = index_temp;
			CFG_path[ checkpoint_label_temp.L_checkpoints[index_temp][i] - 1 ].L_checkpoint_en[1] = i;
		}
	 	L_loop_iteration.push_back(checkpoint_label_temp.L_loop_iteration[index_temp]);
		L_loop_exit.push_back(checkpoint_label_temp.L_checkpoints[index_temp].front()); 
	}
*/
}
