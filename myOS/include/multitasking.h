 
#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <multithreading.h>
#include <gdt.h>


using namespace myos;
namespace myos
{
    class Task
    {
    friend class TaskManager;
    private:
        ThreadManager threadManager;
    public:
        Task();
        Task(Thread * thread);
        ~Task();
        bool addThread(Thread * thread);
    };
    
    
    class TaskManager
    {
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(Task* task);
        CPUState* Schedule(CPUState* cpustate);
    }; 
    
    
}
#endif