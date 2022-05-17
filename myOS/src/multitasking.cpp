
#include <multitasking.h>

using namespace myos;
using namespace myos::common;

namespace myos{
    //******************Task Class Implementations**********************
    
    Task::Task(Thread * thread)
    {
        this->threadManager.addThread(thread); 
    }
    Task::Task(){
    }
    Task::~Task()
    {
    }
    // adds new thread to the task
    bool Task::addThread(Thread * thread){
        return this->threadManager.addThread(thread);
    }
    //******************TaskManager Class Implementations**********************
    TaskManager::TaskManager()
    {
        numTasks = 0;
        currentTask = -1;
    }

    TaskManager::~TaskManager()
    {
    }

    bool TaskManager::AddTask(Task* task)
    {
        if(numTasks >= 256)
            return false;
        tasks[numTasks++] = task;
        return true;
    }

    CPUState* TaskManager::Schedule(CPUState* cpustate)
    {
        int numOfEnded=0;
        if(numTasks <= 0)
            return cpustate;
        
        if(currentTask >= 0)
            tasks[currentTask]->threadManager.setCurrentThread(cpustate);
        
        if(++currentTask >= numTasks)
            currentTask %= numTasks;
        while(true){
            if(numOfEnded==numTasks){
                numTasks=0;
                return cpustate;
            }
            if(tasks[currentTask]->threadManager.getNumThreads()==0){
                numOfEnded++;
                if(++currentTask >= numTasks)
                    currentTask %= numTasks;
            }
            else
                break;
            
        }
        
        return tasks[currentTask]->threadManager.Schedule(cpustate);
    }

    
    
}









