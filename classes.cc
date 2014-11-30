#include "classes.h"
#include <iterator>

int Debug = 0;
int cycles = -1;

InstEntry::InstEntry()
{
    valid = false;
    pc = 0;
    op = 0;
    dest = 0;
    src1 = 0;
    src2 = 0;
    src1_r = src2_r = false;
    mem = 0;
    state = NONE;
    tag = 0;
    exec_latency = 0;

}

void InstEntry::inst_up(const string &x)
{
    istringstream buf(x);
    buf >> hex >> pc;
    buf >> dec >> op;
    buf >> dec >> dest;
    buf >> dec >> src1;
    buf >> dec >> src2;
    buf >> hex >> mem;
}

Pipeline::Pipeline(int k,int r,int a,int b)
{    
    size = k;
    s = a;
    n = b;
    tran_cnt = -1;
    rob.resize(size);
    reg_file.resize(r);
    head = 0;
    tail = 0;
    //TODO will be different for bonus part
    max_lat = 5;
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
    for(int i=fetch_list.size();i<n;i++)
    {
        tst = test_entry();
        
        if(!trace.eof())
        {
            //Get line from trace
            getline(trace,str);
            tran_cnt++;
        }

        //Check:
        //1. ROB can be filled?
        //2. End of Trace?
        //3. Fetch Queue full
        if(tst & !trace.eof()&(fetch_list.size()<n)) 
        {
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
            if(Debug) cout << "fetch " << fetch_list.at(i)<<endl;
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
    int tmp;
    int src1,src2,dest;
    for (int i=issue_list.size();i<s;i++)
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

            //SRC1 and SRC2 based on the RMT
            //DEST for sure
            //SRC1
            src1 = rob.at(tmp).src1_ret();
            if(src1 > -1)
            {
                if(!reg_file.at(src1).is_ready())
                {
                    rob.at(tmp).src1_rename(reg_file.at(src1).name_ret());
                    rob.at(tmp).src1_wait();
                    if(Debug)
                    {
                        cout << "SRC 1 "<< src1 << endl;
                        cout << "SRC 1 new " << reg_file.at(src1).name_ret() << endl;
                    }
                }
                else
                    rob.at(tmp).src1_ready();
            }
            else
                rob.at(tmp).src1_ready();
            //SRC2
            src2 = rob.at(tmp).src2_ret();
            if(src2 > -1)
            {
                if(!reg_file.at(src2).is_ready())
                {
                    rob.at(tmp).src2_rename(reg_file.at(src2).name_ret());
                    rob.at(tmp).src2_wait();
                    if(Debug)
                    {
                        cout << "SRC 2 "<< src2 << endl;
                        cout << "SRC 2 new " << reg_file.at(src2).name_ret() << endl;
                    }
                }
                else
                    rob.at(tmp).src2_ready();
            }
            else
                rob.at(tmp).src2_ready();

            //DEST
            dest = rob.at(tmp).dest_ret();
            //Rename only if valid, i.e. >=0
            if (dest > -1)
            {
                //RMT Rename
                reg_file.at(dest).rename_reg(rob.at(tmp).tag_ret());
                reg_file.at(dest).clear_ready();
                //ROB rename
                rob.at(tmp).dest_rename(rob.at(tmp).tag_ret());
                
                if(Debug)
                {
                    cout << "Dest " << dest << endl;
                    cout << "Dest new " << rob.at(tmp).dest_new_ret() << endl;
                }
            }

            //Debug
            if(Debug) cout << "issue " << issue_list.at(i) << endl;
        }
        else
            break;
    }
    if (Debug)
    {
        cout << "Issue Queue entries: " << 
            issue_list.size() << endl;
        if (issue_list.size()) cout << "Issue Back " << issue_list.back() << endl;
    }

    //------------------------------------------------------------------
    //Transfer from IF to ID stage
    for (int i=dispatch_list.size();i<n;i++)
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
            if(Debug) cout << "disp " << dispatch_list.at(i) << endl;
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

    int tmp;
    int iter = 0;
    //Transfer from IS to EX stage
    for (int i=0;i<s;i++)
    {
        //Check if execute queue is full
        //Get new insts from the issue stage
        if(execute_list.size()<n*max_lat)
        {
            if(issue_list.size() == iter)
                break;
            
            if(Debug) cout << "Is it ready? " << rob.at(issue_list.at(iter)).is_inst_ready() << endl;
            //Move from issue to execute
            //Check if the inst is ready to execute first
            if(!rob.at(issue_list.at(iter)).is_inst_ready())
            {
                iter++;
                continue;
            }
            else
            {
                tmp = issue_list.at(iter);
                execute_list.push_back(tmp);
                issue_list.erase(issue_list.begin()+iter);
                
                if(Debug) 
                {
                    cout << "Issue size after execute list " << issue_list.size() << endl;
                    cout << "Present Cycle "<< cycles << endl;
                }
                //Update details at ROB
                rob.at(tmp).state_up(EX);
                rob.at(tmp).enter_ex(cycles);

                //Set execution latency
                switch(rob.at(tmp).op_ret())
                {
                    case 0:
                        rob.at(tmp).set_exec_lat(1);
                        break;

                    case 1:
                        rob.at(tmp).set_exec_lat(2);
                        break;

                    case 2:
                        //TODO additions for bonus part
                        rob.at(tmp).set_exec_lat(5);
                        break;
                }
                
                if(Debug) cout << "exec tag " << execute_list.back() << " exec lat " 
                    << rob.at(execute_list.back()).exec_lat_ret()<< endl;
            }
        }
        else
            break;
    }
    if (Debug)
        cout << "Executing entries: " << 
            execute_list.size() << endl;
}
void Pipeline::execute_inst()
{
    int tmp;
    int iter = 0;
    if(Debug) cout << "Enter Execute function" << endl;
    for (int i=0;i<n*max_lat;i++)
    {
        if(execute_list.size() == iter)
            break;
        else
        {
            if(Debug) cout << "Functional unit execute " << execute_list.at(iter)<<endl;
            tmp = execute_list.at(iter);
        
            rob.at(tmp).execute();
            if(Debug) cout << "rem exec time " << rob.at(tmp).exec_lat_ret() << endl;
            if(rob.at(tmp).is_inst_done())
            {
                //Update details at ROB
                rob.at(tmp).state_up(WB);
                rob.at(tmp).enter_wb(cycles);
                //TODO update regesters in issue list and RMT
                //RMT register update
                //These are updated only if reg# and tag are both matching
                if(rob.at(tmp).dest_ret()>=0)
                {
                    if(reg_file.at(rob.at(tmp).dest_ret()).name_ret() == 
                            rob.at(tmp).dest_new_ret())
                        reg_file.at(rob.at(tmp).dest_ret()).set_ready();
                }
                //Issue list source registers update
                for (int j=0;j<issue_list.size();j++)
                {
                    if(Debug)
                    {
                        cout << "Registers Update loop" << endl;
                        cout << "Issue tag " << rob.at(issue_list.at(j)).tag_ret() << endl;
                        cout << "Dest of exec " << rob.at(tmp).dest_ret() << 
                            " Dest_new of exec " << rob.at(tmp).dest_new_ret() << endl; 
                        cout << "Src 1 of iter " << rob.at(issue_list.at(j)).src1_ret() <<
                            " Src1 new of iter " << rob.at(issue_list.at(j)).src1_new_ret() <<
                            endl;
                        cout << "Src 1 of iter before" << 
                            rob.at(issue_list.at(j)).src1_is_ready() << endl;
                    }
                    //For src1
                    if (rob.at(tmp).dest_new_ret() == 
                            rob.at(issue_list.at(j)).src1_new_ret())
                        rob.at(issue_list.at(j)).src1_ready();

                    if(Debug)
                    {
                        cout << "Src 1 of iter after" << rob.at(issue_list.at(j)).src1_is_ready() << 
                            endl;
                        cout << "Src 2 of iter " << rob.at(issue_list.at(j)).src2_ret() <<
                            " Src2 new of iter " << rob.at(issue_list.at(j)).src2_new_ret() <<
                            endl;
                        cout << "Src 2 of iter before" <<
                            rob.at(issue_list.at(j)).src2_is_ready() << endl;
                    }

                    //For src2
                    if (rob.at(tmp).dest_new_ret() == 
                            rob.at(issue_list.at(j)).src2_new_ret())
                        rob.at(issue_list.at(j)).src2_ready();
                    if(Debug)
                    {
                        cout << "Src 2 of iter after" << 
                            rob.at(issue_list.at(j)).src2_is_ready() << endl;
                        cout << "Src 2 of iter " << rob.at(issue_list.at(j)).src2_ret() <<
                            " Src2 new of iter " << rob.at(issue_list.at(j)).src2_new_ret() <<
                            endl;
                    }
                }
                execute_list.erase(execute_list.begin()+iter);
            }
            else
                iter++;
        }
    }

}

void Pipeline::retire_inst()
{
    for (int i = head;i<tail;i++)
    {
        if(rob.at(head).state_ret() == WB)
            rob.at(head).exit_wb(cycles);
    }
    while(rob.at(head).state_ret() == WB)
    {
        //Exit step
        if (!rob.at(head).is_valid())
            return;
        //Print Data
        cout << rob.at(head).tag_ret();
        cout << " fu{"<< rob.at(head).op_ret() << "} ";
        cout << "src{"<< rob.at(head).src1_ret() << 
            "," << rob.at(head).src2_ret() << "} ";
        cout << "dst{"<< rob.at(head).dest_ret() << "} ";
        cout << "IF{"<< rob.at(head).if_entry_ret() << 
            "," << rob.at(head).if_dur_ret() << "} ";
        cout << "ID{"<< rob.at(head).id_entry_ret() << 
            "," << rob.at(head).id_dur_ret() << "} ";
        cout << "IS{"<< rob.at(head).is_entry_ret() << 
            "," << rob.at(head).is_dur_ret() << "} ";
        cout << "EX{"<< rob.at(head).ex_entry_ret() << 
            "," << rob.at(head).ex_dur_ret() << "} ";
        cout << "WB{"<< rob.at(head).wb_entry_ret() << 
        ",1}";// << rob.at(head).wb_dur_ret() << "}";
        cout << endl;
        rob.at(head).clear_valid();
        head_up();
    }
}
