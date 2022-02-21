#include "../../h/const.h"
#include "../../h/types.h"
#include "../../h/util.h"
#include "../../h/vpop.h"
#include "../../h/procq.e"
#include "../../h/asl.e"
#include "../../h/int.e"
#include "../../h/main.e"

void Create_Process(state_t* old_state){
   // print("11th    ");
    proc_t* fork_process = allocProc();
    proc_t* cur = headQueue(running_queue);
    if (fork_process == (proc_t *)ENULL){
        old_state->s_r[2] = -1;
    }else{
        fork_process->p_s = *((state_t*)old_state->s_r[4]);
        if (cur->child == (proc_t *)ENULL){
            cur->child = fork_process;
            fork_process->parent = cur;
        }else{
            proc_t* first_child = cur->child;
            while(first_child->parent == (proc_t *)ENULL){
                first_child = first_child->brother;
            }
            first_child->parent = (proc_t *)ENULL;
            first_child->brother = fork_process;
            fork_process->parent = cur;
        }
        insertProc(&running_queue,fork_process);
        old_state->s_r[2] = 0;
    }
}

void check_sibling(proc_t* cur){
    proc_t* tmp;
    proc_t* first_child = cur->child;
    while(first_child->brother != (proc_t *)ENULL){
        tmp = first_child->brother;
        first_child->brother = (proc_t *)ENULL;
        proc_t* removed = outBlocked(first_child);
        // either in sem list or running queue
        if (removed == (proc_t *)ENULL){
            removed = outProc(&running_queue,first_child);
        }
        // call recursion
        if (removed->child != (proc_t *)ENULL) {
            check_sibling(removed);
        }
        freeProc(removed);
       // print("rm process");
        first_child = tmp;
    }
    proc_t* removed = outBlocked(first_child);
    // either in sem list or running queue
    if (removed == (proc_t *)ENULL){
        removed = outProc(&running_queue,first_child);
    }
    // call recursion
    if (removed->child != (proc_t *)ENULL) {
        check_sibling(removed);
    }
    freeProc(removed);
    // last one undo the link
    first_child->parent =  (proc_t *)ENULL;
    cur->child = (proc_t *)ENULL;
}
void Terminate_Process_Helper(proc_t* cur){
    if (cur->child == (proc_t *)ENULL){
        freeProc(outProc(&running_queue,cur));
        //print("rm process");
    }else{
        check_sibling(cur);
        freeProc(outProc(&running_queue,cur));
       // print("rm process");
    }
}
void Terminate_Process(){
    proc_t* cur = headQueue(running_queue);
    if (cur->parent != (proc_t *)ENULL && cur->brother == (proc_t *)ENULL){
        proc_t* first_child = cur->parent->child;
        if (first_child == cur){
            cur->parent->child = (proc_t *)ENULL;
            cur->parent = (proc_t *)ENULL;
        }else{
            while(first_child->brother != cur){
                first_child = first_child->brother;
            }
            first_child->brother = cur->brother;
            first_child->parent = cur->parent;
            cur->parent = (proc_t *)ENULL;
        }
    }else if ( (cur->parent ==  (proc_t *)ENULL && cur->brother!= (proc_t *)ENULL) ||
            (cur->parent != (proc_t *)ENULL && cur->brother!= (proc_t *)ENULL)){
            proc_t* first_brother = cur->brother;
            while(first_brother->parent == (proc_t *)ENULL){
                first_brother = first_brother->brother;
            }
            proc_t* first_child = first_brother->parent->child;
            if (first_child == cur){
                first_brother->parent->child = cur->brother;
                cur->brother = (proc_t *)ENULL;
            }else{
                while(first_child->brother != cur){
                    first_child = first_child->brother;
                }
                first_child->brother = cur->brother;
                cur->brother = (proc_t *)ENULL;
            }
    }
    Terminate_Process_Helper(cur);
    schedule();
}
void Sem_OP(state_t* old_state){
    proc_t* cur = headQueue(running_queue);
    vpop* vpop_addr = (vpop*) (old_state->s_r[4]);
    int flag = 0;
    int i;
    for(i = 0;i<old_state->s_r[3];i++,vpop_addr++){
        if(vpop_addr->op == LOCK){
            if ( --(*(vpop_addr->sem)) < 0){
                flag = 1;
                cur->p_s = *old_state;
                insertBlocked(vpop_addr->sem,cur);
            }
        }else{
            if ((*(vpop_addr->sem))++ <0){
                proc_t* removed = removeBlocked(vpop_addr->sem);
                if(removed!=(proc_t *)ENULL && removed->qcount == 0){
                    //print("131th");
                    insertProc(&running_queue,removed);
                }
            }
        }
    }
    if(flag){
       // print("138th");
        removeProc(&running_queue);
        schedule();
    }
}
void Specify_Trap_State_Vector(state_t* old_state){
    proc_t* cur = headQueue(running_queue);
    int trap = old_state->s_r[2];
    if (trap == 0){
        if (cur->prog_old != (state_t*)ENULL){
            Terminate_Process(old_state);
        }else{
            cur->prog_old = (state_t *) (old_state->s_r[3]);
            cur->prog_new = (state_t *) (old_state->s_r[4]);
        }
    }else if(trap == 1){
        if (cur->memory_management_old != (state_t*)ENULL){
            Terminate_Process(old_state);
        }else{
            cur->memory_management_old = (state_t *) (old_state->s_r[3]);
            cur->memory_management_new = (state_t *) (old_state->s_r[4]);
        }
    }else if(trap == 2){
        if (cur->system_call_old != (state_t*)ENULL){
            Terminate_Process(old_state);
        }else{
            cur->system_call_old = (state_t*) (old_state->s_r[3]);
            cur->system_call_new = (state_t *) (old_state->s_r[4]);
        }
    }
}
void notused(state_t* old_state){
     HALT();
}
void Get_CPU_Time(state_t* old_state){
    proc_t* cur = headQueue(running_queue);
    old_state->s_r[2] = cur->cpu_time;
}