#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/syscalls.h>
#include <linux/types.h>
asmlinkage long sys_hello(void) {
    printk("Hello, World!\n");
    return 0;
}

asmlinkage long sys_set_weight(int weight){
    if(weight < 0){
        return -EINVAL;
    }
    current->weight=weight;
    return 0;
}


asmlinkage long sys_get_weight(void){
    return current->weight;
}

asmlinkage int sys_get_path_sum(pid_t target){
    struct task_struct *task;
    long sum = 0;

    task = pid_task(find_vpid(target), PIDTYPE_PID);
    if (!task) {
        return -ECHILD;
    }


    while (task && task->parent != task) {
        sum += task->weight;
        if (task->pid == current->pid) {
            return sum;
        }
        task = task->parent;
    }

    return -ECHILD;

}



asmlinkage pid_t sys_get_heaviest_sibling(void){

    struct task_struct *current_process = current;
    struct task_struct *sibling;
    struct task_struct *heaviest_sibling = current;
    struct list_head *list;


    list_for_each(list, &current_process->parent->children) {
        sibling = list_entry(list, struct task_struct, sibling);
        if (sibling->weight > heaviest_sibling->weight ||
            (sibling->weight == heaviest_sibling->weight && sibling->pid < heaviest_sibling->pid)) {
            heaviest_sibling = sibling;
        }
    }

    return heaviest_sibling->pid;

}