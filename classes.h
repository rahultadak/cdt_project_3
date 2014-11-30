#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdlib>
using namespace std;

extern int Debug;
extern int cycles;

enum{   NONE=0,IF,ID,IS,EX,WB  };

class Register{
    private:
        int rename;
        bool ready;

    public:
        Register() { rename = 0; ready = true; }
        void rename_reg(int r) { rename = r;}
        int name_ret()  { return rename;    }
        bool is_ready() { return ready; }
        void set_ready(){ ready = true; }
        void clear_ready(){ ready = false;}

};

class InstEntry{
    private:
        bool valid;
        int pc,op,dest,src1,src2,mem;
        bool src1_r,src2_r;
        int dest_new,src1_new,src2_new;
        int tag,state;
        int if_entry, if_dur;
        int id_entry,id_dur;
        int is_entry, is_dur;
        int ex_entry,ex_dur;
        int wb_entry,wb_dur;

        bool inst_ready;

        int exec_latency;

    public:
        InstEntry();
        bool is_valid() {   return valid; }
        void set_valid(){   valid = true;   }
        void clear_valid(){ valid = false;  }

        void tag_up(int n)  { tag = n;  }
        void state_up(int a) { state = a;   }

        int tag_ret() { return tag; }
        int state_ret() { return state; }
        void inst_up(const string &x);

        int op_ret() { return op; }
        int src1_ret() { return src1; }
        int src2_ret() { return src2; }
        int dest_ret() { return dest; }
        int mem_ret() { return mem; }

        int src1_rename(int a)  { src1_new = a; }
        int src2_rename(int a)  { src2_new = a; }
        int dest_rename(int a)  { dest_new = a; }

        int src1_new_ret() { return src1_new; }
        int src2_new_ret() { return src2_new; }
        int dest_new_ret() { return dest_new; }

        void src1_ready()   { src1_r = true;}
        void src2_ready()   { src2_r = true;}

        void src1_wait()    { src1_r = false;}
        void src2_wait()    { src2_r = false;}

        bool src1_is_ready(){ return src1_r;}
        bool src2_is_ready(){ return src2_r;}

        void enter_if(int c){ if_entry = c; }

        //Enter ID means exit IF
        void enter_id(int c){ id_entry = c; if_dur = c - if_entry;}

        void enter_is(int c){ is_entry = c; id_dur = c - id_entry;}

        void enter_ex(int c){ ex_entry = c; is_dur = c - is_entry;}

        void enter_wb(int c){ wb_entry = c; ex_dur = c - ex_entry;}
        void exit_wb(int c) { wb_dur = c - wb_entry;}

        int if_entry_ret() { return if_entry;}
        int if_dur_ret() { return if_dur;}
        int id_entry_ret() { return id_entry;}
        int id_dur_ret() { return id_dur;}
        int is_entry_ret() { return is_entry;}
        int is_dur_ret() { return is_dur;}
        int ex_entry_ret() { return ex_entry;}
        int ex_dur_ret() { return ex_dur;}
        int wb_entry_ret() { return wb_entry;}
        int wb_dur_ret() { return wb_dur;}

        bool is_inst_ready()    { return (src1_r & src2_r);}

        void set_exec_lat(int a)    { exec_latency = a;}
        int exec_lat_ret()  {return exec_latency;}
        bool is_inst_done() {if(exec_latency == 0) return true; else return false;}
        void execute()  {exec_latency = exec_latency - 1 ;}
};

class Pipeline{
    private:
        vector<InstEntry> rob;
        vector<Register> reg_file;
        vector<int> fetch_list;
        vector<int> dispatch_list;
        vector<int> issue_list;
        vector<int> execute_list;
        int head,tail,size;
        int s,n;
        int tran_cnt;
        int max_lat;

    public:
        Pipeline(int k,int r,int a,int b);
        
        //int sizechk() {    return rob.size();  }

        int n_ret() { return n;}
        int s_ret() { return s;}

        int tail_ret() {    return tail;    }
        int head_ret() {    return head;    }
        void tail_up();
        void head_up();
        int head_tag();
        bool head_valid();

        int tran_cnt_ret(){ return tran_cnt;}
        bool test_entry();

        void fetch_inst(ifstream& trace);
        void dispatch_inst();
        void issue_inst();
        void execute_inst();
        void retire_inst();
};

