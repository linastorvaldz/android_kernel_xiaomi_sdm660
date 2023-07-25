
#ifdef CONFIG_SCHED_TUNE

#include <linux/reciprocal_div.h>

/*
 * System energy normalization constants
 */
struct target_nrg {
	unsigned long min_power;
	unsigned long max_power;
	struct reciprocal_value rdiv;
};

int schedtune_cpu_boost_with(int cpu, struct task_struct *p);
int schedtune_task_boost(struct task_struct *tsk);
<<<<<<< HEAD
int schedtune_task_boost_rcu_locked(struct task_struct *tsk);
=======
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)

int schedtune_prefer_idle(struct task_struct *tsk);

void schedtune_enqueue_task(struct task_struct *p, int cpu);
void schedtune_dequeue_task(struct task_struct *p, int cpu);

#else /* CONFIG_SCHED_TUNE */

#define schedtune_cpu_boost_with(cpu, p)  0
#define schedtune_task_boost(tsk) 0

#define schedtune_prefer_idle(tsk) 0

#define schedtune_enqueue_task(task, cpu) do { } while (0)
#define schedtune_dequeue_task(task, cpu) do { } while (0)

<<<<<<< HEAD
#define stune_util(cpu, other_util, walt_load) cpu_util_cfs(cpu_rq(cpu))
=======
>>>>>>> 5958b69937a3 (Merge 4.19.289 into android-4.19-stable)
#endif /* CONFIG_SCHED_TUNE */
