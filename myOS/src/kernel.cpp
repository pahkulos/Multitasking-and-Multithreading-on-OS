
#include <common/types.h>
#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>



// #define GRAPHICSMODE


using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;

#define FALSE 0 
#define TRUE 1 
#define N 2         /* number of processes */
int turn;           /* whose turn is it? */
int interested[N];  /* all values initially 0 (FALSE) */
int item=0;         /*  variable for critical region*/


/*
    Petersons algorithm functions
*/
void enter_region(int process){         /* process is 0 or 1 */
    int other;                          /* number of the other process */
    other=1-process;                    /* the opposite of process */
    interested[process]=TRUE;           /* show that you are interested */
    turn = process;                     /* set flag */
    while (turn == process && interested[other] == TRUE);   /* null statement */
}
void leave_region(int process){         /* process: who is leaving */
    interested[process]=FALSE;          /* indicate departure from critical region */
}




void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}



class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:
    
    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);        
    }
    
    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                            | (VideoMemory[80*y+x] & 0xF000) >> 4
                            | (VideoMemory[80*y+x] & 0x00FF);
    }
    
};

/*
    Producer Consumer Fashion.
    In these 2 thread functions, one function tries to increase the item value by 1, 
    while the other function tries to decrease it. 
    If 2 threads want to access the item value at the same time, critical action happens.

*/
void producer(){
    while(true){
        enter_region(0);        
        item++;                 // It tries to increase the global variable item by 1. !Critical action happen
        printf("Producer: ");
        printfHex(item);
        leave_region(0);
    }
    
}
void consumer(){
    while (true)
    {
        enter_region(1);
        item--;                // It tries to decrease the global variable item by 1. !Critical action happen
        printf("Consumer: ");
        printfHex(item);
        leave_region(1);
    }

}


void threadA()
{
    while(true)
        printf("A");
}
void threadB()
{
    while(true)
        printf("B");
}
void threadC()
{
    while(true)
        printf("C");
}
void threadD()
{
    while(true)
        printf("D");
}




typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}




extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");
    Thread t1,t2,t3,t4;
    GlobalDescriptorTable gdt;
    TaskManager taskManager;
    /* 2 threads that communicate with each other in a producer consumer fashion. */
    pthread_create(&t1,&gdt,producer);
    pthread_create(&t2,&gdt,consumer);
    Task task1 = Task(&t1);
    task1.addThread(&t2);
    taskManager.AddTask(&task1);

    /*//4 thread execution demo in 2 processes with 2 threads
    pthread_create(&t1,&gdt,threadA);
    pthread_create(&t2,&gdt,threadB);
    pthread_create(&t3,&gdt,threadC);
    pthread_create(&t4,&gdt,threadD);
    Task task1 = Task(&t1);
    task1.addThread(&t2);
    Task task2 = Task(&t3);
    task2.addThread(&t4);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);*/
    
    /* //terminate function demo. 
    pthread_create(&t1,&gdt,threadA);
    pthread_create(&t2,&gdt,threadB);
    Task task1 = Task(&t1);
    Task task2 = Task(&t2);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    pthread_terminate(&t1);*/

    

    /*  // join function demo. when this block is executed, it will not print the value A to the screen. 
        //If we terminate the t3 threat, the value A will be printed.
    pthread_create(&t1,&gdt,threadA);
    pthread_create(&t2,&gdt,threadB);
    pthread_create(&t3,&gdt,threadC);
    Task task1 = Task(&t1);
    Task task2 = Task(&t2);
    task1.addThread(&t3);
    pthread_join(&t1,&t3);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    
    //pthread_terminate(&t3);     //terminate t3
    
    */

    /*  //yield function demo. At first it tries to run another thread, then it starts to run itself.
    pthread_create(&t1,&gdt,threadA);
    Task task1 = Task(&t1);
    pthread_yield(&t1);
    taskManager.AddTask(&task1);
    */
    /*  //if all threads in the task are terminated
    pthread_create(&t1,&gdt,threadA);
    pthread_create(&t2,&gdt,threadB);
    pthread_create(&t3,&gdt,threadC);
    Task task1 = Task(&t1);
    Task task2 = Task(&t2);
    task1.addThread(&t3);
    pthread_terminate(&t2);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    */
    /* if thre is no task
    pthread_create(&t1,&gdt,threadA);
    pthread_create(&t2,&gdt,threadB);
    Task task1 = Task(&t1);
    Task task2 = Task(&t2);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    pthread_terminate(&t1);
    pthread_terminate(&t2);
    */
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    
    printf("Initializing Hardware, Stage 1\n");
    
    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif
    
    DriverManager drvManager;
    
        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);
        
    
        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);
        
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        VideoGraphicsArray vga;
        
    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();
        
    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif


    interrupts.Activate();
    
    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}
