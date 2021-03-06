//#include "../inc/inter_bus.h"
#include "../inc/sched.h"
#include "../inc/main.h"

/*class Task_State_Bus {
	private:
		vector<task_info_t> &inter_tasks;
		vector<Src_CFG> &intra_tasks;
		Time_Management *time_management;

	public:
		void release_time_update(int src_inter, int des_intra); // Updating release time for its corrsponding Intra-task CFG
		void start_time_update(int src_inter, int des_intra); // Updating start time for its corrsponding Intra-task CFG
		
};*/

Task_State_Bus::Task_State_Bus(Time_Management *&timer, vector<task_info_t> *src_inter, vector<Src_CFG> *src_intra)
{
	time_management = timer;

	inter_tasks = *src_inter;
	intra_tasks = *src_intra;
	
	for(int i = 0; i < inter_tasks.size(); i++) {
		intra_tasks[i].mining_table_gen();
		cout << "task_" << i << ":" << endl;
		cout << "Release time: " << inter_tasks[i].release_time << endl;
		cout << "Priority: " << inter_tasks[i].prt << endl;
		cout << "Relative Deadline: " << inter_tasks[i].rel_dline << endl;
		cout << "WCET: " << inter_tasks[i].wcet << endl;
		cout << "Period: " << inter_tasks[i].period << endl;
		cout << "Operating State: " << (int) inter_tasks[i].state << endl;
	}	
}

Task_State_Bus::~Task_State_Bus(void)
{

}

void Task_State_Bus::scheduling_point_assign(int task_id, int case_t, char dvfs_en)
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
	float release_time_new, 
	float start_time_new, 
	float Deadline, 
	char DVFS_en
)
{
	intra_tasks[new_task_id].cycles_cnt = 0;	
	intra_tasks[new_task_id].dvfs_en = DVFS_en;
	intra_tasks[new_task_id].exe_speed_config();
	
//--------------------------------------------------------------------------------//
// Setting the release time, start time, absolute deadline and relative deadline
	// The release time of current(new) instance 
	intra_tasks[new_task_id].release_time  = time_management -> time_unit_config(release_time_new);
	
	// The start time of current(new) instance
	intra_tasks[new_task_id].start_time = time_management -> sys_clk -> cur_time;// time_management -> time_unit_config(start_time_new);  
	intra_tasks[new_task_id].pre_eval_time = intra_tasks[new_task_id].start_time;
	
	intra_tasks[new_task_id].rel_dline = time_management -> time_unit_config(Deadline);
	intra_tasks[new_task_id].abs_dline = intra_tasks[new_task_id].release_time + intra_tasks[new_task_id].rel_dline;

	time_management -> update_cur_time(intra_tasks[new_task_id].start_time);

//--------------------------------------------------------------------------------//
#ifndef DEBUG
	cout << "==================================================================" << endl;
	printf("#	Release time: %.05f us, Start time: %.05f us\r\n", 
				intra_tasks[new_task_id].release_time, 
				intra_tasks[new_task_id].start_time
	);
	printf("#	max: %.02f MHz, min: %.02f MHz, Default speed: %.02f MHz\r\n", 
				intra_tasks[new_task_id].max_freq_t, 
				intra_tasks[new_task_id].min_freq_t, 
				time_management -> sys_clk -> cur_freq
	);
	printf("#	Jitter constraint: BCET + (WCET - BCET) * %.02f%\r\n", intra_tasks[new_task_id].jitter_config.alpha * 100);
	cout << "==================================================================" << endl;
	cout << "Start -> " << endl; cout << "current time: " << time_management -> sys_clk -> cur_time << endl;
#endif

	intra_tasks[new_task_id].executed_cycles = 0;
	intra_tasks[new_task_id].cur_case_id = case_id;
}

void Task_State_Bus::time_driven_cfg(int new_task_id)
{
	float time_temp;
	int case_id = intra_tasks[new_task_id].cur_case_id;
	int cur_block_index = intra_tasks[new_task_id].cur_block_index;

	if(intra_tasks[new_task_id].exe_path[case_id][cur_block_index] != 0x7FFFFFFF) {
#ifdef DEBUG
		cout << "Block_" << intra_tasks[new_task_id].CFG_path[ 
						intra_tasks[new_task_id].exe_path[case_id][cur_block_index] - 1 
						].get_index() << " -> ";
#endif
		intra_tasks[new_task_id].context_reg = intra_tasks[new_task_id].isr_driven_cfg((int) WORST, (char) DVFS_ENABLE);
		time_temp = time_management -> time_unit_config(
			intra_tasks[new_task_id].context_reg.act_exe_time
		); 
		intra_tasks[new_task_id].cycles_cnt += intra_tasks[new_task_id].context_reg.act_cycles; 
		printf("\r\n#actual Execution cycles withing this period of interrupt timer: %d cycles\r\n", 
			intra_tasks[new_task_id].context_reg.act_cycles
		);
		printf("#Actual Execution time within this period of interrupt timer: %.05f us\r\n", time_temp);
		time_management -> update_cur_time(time_temp + time_management -> sys_clk -> cur_time);
		printf("Current Time: %.05f us\r\n", time_management -> sys_clk -> cur_time);
		intra_tasks[new_task_id].power_eval();
	}
	else {
#ifdef DEBUG
	cout << "End" << endl << endl;
#endif
		intra_tasks[new_task_id].power_eval();
		intra_tasks[new_task_id].global_param_eval();
		intra_tasks[new_task_id].completion_config();
	}
}
