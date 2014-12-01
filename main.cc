#include "classes.h"

using namespace std;

bool tst;
ifstream trace;
string strIn;

bool AdvanceCycle(Pipeline* pipe);

int main(int argc, char *argv[])
{
    if(atoi(argv[3])!=0)
    {
        if(atoi(argv[6])!=0)
        {
            L2 = new Cache(atoi(argv[3]),atoi(argv[6]),atoi(argv[7]),0,0,"L2",NULL,NULL);
        }
        L1 = new Cache(atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),0,0,"L1",L2,NULL);
    }
   	trace.open(argv[8]);
    if (!trace)
    {
        cerr << "File not found, please check" << endl;
        exit(1);
    }

    Pipeline* pipe = new Pipeline(1024,128,atoi(argv[1]),atoi(argv[2]));

    Transaction InTran;
    InTran.setType(0);
    
    while(AdvanceCycle(pipe))
    {
        //Retire Instruction
        pipe->retire_inst();

        //Execute Instruction
        pipe->execute_inst();

        //Issue Instructions
        pipe->issue_inst(InTran,L1);

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
    //Print Cache contents
    
    if(atoi(argv[3])!=0)
    {
        cout <<"L1 CACHE CONTENTS" << endl;
        L1->print_raw_op();
        L1->print_contents();
        cout << endl;

        if(atoi(argv[6])!=0)
        {
            cout <<"L2 CACHE CONTENTS" << endl;
            L2->print_raw_op();
            L2->print_contents();
            cout << endl;
        }
    }

    cout << "CONFIGURATION" << endl;
    cout << " superscalar bandwidth (N) = " << dec << pipe->n_ret() << endl;
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

    //End trace and end simulation
    if(trace.eof() & !pipe->head_valid())
        return false;
    
    //Increment cycles
    cycles++;

    //if(Debug&(pipe->tran_cnt_ret()>200))
    //    return false;
    return true;
}
