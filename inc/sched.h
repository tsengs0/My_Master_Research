#ifndef __SCHED_H
#define __SCHED_H

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "main.h"
#include "dvfs_info.h"
#include "timer.h"
#include "cfg_info.h"
#include "main.h"

using namespace std;

#define CPU_IDLE 0x7FFFFFFF
#define NO_PREEMPTION 0x7FFFFFFF

enum {
	BEFORE = 1,
	AFTER  = 2
};

enum {
	SCHEDULABLE     = true,
	NON_SCHEDULABLE = false
};

enum {
	RM  = 1,
	EDF = 2
};

enum {
	NO_ARRIVE   = 0,
	TASK_ARRIVE = 1
};

enum {
	READY  = 1,
	WAIT   = 2,
	IDLE   = 3,
	RUN    = 4,
	ZOMBIE = 5
};

typedef struct Task_Element {
	int task_id;
	struct Task_Element *pre_task;
	struct Task_Element *next_task;
} task_element_t;

class Ready_Queue {
	private: 

	public:
		Ready_Queue(void);
		~Ready_Queue(void);

		bool IsEmpty(void);
		int get_front(void);
		int get_rear(void);
		void push(int &front_new);
		void insert(int new_val, task_element_t *&pre_task, char location); 
		int pop(void);		
		void list_sched_point(void);
		//friend bool Task_Scheduler::RM_sched(void);
		//friend bool Task_Scheduler::EDF_sched(void);
		task_element_t *ptr;
		task_element_t *front;
		task_element_t *rear;
		int cnt; // The number of tasks in Ready Queue
};
/*
class Task_Management {
	private:
		Ready_Queue ready_queue; 	
	public:
		Task_Management(void);
		~Task_Management(void);
		
		char task_scheduler();
};
*/

typedef struct Task_Info {
	float release_time;
	float start_time;
	int prt;
	float rel_dline;
#ifdef INTRA_SCHED
	int wcet; // unit: cycle
#else
	float wcet; // unit: us
#endif
	float period;
	bool NRT_USED; // A flag used to label whether the polling of release_time have been detected/used or not
	char state;
//	Task_Info(int r, int pr, int rd, int wc, int p, char st)
//	: release_time(r), prt(pr), rel_dline(rd), wcet(wc), period(p), state(st) {}
} task_info_t;

typedef struct Preemption_Stack {
	int task_id;
#ifdef INTRA_SCHED
	int rwcet; // unit: cycle
#else
	float rwcet; // unit: us
#endif
	bool isr_flag;
} preemption_stack_t;

class Task_State_Bus;
class Task_Scheduler {
	private:		

	public:  
		Task_Scheduler(Time_Management *timer, vector<task_info_t> &tasks, Ready_Queue &queue, char policy, Task_State_Bus *&msg_bus);
		~Task_Scheduler(void);
		
		void context_switch(int &cur_task, int &new_task);
		void resume(void);
		void sched_arbitration(float sched_tick);
		bool RM_sched(int &task_new);
		bool EDF_sched(int &task_new);
		void dispatcher(void);
		bool IsIdle(void); // Checking is processor at idle state
		void list_task_state(void);

		Time_Management *time_management;
		vector<task_info_t> task_list;	
		Ready_Queue *ready_queue;	
		char sched_policy;
#ifdef INTRA_SCHED
		int rwcet; // unit: cycle
#else
		float rwcet; // unit: us
#endif
		preemption_stack_t isr_stack;
		int running_task_id; // The identity of currently running task
		int pre_task; // To record the current task id, whilst a new task arrives and a preemption may occur
		bool new_task_start_flag;

		// #(Interface) Communication Bus
		Task_State_Bus *inter_intra_bus;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Head File "inter_bus.h"

class Task_State_Bus {
	private:

	public:
		Task_State_Bus(Time_Management *&timer, vector<task_info_t> *src_inter, vector<Src_CFG> *src_intra);
		~Task_State_Bus(void);

		void scheduling_point_assign(int task_id, int case_t, char dvfs_en);

		// #(Interface)
		Time_Management *time_management;
		vector<Src_CFG> intra_tasks;
		vector<task_info_t> inter_tasks;
		
		// Labelling the communicating task, in order to remark
		// which task is utilising the communication bus
		int communication_id;  

		// Command 1			
		void start_new_task_config(
			int new_task_id, 
			int case_id, 
			int case_t, 
			float release_time_new, 
			float start_time_new, 
			float Deadline, 
			char DVFS_en
		);

		// Command 2
		void time_driven_cfg(int new_task_id);
		
};

#endif // __SCHED_H
