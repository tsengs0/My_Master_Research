#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include "../inc/sched.h"
#include "../inc/dvfs_info.h"
#include "../inc/main.h"

#ifdef INTRA_SCHED
	extern Task_State_Bus *inter_intra_bus;
#endif

//extern int instance_case[2][3];
//extern int instance_index[2];
//extern int tasks_num;
extern int sim_mode;

using std::cout;
using std::cin;
using std::endl;
using std::vector;

Ready_Queue::Ready_Queue(void)
{
	front = NULL; //front -> pre_task = NULL; front -> next_task = NULL;
	rear = NULL; //rear -> pre_task = NULL; rear -> next_task = NULL;
	ptr = NULL; //ptr -> pre_task = NULL; ptr -> next_task = NULL;
}

Ready_Queue::~Ready_Queue(void)
{

}

bool Ready_Queue::IsEmpty(void)
{
	return (front == NULL) ? true : false;
}

int Ready_Queue::get_front(void)
{
	return front -> task_id;
}

int Ready_Queue::get_rear(void)
{
	return rear -> task_id;
}

void Ready_Queue::push(int &new_val)
{
	if(IsEmpty()) {
		front = new task_element_t();
		front -> task_id = new_val;
		front -> pre_task = NULL;
		front -> next_task = NULL;
		rear = front;
		cnt += 1;
#ifdef DEBUG
		cout << "Add first job: Task_" << front -> task_id << " to Read Queue" << endl;
#endif
	}
	else {
		ptr = new task_element_t();
		ptr -> task_id = new_val;
		ptr -> pre_task = NULL;
		ptr -> next_task = front;
		front -> pre_task = ptr;
		front = ptr;
		ptr = NULL;
		cnt += 1;
#ifdef DEBUG
		cout << "Add new job: Task_" << front -> task_id << " to Read Queue" << endl;
#endif
	}
}

/*
	param:
		1) new_val: the new item adding into queue
		2) pre_task: new item's neigbouring element
		3) location: put new item after of before pre_task
*/
void Ready_Queue::insert(int new_val, task_element_t *&pre_task, char location)
{
/*if(pre_task -> pre_task == NULL) cout << "NULL -> " << pre_task -> task_id << " -> ";
else cout << pre_task -> pre_task -> task_id << " -> " << pre_task -> task_id << " -> ";
if(pre_task -> next_task == NULL) cout << "NULL" << endl;
else cout << pre_task -> next_task -> task_id << endl; 
*/
	ptr = new task_element_t();
	ptr -> task_id = new_val;
	if(pre_task == front && location == (char) BEFORE) {
		ptr -> next_task = front;
		ptr -> pre_task = NULL;
		front -> pre_task = ptr;
	}
	else if(pre_task == front && location == (char) AFTER) {
#ifdef DEBUG	
		if(pre_task -> pre_task == NULL && pre_task -> next_task == NULL) cout << "Only one, " << pre_task -> task_id << endl;
#endif
		ptr -> next_task = front -> next_task;
		ptr -> pre_task =  front;
		front -> next_task = ptr;
		if(ptr -> next_task == NULL) {rear = ptr; /*cout << "rear -> task_id: " << rear -> task_id << endl;*/}
		else  ptr -> next_task -> pre_task = ptr; 
	}
	else if(pre_task != front && location == (char) BEFORE){
		ptr -> next_task = pre_task;
		ptr -> pre_task = pre_task -> pre_task;
		pre_task -> pre_task = ptr;
		if(pre_task != front) ptr -> pre_task -> next_task = ptr; 
	}
	else if(pre_task != front && location == (char) AFTER){
		ptr -> next_task = pre_task -> next_task;
		ptr -> pre_task = pre_task;
		pre_task -> next_task = ptr;
		if(pre_task != rear) ptr -> next_task -> pre_task = ptr; 
	}

	// Re-locating the Front-pointer
	ptr = front;
	while(ptr -> pre_task != NULL) {ptr = ptr -> pre_task; /*cout << "Relocating Front" << endl;*/}
	front = ptr;
	
	// Re-locating the Rear-pointer
	ptr = rear;
	while(ptr -> next_task != NULL){ ptr = ptr -> next_task; /*cout << "ptr -> task_id: " << ptr -> task_id << endl;*/}
	rear = ptr;
	ptr = NULL;

#ifdef DEBUG
cout << endl << endl; list_sched_point();
	cout << "Insert job: Task_" << new_val << endl;
#endif
}

int Ready_Queue::pop(void)
{
	int temp;

	if(rear == front) {
		temp = rear -> task_id;
		delete front;
		front = NULL;
		rear = NULL;
	}
	else {
		ptr = rear -> pre_task;
		temp = ptr -> next_task -> task_id;
		delete rear;
		rear = ptr;
		rear -> next_task = NULL;
		//ptr = NULL;
	}
#ifdef DEBUG
	cout << "pop task_" << temp << endl;
#endif
	return temp;
}

void Ready_Queue::list_sched_point(void)
{
	ptr = rear;

	if(ptr == NULL) cout << "Empty in the Ready Queue" << endl;
	else if(front == rear) cout << "Rear and Front: Task_" << ptr -> task_id << endl;
	else {
		cout << "Rear:  Task_" << ptr -> task_id << endl; ptr = ptr -> pre_task;
		while(ptr != front) {
			cout << "Task_" << ptr -> task_id << endl;
			ptr = ptr -> pre_task;
		}
		cout << "Front: Task_" << ptr -> task_id << endl;
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////
double ceiling(double x)
{
	return   ((x - (int) x) > 0) ? (int) (x + 1) : (int) x;
}

RT_Analyser::RT_Analyser(task_info_t *tasks_list) 
{
	tasks = tasks_list;	
}

/**
 *  @brief Calculating the worst case response time of certain task by recursive function
 *  @param task_id: the task which is demanded to be analysed
 *  @param wcrt_cur: passing the WCRT of previous iteration
**/
double RT_Analyser::RM_WCRT(int task_id, double wcrt_pre)		
{
	double wcrt_t;
	wcrt_t = tasks[task_id].wcet;
	for(int i = 0; i < tasks_num; i++) {
		//Task's priority is greater than analysed task's and current iteration is not equal to indicated task index
		if(tasks[i].prt < tasks[task_id].prt && i != task_id)
			wcrt_t += ceiling(wcrt_pre / tasks[i].period) * tasks[i].wcet; 
	}
	if(wcrt_t == wcrt_pre) return wcrt_t;
	return RM_WCRT(task_id, wcrt_t);
}

/**
 *  @brief Calculating the worst case response time of certain task by recursive function
 *  @param task_id: the task which is demanded to be analysed
 *  @param wcrt_cur: passing the BCRT of previous iteration
**/
double RT_Analyser::RM_BCRT(int task_id, double bcrt_pre)		
{
	double bcrt_t;
	bcrt_t = tasks[task_id].bcet;
	for(int i = 0; i < tasks_num; i++) {
		//Task's priority is greater than analysed task's and current iteration is not equal to indicated task index
		if(tasks[i].prt < tasks[task_id].prt && i != task_id)
			bcrt_t += (ceiling(bcrt_pre / tasks[i].period) - 0) * tasks[i].bcet; 
	}
	if(bcrt_t == bcrt_pre) return bcrt_t;
	return RM_BCRT(task_id, bcrt_t);
}

double gcd(double a, double b)
{
	double tmp;
	while(a) {
		tmp = a; 
		a = fmod(b, a); 
		b = tmp;
	}
	return b;
}

double lcm(double a, double b)
{
	return a / gcd(a, b) * b;
}

Task_Scheduler::Task_Scheduler(Time_Management *timer, task_info_t *tasks, Ready_Queue *queue, char policy, Task_State_Bus *msg_bus)
{
	time_management = timer;
	task_list = tasks;
	ready_queue = queue;
	sched_policy = policy;
	running_task_id = (int) CPU_IDLE;
	pre_task = running_task_id;
	new_task_start_flag = false;
	inter_intra_bus = msg_bus;
	rwcet = (double) 0.0;
	cur_context.task_id = (int) NO_PREEMPTION;
	cur_context.rwcet = (double) 0.0;
	cur_context.isr_flag = false;	
	slack = (double) 0.0;

	// To calculate the Hyperperiod
	double period_temp[tasks_num];
	for(int i = 0; i < tasks_num; i++) period_temp[i] = (double) task_list[i].period;
	hyperperiod = (double) std::accumulate(period_temp + 1, period_temp + tasks_num, period_temp[0], lcm);
	NRT_nearest = (double) hyperperiod; // Initial value;
	
	// Creating Response-Time-Analyser module
	rta = new RT_Analyser(task_list);
	for(int i = 0; i < tasks_num; i++) {
		double wcrt_t = rta -> RM_WCRT(i, task_list[i].wcrt);
		double bcrt_t = rta -> RM_BCRT(i, wcrt_t); // According to the RTA, the BCRT in the first iteration is input its WCRT
		task_list[i].wcrt = wcrt_t;
		task_list[i].bcrt = bcrt_t;
		inter_intra_bus -> intra_tasks[i].wcrt = wcrt_t;
		inter_intra_bus -> intra_tasks[i].bcrt = bcrt_t;
#ifdef DEBUG
		cout << "Worst-Case Response Time (WCRT) of Task_" 
		     << i << ": " 
		     << inter_intra_bus -> intra_tasks[i].wcrt << " ns" << endl;
		cout << "Best-Case Response Time (BCRT) of Task_" 
		     << i << ": " 
		     << inter_intra_bus -> intra_tasks[i].bcrt << " ns" << endl;
#endif
	}
	for(int i = 0; i < tasks_num; i++) {
		acrt[i] = task_list[i].wcrt;
		acet[i] = (double) 0.0;
		acet_acc[i] = (double) 0.0;
		// Analyse the worst-case interference time for every task 
		worst_interference[i] = (double) task_list[i].wcrt - task_list[i].wcet;
		avg_interference[i] = worst_interference[i];
	} 

	// Initially, all tasks' state are set to 'IDLE'
	for(int i = 0; i < tasks_num; i++) task_list[i].state = (char) IDLE;
	
	SimPattern_cnt = new int[tasks_num];
	for(int i = 0; i < tasks_num; i++) SimPattern_cnt[i] = 0;
}

Task_Scheduler::~Task_Scheduler(void)
{

}

void Task_Scheduler::resume(void)
{
	int TskID = cur_context.task_id;
	isr_stack.pop((Preemption_Stack*) (&cur_context));
	rwcet = cur_context.rwcet;
	double rwcec_t = (int) (inter_intra_bus -> intra_tasks[TskID].rem_wcec);
	//time_management -> cur_freq_config(cur_context.cur_freq);
	inter_intra_bus -> intra_tasks[running_task_id].exe_speed_scaling(cur_context.cur_freq);
	time_management -> ExecutedTime_Accumulator((unsigned char) RESUME_POINT, (int) TskID);
/*
#ifdef AVERAGE_INTERFERENCE_EVAL	
	double interference_temp = avg_interference[TskID];
#else
	double interference_temp = inter_intra_bus -> intra_tasks[TskID].interference - slack;
#endif
*/
	double interference_temp;
	if(inter_intra_bus -> intra_tasks[TskID].sys_mode == (int) L_JITTER)
		interference_temp = avg_interference[TskID];
	else
		interference_temp = inter_intra_bus -> intra_tasks[TskID].interference - slack;
		
	// In the mean time, the interference from Intra-task context also need to be synchronised referring to just updated speculative/actual interference time
	inter_intra_bus -> intra_tasks[TskID].interference = interference_temp;

// Before resuming back to certain task's execution, check if previously preemption task left some slack time
	if(
		(inter_intra_bus -> intra_tasks[TskID].sys_mode != (int) NORMAL && inter_intra_bus -> intra_tasks[TskID].sys_mode != (int) L_JITTER) ||
		(inter_intra_bus -> intra_tasks[TskID].sys_mode == (int) L_JITTER && inter_intra_bus -> intra_tasks[TskID].response_case.size() >= (unsigned int) EVAL_CNT_START)
	) { 
		if(slack < 0.0) {
#ifdef DEBUG
			cout << "slack: " << slack << endl;	
			cout << "Wrong Calculation, slack may not be negative" << endl;
#endif
			exit(1);
		}
		else if(slack == 0.0) {/*cout << "Previously preemption task didn't leave any slack time" << endl;*/}
		else {
			if(
				// inter_intra_bus -> intra_tasks[TskID].sys_mode == (int) H_PREDICT || 
				 inter_intra_bus -> intra_tasks[TskID].sys_mode == (int) L_JITTER   
				// inter_intra_bus -> intra_tasks[TskID].sys_mode == (int) L_POWER  
			) {
				double resp_time_act = time_management -> ExecutedTime[TskID] + ((1000 * rwcec_t) / time_management -> sys_clk -> cur_freq) + interference_temp;
				double new_freq;
				if(resp_time_act < inter_intra_bus -> intra_tasks[TskID].min_response) {
					double f_new_ub = rwcec_t / (inter_intra_bus -> intra_tasks[TskID].min_response - time_management -> ExecutedTime[TskID] - interference_temp);
					new_freq = f_new_ub;
					new_freq = (new_freq > (double) MAX_speed) ? (double) MAX_speed : 
					           (new_freq < (double) MIN_speed) ? (double) MIN_speed : new_freq;
#ifdef DISCRETE_DVFS
					if(new_freq != (double) MAX_speed && new_freq != (double) MIN_speed) {
						//printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
						new_freq = inter_intra_bus -> intra_tasks[TskID].discrete_handle_SelectBound(new_freq, (int) LOWER_BOUND);
					}
#endif
				}
				else if(resp_time_act > inter_intra_bus -> intra_tasks[TskID].max_response) {
					double f_new_lb = rwcec_t / (inter_intra_bus -> intra_tasks[TskID].max_response - time_management -> ExecutedTime[TskID] - interference_temp);
					new_freq = f_new_lb;
					new_freq = (new_freq > (double) MAX_speed) ? (double) MAX_speed : 
					           (new_freq < (double) MIN_speed) ? (double) MIN_speed : new_freq;
#ifdef DISCRETE_DVFS
					if(new_freq != (double) MAX_speed && new_freq != (double) MIN_speed) {
						//printf("Before Discrete Bound Handling: %f MHz, ", new_freq);
						new_freq = inter_intra_bus -> intra_tasks[TskID].discrete_handle_SelectBound(new_freq, (int) UPPER_BOUND);
					}
#endif
				}
				else {
					new_freq = time_management -> sys_clk -> cur_freq;
				}
				//inter_intra_bus -> intra_tasks[TskID].exe_speed_scaling(new_freq);
#ifdef DEBUG
				char *msg_slack = new char[300];
//================================================================================================================================================================================================================//
// Export specific log file containing different types of interference time, i.e., 1) worst-case interference time and 2) average-case interference time
#ifdef AVERAGE_INTERFERENCE_EVAL	
				sprintf(msg_slack, "echo \"(Resume Point) curTime: %.05fns, TskID: %d, Slack:%.05fns, RWCEC: %d cycle(s), ExecutedTime: %fns, Interference: %f, PrevFreq: %.05fMHz, UpdateFreq: %.05fMHz\" >> slack_reclaim.resume.sim_%d.txt", 
						time_management -> sys_clk -> cur_time,
						cur_context.task_id,
						slack, 
						(int) rwcec_t,
						(double) time_management -> ExecutedTime[TskID],
						(double) interference_temp,
						cur_context.cur_freq, 
						new_freq,
						sim_mode
				);
#else
				sprintf(msg_slack, "echo \"(Resume Point) curTime: %.05fns, TskID: %d, Slack:%.05fns, RWCEC: %d cycle(s), ExecutedTime: %fns, Interference: %f - %f = %f, PrevFreq: %.05fMHz, UpdateFreq: %.05fMHz\" >> slack_reclaim.resume.sim_%d.txt", 
						time_management -> sys_clk -> cur_time,
						cur_context.task_id,
						slack, 
						(int) rwcec_t,
						(double) time_management -> ExecutedTime[TskID],
						(double) worst_interference[TskID],
						(double) slack,
						(double) interference_temp,
						cur_context.cur_freq, 
						new_freq,
						sim_mode
				);
#endif
//================================================================================================================================================================================================================//
				system(msg_slack);
				delete [] msg_slack;
#endif
			}

			else {
#ifdef DEBUG
				cout << "Previously preemption task completed its work earlier than its WCRT, thus remaining " << slack << " ns of slack time" << endl;	
#endif
				double TimeAvailable_init = inter_intra_bus -> intra_tasks[TskID].jitter_config.fin_time_target - 
							   interference_temp - 
							   time_management -> ExecutedTime[TskID];
				double new_freq = inter_intra_bus -> intra_tasks[TskID].discrete_handle(
								// GHz -> MHz
								((1000 * rwcec_t) / TimeAvailable_init), 
								(int) rwcec_t, 
								(double) TimeAvailable_init,
								(inter_intra_bus -> intra_tasks[TskID].wcrt + inter_intra_bus -> intra_tasks[TskID].release_time - time_management -> sys_clk -> cur_time)
				);
				//inter_intra_bus -> intra_tasks[TskID].exe_speed_scaling(new_freq);
#ifdef DEBUG		
				char *msg_slack = new char[300];
				sprintf(msg_slack, "echo \"(Resume Point) curTime: %.05fns, TskID: %d, Slack:%.05fns, RWCEC: %d cycle(s), ExecutedTime: %fns, AvailableTime: %fns, PrevFreq: %.05fMHz, UpdateFreq: %.05fMHz\" >> slack_reclaim.resume.sim_%d.txt", 
						time_management -> sys_clk -> cur_time,
						cur_context.task_id,
						slack, 
						(int) rwcec_t,
						(double) time_management -> ExecutedTime[TskID],
						(double) TimeAvailable_init,
						cur_context.cur_freq, 
						default_freq_new,
						sim_mode
				);
				system(msg_slack);
				delete [] msg_slack;
#endif
			}
		}
	}

//--------------------------------------------------------------------------------//
#ifdef DEBUG
	cout << "Resume Task_" << cur_context.task_id << "'s execution(RWCET:" << rwcet << " ns) " << "from Task_" << running_task_id << endl;
	list_task_state();
//====================================================================================================//
	char *msg_resume = new char[200];
	sprintf(msg_resume, "echo \"(Resume Point) curTime: %.05fns, resume Task_%d from Task_%d\" >> resume.invoke.sim_%d.txt", 
			time_management -> sys_clk -> cur_time,
			cur_context.task_id,
			running_task_id,
			sim_mode
	);
	system(msg_resume);
	delete [] msg_resume;
//====================================================================================================//
#endif
}

/*
 Since it is just a simulation, so the preemption will be behaved as a recursion,
 and the behaviour of contex switch in terms of simulation will be:
 1) Intentially put some delay
 2) Starting running another/new task with the highest priority
*/
void Task_Scheduler::context_switch(int cur_task, int new_task)
{
	bool sched_verify = true;
	// (1) delay()
	// (2) Src_CFG[new_task].traverse_spec_path()
	
	// Fill current context into Preemption Stack
	cur_context.task_id = cur_task;
	cur_context.rwcet = rwcet;
	cur_context.cur_freq = time_management -> show_cur_freq();
	cur_context.isr_flag = true;	
	isr_stack.push(cur_context);
	
	rwcet = task_list[new_task].wcet;
	pre_task = (int) CPU_IDLE;
/*
	// Suspending the running task temporarily, and putting it into the rear of Ready Queue
	task_list[cur_task].state = (char) READY;
	ready_queue -> insert(cur_task, ready_queue -> rear, (char) AFTER);

	if(sched_policy == (char) RM) sched_verify = RM_sched(cur_task);
	else if(sched_policy == (char) EDF) sched_verify = EDF_sched(cur_task);
*/
#ifdef DEBUG
	cout << "Preemption!" << endl;
	list_task_state();
//====================================================================================================//
	char *msg_preemption = new char[200];
	sprintf(msg_preemption, "echo \"(Preemption Point) curTime: %.05fns, Preempt Task_%d by Task_%d\" >> preemption.invoke.sim_%d.txt", 
			time_management -> sys_clk -> cur_time,
			cur_task,
			new_task,
			sim_mode
	);
	system(msg_preemption);
	delete [] msg_preemption;
//====================================================================================================//
#endif

}

Preemption_Stack::Preemption_Stack(void)
{
	top = NULL;
	bottom = NULL;
	ptr = NULL;
	stack_cnt = 0;
}

Preemption_Stack::~Preemption_Stack(void)
{
	stack_cnt = 0;
/*	if(top == NULL) delete top;
	if(bottom == NULL) delete bottom;
	if(ptr == NULL) delete ptr;
*/
}

void Preemption_Stack::push(context_t in)
{
	ptr = new context_t;
	ptr -> task_id = in.task_id;
	ptr -> rwcet = in.rwcet;
	ptr -> cur_freq = in.cur_freq;
	ptr -> isr_flag = in.isr_flag;

	if(bottom == NULL) {
		if(top != NULL) {
#ifdef DEBUG
			cout << "Wrong Stack Manipulation" << endl;
			cout << "Since Bottom is pointing to the NULL, so Top also should be located in NULL" << endl;
#endif	
			exit(1);
		}
		else {
			top = ptr; bottom = ptr;
			bottom -> next = NULL;
		}
	}
	else {
		ptr -> next = top;
		top = ptr;
	}
	stack_cnt += 1;
}

void Preemption_Stack::pop(void *inout)
{
	context_t *out = (context_t*) inout;

	if(bottom == NULL) {
		if(top != NULL) {
#ifdef DEBUG
			cout << "Wrong Stack Manipulation" << endl;
			cout << "Since Bottom is pointing to the NULL, so Top also should be located in NULL" << endl;
#endif	
			exit(1);
		}
		else {
#ifdef DEBUG
			cout << "You use Preemption Stack at improper time" << endl;
			cout << "Because there is no context inside Preemption Stack" << endl;
#endif	
			exit(1);
		}
	}
	else {
		out -> task_id  = top -> task_id;
		out -> rwcet    = top -> rwcet; // unit: ns		
		out -> cur_freq = top -> cur_freq;
		out -> isr_flag = top -> isr_flag;
		top -> isr_flag = false; // Free the Preemption Flag of current task's context
		out -> next     = NULL;
	}
	stack_cnt -= 1;
	ptr = top;
	if(top != bottom) top = top -> next;
	else top = NULL;	
	delete ptr;
}

void Preemption_Stack::stack_list(void)
{
	ptr = top;
	cout << "Top(Input/Output) -> ...... -> Bottom" << endl;
	for(int i = 0; i < stack_cnt; i++) {
		cout << "Task_" << ptr -> task_id << " -> ";
		if(i != stack_cnt - 1) ptr = ptr -> next;
		else cout << "NULL" << endl;
	}
}

bool Preemption_Stack::IsEmpty(void)
{
	return (top == NULL) ? true : false;
}

int Preemption_Stack::show_StackCnt(void)
{
	return stack_cnt;
}

/**
  * @brief Reload some parameter for next instance while current task just completed its exeuction (computation of current instance) 
**/   
void Task_Scheduler::completion_config(void)
{
	// Update actual executed time of currently completed task
	time_management -> ExecutedTime_Accumulator((unsigned char) COMPLETION_TIME, (int) running_task_id);
	
	// Synchronise the ACET adn ACRT between Task Scheduler and CFG-base simulator
	acet_acc[running_task_id] += time_management -> ExecutedTime[running_task_id];
	acet[running_task_id] = (double) (acet_acc[running_task_id] / SimPattern_cnt[running_task_id]);
	acrt[running_task_id] = inter_intra_bus -> intra_tasks[running_task_id].average_response;

	// Update interference time in average 
	avg_interference[running_task_id] = acrt[running_task_id] - acet[running_task_id];

#ifdef DEBUG	
	printf("TskID: %d, ACRT: %fns, ACET: %fns, ACRT - ACET = average-case interference: %fns\r\n", running_task_id, acrt[running_task_id], acet[running_task_id], avg_interference[running_task_id]);
	char *msg_interference = new char[200];
	sprintf(msg_interference, "echo \"Tsk_%d, (inst.: %d), ACRT: %fns, ACET: %fns, Interference: %fns\" >> AvgInterference.Tsk_%d.sim_%d.txt", 
		running_task_id,
		inter_intra_bus -> intra_tasks[running_task_id].response_case.size(),
		acrt[running_task_id],
		acet[running_task_id],
		avg_interference[running_task_id],
		running_task_id,
		sim_mode
	);
	system(msg_interference);
	delete [] msg_interference;
#endif
//====================================================================================================================================================================//
/*
 *  If take Average-case as reference/guideline
 *
*/
#ifdef AVERAGE_INTERFERENCE_EVAL
		// Calculate the slack time arose from the difference between WCRT and actual response time
		slack = (ready_queue -> IsEmpty() == true) ? (double) 0.0 :  // Since no body will come at this time, so such slack time cannot be utilised by any task
							     acrt[running_task_id] - 
							     (time_management -> sys_clk -> cur_time - task_list[running_task_id].release_time);
#ifdef DEBUG	
		cout << "ACRT - (curTime - Release): " 
		     << acrt[running_task_id] << " - (" << time_management -> sys_clk -> cur_time
		     << " - " << task_list[running_task_id].release_time << ") = "  << slack << endl;
#endif

#else
/*
 *  If take Worst-case as reference/guideline
 *
*/
		// Calculate the slack time arose from the difference between WCRT and actual response time
		slack = (ready_queue -> IsEmpty() == true) ? (double) 0.0 :  // Since no body will come at this time, so such slack time cannot be utilised by any task
							     task_list[running_task_id].wcrt - 
							     (time_management -> sys_clk -> cur_time - task_list[running_task_id].release_time);
#ifdef DEBUG	
		cout << "WCRT - (curTime - Release): " 
		     << task_list[running_task_id].wcrt << " - (" << time_management -> sys_clk -> cur_time
		     << " - " << task_list[running_task_id].release_time << ") = "  << slack << endl;
#endif

#endif
//====================================================================================================================================================================//

	// Since the latency from interrupt detection timer, even if the actual response time exceeded WCRT a little bit but still no deadline occur
	// In this case, we still regard it as acceptable result 
	if(slack < 0 && time_management -> sys_clk -> cur_time <= task_list[running_task_id].release_time + task_list[running_task_id].rel_dline) {
		slack = (double) 0.0;
	}
	else if(slack > task_list[running_task_id].wcrt || slack < 0) {
		cout << "Therefore, wrong calculation" << endl;
		exit(1);
	}
	task_list[running_task_id].state = (char) IDLE;
	task_list[running_task_id].release_time += task_list[running_task_id].period;
	inter_intra_bus -> intra_tasks[running_task_id].completion_flag = false;
	task_list[running_task_id].NRT_USED = false;
	task_list[running_task_id].completion_cnt += 1;

//	if(sim_mode != (int) NonDVFS_sim && running_task_id != 0 && inter_intra_bus -> intra_tasks[running_task_id].response_case.size() == (unsigned int) EVAL_CNT_START) 
//		inter_intra_bus -> intra_tasks[running_task_id].SysMode_reconfig((int) L_JITTER); 	
	
	if(isr_stack.IsEmpty() == false) {
		resume(); //isr_stack.stack_list(); 
#ifdef DEBUG
		cout << "isr stack is not empty, therefore resume" << endl;
#endif	
		/*if(isr_stack.show_StackCnt() == 0) {
			cur_context.task_id = (int) NO_PREEMPTION;
			cur_context.rwcet = (double) 0.0;
			cur_context.isr_flag = false;
			cout << "Flush Preemption Stack" << endl;
			//isr_stack.stack_list();
		}*/
	}
	else {
		cur_context.task_id = (int) NO_PREEMPTION;
		cur_context.rwcet = (double) 0.0;
		cur_context.isr_flag = false;
#ifdef DEBUG
		cout << "Flush all redundant element(s) out of ISR Stack" << endl;
#endif
		//isr_stack.stack_list();
	}
#ifdef DEBUG
	cout << endl << "Task_" << running_task_id << " complete at " << time_management -> sys_clk -> cur_time << " ns" << endl;
	cout << "Task_" << running_task_id << "'s next release time: " << task_list[running_task_id].release_time << " ns" << endl;
#endif
	running_task_id = (int) CPU_IDLE;
}

void Task_Scheduler::sched_arbitration(double sched_tick)
{
	bool sched_verify = true;
	//cout << "d" << endl;
	// The completion of currently running task
#ifdef INTRA_SCHED
	if(IsIdle() != true && inter_intra_bus -> intra_tasks[running_task_id].completion_flag == true) {
		completion_config();
	}
	else if(IsIdle() != true && inter_intra_bus -> intra_tasks[running_task_id].completion_flag == false) {
		// us -> ns
		rwcet = (1000 * inter_intra_bus -> intra_tasks[running_task_id].rem_wcec) / (time_management -> sys_clk -> cur_freq); // Conservative estimation
	}
#else
	if(IsIdle() != true && rwcet == 1) {
		task_list[running_task_id].state = (char) IDLE;
		task_list[running_task_id].release_time += task_list[running_task_id].period;
		running_task_id = (int) CPU_IDLE;
	}
	else if(IsIdle() != true && rwcet != 1) {
		 rwcet -= 1;
	}
#endif
	// Filling the arrival task in Ready Queue
	// In the real processor, we only need to deal with the parameter(index of arrival task, etc.) given from interruption,
	// instead of travesing whole task set to see whether there is a new request for executing a certain task
	for(int i = 0; i < tasks_num; i++) {
		if(
			task_list[i].release_time <= time_management -> sys_clk -> cur_time &&
			task_list[i].NRT_USED == false
		) {
			 
			//time_management -> update_cur_time(sched_tick);
			task_list[i].NRT_USED = true;
#ifdef DEBUG
			cout << "Task_" << i << " release at " << sched_tick << "ns" << endl;
#endif		
			if(IsIdle() == true && ready_queue -> IsEmpty() == true) {
#ifdef DEBUG
				//cout << "Idle and Empty-queue: " << i << endl;
#endif
				task_list[i].state = (char) READY;
				ready_queue -> push(i);
				//pre_task = running_task_id;
			}
			else if(IsIdle() == true && ready_queue -> IsEmpty() != true) {
#ifdef DEBUG
				//cout << "Idle and non-Empty-queue: " << i << endl;
#endif
				task_list[i].state = (char) READY;
				//pre_task = running_task_id;

				if(sched_policy == (char) RM) sched_verify = RM_sched(i);
				else if(sched_policy == (char) EDF) sched_verify = EDF_sched(i);
			}
			else if(IsIdle() != true && ready_queue -> IsEmpty() == true) {
#ifdef DEBUG
				//cout << "non-Idle and Empty-queue" << endl;
#endif
				task_list[i].state = (char) READY;

				// Suspending the running task temporarily, and putting it into the rear of Ready Queue
				task_list[running_task_id].state = (char) READY;
				ready_queue -> push(running_task_id);
				pre_task = running_task_id;
				running_task_id = (int) CPU_IDLE;

				if(sched_policy == (char) RM) sched_verify = RM_sched(i);
				else if(sched_policy == (char) EDF) sched_verify = EDF_sched(i);
			}
			else{ // IsIdle() != true && ready_queue -> IsEmpty() == false
#ifdef DEBUG
				//cout << "non-Idle: " << i << endl;
#endif
				task_list[i].state = (char) READY;

				// Suspending the running task temporarily, and putting it into the rear of Ready Queue
				task_list[running_task_id].state = (char) READY;
				
				ready_queue -> insert(running_task_id, ready_queue -> rear, (char) AFTER);
				pre_task = running_task_id;
				running_task_id = (int) CPU_IDLE;
				
				if(sched_policy == (char) RM) sched_verify = RM_sched(i);
				else if(sched_policy == (char) EDF) sched_verify = EDF_sched(i);
			}
		}
	}
	if(IsIdle() == true && ready_queue -> IsEmpty() != true) dispatcher();
}

bool Task_Scheduler::RM_sched(int &task_new)
{
	task_element_t *ptr = ready_queue -> rear;
	// Rearranging the order of tasks basing on their priorities
	while(1) {
		if(ptr == ready_queue -> front) break;
		else if(task_list[task_new].prt < task_list[ptr -> task_id].prt) break;
		else ptr = ptr -> pre_task;
	}
	ready_queue -> insert(
				task_new,
				ptr,
				(task_list[task_new].prt > task_list[ptr -> task_id].prt) ?
				(char) BEFORE : (char) AFTER
			);

	return SCHEDULABLE;
}

bool Task_Scheduler::EDF_sched(int &task_new)
{

	// Rearranging every task's priority
	return SCHEDULABLE;
}

void Task_Scheduler::dispatcher(void)
{
#ifdef DEBUG
	//int pre_task;
	cout << "DISPATCHER" << endl;
#endif
	//pre_task = running_task_id;
	running_task_id = ready_queue -> pop();
	task_list[running_task_id].state = (char) RUN;
#ifdef DEBUG
	cout << "running_task: Task_" << running_task_id << endl;
#endif
	// Checking whether context switch is needed or not, e.g., preemption or comletion/start is happened
	if(pre_task != running_task_id && pre_task != (int) CPU_IDLE) { // Preemption
		time_management -> ExecutedTime_Accumulator((unsigned char) PREEMPTED_POINT, (int) pre_task);
#ifdef DEBUG
		cout << "preemption(pre:" << pre_task << ", new:" << running_task_id << ")" << endl;
#endif	
		context_switch(pre_task, running_task_id);

#ifdef DEBUG		
		cout << "start new task" << endl; 
#endif	
		rwcet = task_list[running_task_id].wcet;
		new_task_start_flag = true;
		inter_intra_bus -> start_new_task_config(
					running_task_id, 
					SimPattern_cnt[running_task_id] % ((int) patterns_num),//rand() % ((int) patterns_num),
					(int) WORST, 
					task_list[running_task_id].release_time, 
					(double) slack,
					(double) worst_interference[running_task_id] , // pass the worst-case interference time
					(double) avg_interference[running_task_id], // pass the average-case interference time
					task_list[running_task_id].rel_dline, 
					inter_intra_bus -> intra_tasks[running_task_id].dvfs_en
				);
		SimPattern_cnt[running_task_id] += 1; 
		new_task_start_flag = false;
		time_management -> ExecutedTime_Accumulator((unsigned char) START_TIME, (int) running_task_id);
		//printf("running_task_id for ExecutedTime Updating is: %d\r\n", running_task_id);
		//printf("ExeutedTime: %f us, UpdatePoint: %f us\r\n", time_management -> ExecutedTime[running_task_id], time_management -> UpdatePoint);
	}
	else if(pre_task == running_task_id && pre_task != (int) CPU_IDLE) { // Change nothing but continue current task which suppose has the highest priority
#ifdef DEBUG
		cout << "Here arrives a new task with lower priority, thus continuing current task without either preemption or restart" << endl;			
#endif	
		pre_task = (int) CPU_IDLE;
	} 
	else if(cur_context.isr_flag == true && cur_context.task_id == running_task_id) { // Resuming from previous preemption
#ifdef DEBUG
		cout << "resume" << endl;
#endif	
		if(running_task_id != cur_context.task_id) {
			cout << "The TaskID poped from Ready Queue is " << running_task_id << ", however the TaskID poped from Preemption Stack is " << cur_context.task_id << endl;
			cout << "There must be wrong operations which endup insistency between Ready Queue and Preemption Stack" << endl;
			exit(1);
		}
		//cout << "cur_context.task_id: " << cur_context.task_id << " running_task_id:  " << running_task_id;
		//resume();
	} 
	else { // Starting new arrival task
#ifdef DEBUG
		cout << "start new task" << endl; 
		list_task_state();
#endif	
		rwcet = task_list[running_task_id].wcet;
		new_task_start_flag = true;
		inter_intra_bus -> start_new_task_config(
					running_task_id, 
					SimPattern_cnt[running_task_id] % ((int) patterns_num),//rand() % ((int) patterns_num), 
					(int) WORST, 
					task_list[running_task_id].release_time, 
					(double) slack,
					(double) worst_interference[running_task_id], // pass the worst-case interference time
					(double) avg_interference[running_task_id], // pass the average-case interference time
					task_list[running_task_id].rel_dline, 
					inter_intra_bus -> intra_tasks[running_task_id].dvfs_en
				);
		SimPattern_cnt[running_task_id] += 1; 
		new_task_start_flag = false;
		time_management -> ExecutedTime_Accumulator((unsigned char) START_TIME, (int) running_task_id);
		//printf("running_task_id for ExecutedTime Updating is: %d\r\n", running_task_id);
		//printf("ExeutedTime: %f us, UpdatePoint: %f us\r\n", time_management -> ExecutedTime[running_task_id], time_management -> UpdatePoint);
	} 
#ifdef DEBUG
//	list_task_state();
#endif
}

bool Task_Scheduler::IsIdle(void)
{
	return (running_task_id == (int) CPU_IDLE) ? true : false;
}

void Task_Scheduler::list_task_state(void)
{
        cout << "===================================" << endl;
        cout << "Current Time: " << (int) (time_management -> sys_clk -> cur_time) << " s" << endl;
        for(int i = 0; i < tasks_num; i++) {
                switch(task_list[i].state) {
                        case (char) READY:
                                cout << "Task_" << i << ": " << "READY";
                                break;
                        case (char) WAIT:
                                cout << "Task_" << i << ": " << "WAIT";
                                break;
                        case (char) IDLE:
                                cout << "Task_" << i << ": " << "IDLE";
                                break;
                        case (char) RUN:
                                cout << "Task_" << i << ": " << "RUN";
                                break;
                        case (char) ZOMBIE:
                                cout << "Task_" << i << ": " << "ZOMBIE";
                                break;
                }
		printf("  (Release Time: %.02f, Absolute Deadline: %.02f)\r\n",
				task_list[i].release_time,
				task_list[i].rel_dline + task_list[i].release_time
		);
        }

        cout << endl << "#Ready Queue:" << endl;
        ready_queue -> list_sched_point();
        cout << "===================================" << endl;

}

int Task_Scheduler::show_SimPatternCnt(int TskID)
{
	return SimPattern_cnt[TskID];
}
