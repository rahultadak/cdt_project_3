
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

    Pipeline* pipe = new Pipeline(1024,128,atoi(argv[1]),atoi(argv[2]));
    

    while(AdvanceCycle(pipe))
    {
        //Retire Instruction
        pipe->retire_inst();

        //Execute Instruction
        pipe->execute_inst();

        //Issue Instructions
        pipe->issue_inst();

        //Dispatch Instructions
        pipe->dispatch_inst();

        //Fetch instructions from trace
        pipe->fetch_inst(trace);

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
    cout << "CONFIGURATION" << endl;
    cout << " superscalar bandwidth (N) = " << pipe->n_ret() << endl;
    cout << " dispatch queue size (2*N) = " << 2*pipe->n_ret() << endl;
    cout << " schedule queue size (S)   = " << pipe->s_ret() << endl;
    cout << "RESULTS" << endl;
    cout << " number of instructions = " << pipe->tran_cnt_ret() << endl;
    cout << " number of cycles       = " << cycles << endl;
    cout << " IPC                    = " << setprecision(2) << fixed 
        << (float)pipe->tran_cnt_ret()/cycles << endl; 
    return 0;
}

bool AdvanceCycle(Pipeline *pipe)
{
    //Increment cycles
    cycles++;

    //End trace and end simulation
    if(trace.eof() & !pipe->head_valid())
        return false;
    
    //if(Debug&(pipe->tran_cnt_ret()>200))
    //    return false;
    return true;
}
