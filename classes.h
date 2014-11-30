#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

extern int Debug;
extern int cycles;

enum{   NONE=0,IF,ID,IS,EX,WB  };

class InstEntry{
    private:
        bool valid;
        int pc,op,dest,src1,src2,mem;
        int tag,state;
        int if_entry, if_dur;
        int id_entry,id_dur;
        int is_entry, is_dur;
        int ex_entry,ex_dur;
        int wb_entry,wb_dur;

    public:
        InstEntry();
        bool is_valid() {   return valid; }
        void set_valid(){   valid = true;   }
        void clear_valid(){ valid = false;  }

        void tag_up(int n)  { tag = n;  }
        void state_up(int a) { state = a;   }

        int tag_ret() { return tag; }
        void inst_up(const string &x);

        void enter_if(int c){ if_entry = c; }

        //Enter ID means exit IF
        void enter_id(int c){ id_entry = c; if_dur = c - if_entry;}

        void enter_is(int c){ is_entry = c; id_dur = c - id_entry;}}

        void enter_ex(int c){ ex_entry = c; is_dur = c - is_entry;}}

        void enter_wb(int c){ wb_entry = c; ex_dur = c - ex_entry;}}
        void exit_wb(int c) { wb_dur = c - wb_entry;}

};

class Pipeline{
    private:
        vector<InstEntry> rob;
        vector<int> fetch_list;
        vector<int> dispatch_list;
        vector<int> issue_list;
        vector<int> execute_list;
        vector<int> retire_list;
        int head,tail,size;
        int s,n;
        int tran_cnt;

    public:
        Pipeline(int k,int a,int b);
        
        //int sizechk() {    return rob.size();  }

        int n_ret() { return n;}

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
        void retire_inst();
};

