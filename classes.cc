#include "classes.h"
#include <iterator>

int Debug = 1;
int cycles = -1;

InstEntry::InstEntry()
{
    valid = false;
    pc = 0;
    op = 0;
    dest = 0;
    src1 = 0;
    src2 = 0;
    mem = 0;
    state = NONE;
    tag = 0;
}

void InstEntry::inst_up(const string &x)
{
    istringstream buf(x);
    buf >> hex >> pc;
    buf >> op;
    buf >> dest;
    buf >> src1;
    buf >> src2;
    buf>> hex >> mem;
}

Pipeline::Pipeline(int k,int a,int b)
{    
    size = k;
    s = a;
    n = b;
    tran_cnt = -1;
    rob.resize(size);
    head = 0;
    tail = 0;
}

void Pipeline::tail_up()
{
    //Update based on max size
    tail++;
    tail %=size;
}
void Pipeline::head_up()
{    
    head++;
    head %=size;
} 

int Pipeline::head_tag()
{
    return rob.at(head).tag_ret();
}

bool Pipeline::head_valid()
{
    return rob.at(head).is_valid();
}

bool Pipeline::test_entry()
{
    if ((head == tail)&(!(tail == 0 && !rob.at(tail).is_valid())))
        return false;

    return true;
}

void Pipeline::fetch_inst(ifstream& trace)
{
    if (Debug)
        cout << "Enter Fetch Function" << endl;

    bool tst;
    string str;
    for(int i=0;i<n;i++)
    {
        tst = test_entry();
        
        //Check:
        //1. ROB can be filled?
        //2. End of Trace?
        //3. Fetch Queue full
        if(tst & !trace.eof()&(fetch_list.size()<n)) 
        {
            //Get line from trace
            getline(trace,str);
            tran_cnt++;

            //Add to Fake ROB
            rob.at(tail).inst_up(str);
            rob.at(tail).state_up(IF);
            rob.at(tail).tag_up(tran_cnt);
            rob.at(tail).set_valid();
            rob.at(tail).enter_if(cycles);
            
            //Add to Dispatch List
            fetch_list.push_back(tail);
            
            //Update ROB tail
            tail_up();

            //Debug
            cout << "fetch " << fetch_list.at(i)<<endl;
        }
        else 
            break;
    }
    if (Debug)
        cout << "Fetch Queue entries: " << 
            fetch_list.size() << endl;
}

//Function does both decode and dispatch and makes instructions ready
//for the issue stage. i.e. at this stage the dispatch is complete,
//hence the final state at this function is IS instead of ID.
void Pipeline::dispatch_inst()
{
    if (Debug)
        cout << "Enter Dispatch Function" << endl;

    //Advancing instructions to the IS stage
    //TODO renaming the instructions
    int tmp;
    for (int i=0;i<s;i++)
    {
        //Check if Scheduling queue is full
        //Get new insts from the disp stage
        if(issue_list.size()<s)
        {
            if(dispatch_list.size() == 0)
                break;
            
            //Move from dispatch to issue
            tmp = dispatch_list.at(0);
            issue_list.push_back(tmp);
            dispatch_list.erase(dispatch_list.begin());
            
            //Update details at ROB
            rob.at(tmp).state_up(IS);
            rob.at(tmp).enter_is(cycles);

            //Debug
            cout << "issue " << issue_list.at(i) << endl;
        }
        else
            break;
    }
    if (Debug)
        cout << "Issue Queue entries: " << 
            issue_list.size() << endl;

    //------------------------------------------------------------------
    //Transfer from IF to ID stage
    for (int i=0;i<n;i++)
    {
        //Check if dispatch queue is full
        //Get new insts from the fetch stage
        if(dispatch_list.size()<n)
        {
            if(fetch_list.size() == 0)
                break;
            
            //Move from fetch to dispatch
            tmp = fetch_list.at(0);
            dispatch_list.push_back(tmp);
            fetch_list.erase(fetch_list.begin());
            
            //Update details at ROB
            rob.at(tmp).state_up(ID);
            rob.at(tmp).enter_id(cycles);

            //Debug
            cout << "disp " << dispatch_list.at(i) << endl;
        }
        else
            break;
    }
    if (Debug)
        cout << "Dispatch Queue entries: " << 
            dispatch_list.size() << endl;
}

void Pipeline::issue_inst()
{
    if (Debug)
        cout << "Enter Issue Function" << endl;

}
void Pipeline::retire_inst()
{
    if (!rob.at(head).is_valid())
        return;
    rob.at(head).clear_valid();
    head_up();
}
