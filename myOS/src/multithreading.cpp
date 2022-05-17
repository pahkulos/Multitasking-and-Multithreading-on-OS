


#include <multitasking.h>
using namespace myos;
using namespace myos::common;

namespace myos{
    //Functions for creating, terminating, yielding, and joining the threads.
    void *pthread_create(Thread *thread, GlobalDescriptorTable *gdt, void entrypoint()){
        
        thread->getCPUState()->cs = gdt->CodeSegmentSelector();
        thread->getCPUState()->eip = (uint32_t)entrypoint;

    }
    void pthread_terminate(Thread *restrict){
        restrict->setMode(TERMINATEMODE);
    }
    void pthread_yield(Thread *restrict){
        if(restrict->getMode()!=TERMINATEMODE)
            restrict->setMode(YIELDMODE);
    }
    void pthread_join(Thread *thread, Thread * other){
        if(thread->getMode()!=TERMINATEMODE){
            thread->setMode(JOINMODE);
            thread->setjoinThread(other);
        }
    }
    //******************Thread Class Implementations**********************
    Thread::Thread()
    {
        threadMode=NORMALMODE;

        cpustate = (CPUState*)(stack + 4096 - sizeof(CPUState));
        
        cpustate -> eax = 0;
        cpustate -> ebx = 0;
        cpustate -> ecx = 0;
        cpustate -> edx = 0;

        cpustate -> esi = 0;
        cpustate -> edi = 0;
        cpustate -> ebp = 0;
        cpustate -> eflags = 0x202;
        
    }

    Thread::~Thread()
    {
    }

    //******************ThreadManager Class Implementations**********************
    ThreadManager::ThreadManager()
    {
        numThreads = 0;
        currThread = -1;
    }   
    ThreadManager::~ThreadManager()
    {
    } 
    bool ThreadManager::addThread(Thread* thread)
    {
        if(numThreads >= 256)
            return false;
        threads[numThreads++] = thread;
        return true;
    }
    void ThreadManager::setCurrentThread(CPUState *cpustate){
        threads[currThread]->cpustate=cpustate;
    }
    int Thread::getMode(){return this->threadMode; }
    void Thread::setMode(int mode) { this->threadMode=mode; }
    void Thread::setjoinThread(Thread *joinThread) { this->joinThread=joinThread; }
    Thread* Thread::getjoinThread() { return this->joinThread; }
    
    void takeMod(int *num1,int num2){
        if(++(*num1) >= num2)
            *num1 %= num2;
    }
    
    CPUState* ThreadManager::Schedule(CPUState* cpustate)
    {
        int numOfTerminated=0;
        if(numThreads <= 0)
            return cpustate;
         takeMod(&currThread,numThreads);
         
        /*
        This loop will loop until it finds a thread that has been normal or joined and the other thread has terminated.
        */
        while(true){
            //If the thread is in yield mode, it switches to normal mode and moves to the next thread.
            if(threads[currThread]->getMode()==YIELDMODE){  
                threads[currThread]->setMode(NORMALMODE);
                takeMod(&currThread,numThreads);
            }
                
            //If the thread is in join mode and the other thread is not terminated, it moves to the next thread.
            if(threads[currThread]->getMode()==JOINMODE && threads[currThread]->getjoinThread()->getMode() != TERMINATEMODE)
                takeMod(&currThread,numThreads);
               
                
            //If the thread is in terminate mode, it moves to the next thread.
            if(threads[currThread]->getMode()==TERMINATEMODE){
                takeMod(&currThread,numThreads);
                numOfTerminated++;
            }
            
            //If thread is normal or joined and the other thread has terminated
            if(threads[currThread]->getMode()==NORMALMODE || threads[currThread]->getMode()==JOINMODE) 
                break;
            
            //if All threads are terminated
            if(numOfTerminated==numThreads){  
                numThreads=0;
                return cpustate;
            }
        }  
        return threads[currThread]->cpustate;
    }
}
