#ifndef __MYOS__MULTITHREADING_H
#define __MYOS__MULTITHREADING_H

#include <common/types.h>
#include <gdt.h>
#define TERMINATEMODE -1
#define NORMALMODE 0
#define YIELDMODE 1
#define JOINMODE 2

using namespace myos;
namespace myos
{
     struct CPUState
    {
        common::uint32_t eax;
        common::uint32_t ebx;
        common::uint32_t ecx;
        common::uint32_t edx;

        common::uint32_t esi;
        common::uint32_t edi;
        common::uint32_t ebp;

        /*
        common::uint32_t gs;
        common::uint32_t fs;
        common::uint32_t es;
        common::uint32_t ds;
        */
        common::uint32_t error;

        common::uint32_t eip;
        common::uint32_t cs;
        common::uint32_t eflags;
        common::uint32_t esp;
        common::uint32_t ss;        
    } __attribute__((packed));

    class Thread{
        friend class ThreadManager;
        private:
            common::uint8_t stack[4096];
            CPUState * cpustate;
            int threadMode;
            Thread *joinThread;
            
        public:
            CPUState* getCPUState(){ return cpustate;}
            void setMode(int mode);
            void setjoinThread(Thread *joinThread);
            Thread* getjoinThread();
            int getMode();
            Thread();
            ~Thread();
    }; 

    class ThreadManager{
    private:
        Thread * threads[256];
        int numThreads;
        int currThread;
        
    public:
        ThreadManager();
        ~ThreadManager();
        int getNumThreads(){return numThreads;}
        bool addThread(Thread* task);
        CPUState* Schedule(CPUState* cpustate);
        void setCurrentThread(CPUState *cpustate);
    };
    void *pthread_create(Thread *thread, GlobalDescriptorTable *gdt, void entrypoint());
    void pthread_terminate(Thread *restrict);
    void pthread_yield(Thread *restrict);
    void pthread_join(Thread *thread, Thread * other);
}

#endif