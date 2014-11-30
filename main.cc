#include <cstdlib>

#include "classes.h"

using namespace std;

bool tst;
ifstream trace;
string strIn;

bool AdvanceCycle(Pipeline* pipe);

int main(int argc, char *argv[])
{
   	trace.open(argv[8]);
    if (!trace)
    {
        cerr << "File not found, please check" << endl;
        exit(1);
    }

    Pipeline* pipe = new Pipeline(1024,atoi(argv[1]),atoi(argv[2]));
    

    while(AdvanceCycle(pipe))
    {
        //Dispatch Instructions
        pipe->dispatch_inst();

        //Fetch instructions from trace
        pipe->fetch_inst(trace);

        //The next if loop is debug only
        //Fake retire algo
        //Retires One instruction every 3rd cycle
        if((cycles%3==0)&(cycles!=0))
            pipe->retire_inst();

        if(Debug) 
        {
            cout<<"Head: " << pipe->head_ret() << " Tail: "
                << pipe->tail_ret() << endl;
            cout << "Tran: "<< pipe->tran_cnt_ret() << endl;
            cout << "Cycle: "<< cycles << endl;
            cout << "EOF: " << trace.eof() << endl;
            cout << "Head Tag: " << pipe->head_tag() << endl;
            cout << "Head Valid: " << pipe->head_valid() << endl;
            cout << "--------------------------------------" << endl;
        }

    }
    return 0;
}

bool AdvanceCycle(Pipeline *pipe)
{
    //Increment cycles
    cycles++;

    //End trace and end simulation
    if(trace.eof() & !pipe->head_valid())
        return false;
    
    if(Debug&(pipe->tran_cnt_ret()>100))
        return false;
    return true;
}
