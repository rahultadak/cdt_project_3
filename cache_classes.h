/*
 * Cache simulator: Can simulate all combinations of L1, Victim and L2.
 * Encoding of the swap variable is given below:
 *
    swp = 0, Normal access, not relation to victim cache
    swp = 1, Swap with victim, no dirty bit
    swp = 2, Swap with victim, with dirty bit
    swp = 3, Swap with victim with dirty bit and Cache itself has a write access
*/

/* Author:      Rahul Tadakamadla
 * Email:       rtadaka@ncsu.edu
 * Student ID:  200065391
 */


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iomanip>

#ifndef CACHE
#define CACHE



using namespace std;
//struct req_op{
//    int index,tag;
//    bool dirty;
//};

//transaction class
class Transaction{
    private:
        int type,addr;
        
    public:
        void setType(int x) { type = x;}
        int tranType() { return type;}

        void setAddr(int x) 
        {
            addr = x;
        }
        int retAddr() { return addr; }

};

//Block Class
class Block{
    private:
        int block_id,tag,bl_size,cnt_block;
        bool valid,dirty;

    public:
        void BlkSize(int x) { bl_size = x; }
        int rdBlkSize() { return bl_size; };

        int getTag() { return tag;}
        void updateTag(int in_tag) { tag = in_tag;} 

        bool isValid() { return valid;}
        void updateValid(bool in) { valid = in;}

        bool getDirty() { return dirty;}
        void updateDirty(bool in) { dirty = in;}

        int getCnt() { return cnt_block; }
        void updateCnt(int x) { cnt_block = x;}
};

//Set Class
class Set{
    private:
        int set_id,cnt_set,num_blk;

    public:
        vector<Block> SetBlocks;
        //vector<int> cnt_blks;
        
        void SetSize(int x) { num_blk =x; }
        int rdSetSize() { return num_blk; }

        int getCntSet() { return cnt_set;}
        void updateCntSet(int x) { cnt_set = x;}

        void cnt_set_init() { cnt_set = 0;}

        void setup_Blocks(int x,int rep_pol)
        {
            SetBlocks.resize(num_blk);
            for (int i=0;i<num_blk;i++)
            {
                SetBlocks.at(i).BlkSize(x);
                SetBlocks.at(i).updateTag(0);
                SetBlocks.at(i).updateValid(false);
                SetBlocks.at(i).updateDirty(false);

                if (!rep_pol){ SetBlocks.at(i).updateCnt(i);}
                else { SetBlocks.at(i).updateCnt(0);}

            }
        }

        //int getIndex() { return index;}
        //void updateIndex(int in_index) { index = in_index;}
};

//Cache class
class Cache{
    private:
    int bl_size;
    int size;
    int assoc;
    int n_blk;
    int n_set;
    int rep_pol;
    int wr_pol;
    string name;
    Cache* nextLevel;
    Cache* victim;
    //Counters
    long rdCnt,rdMCnt, wrCnt, wrMCnt,WB,TotMem, rdHCnt, wrHCnt;
    float MR;


    public:
    vector<Set> CacheSets;
    int bl_offset_mask, index_mask, tag_mask;

    bool has_victim,is_victim;
    //Default constructor
    Cache()
    {
        bl_size = 0;
        size = 0;
        assoc = 0;
        rep_pol = 0;
        wr_pol = 0;
    }
    //Constructor for input
    Cache(int a,int b,int c,int d,int e,string s,Cache* nL,Cache* v)
    {
        is_victim = false;
        bl_size = a;
        size = b;
        assoc = c;
        rep_pol = d;
        wr_pol = e;
        name = s;
        n_blk = size / bl_size;
        if (assoc == 0)
            n_set = 0;
        else
            n_set = n_blk / assoc;

        nextLevel = nL;
        victim = v;
        if (v != NULL)
        {
            if (v->getSize() != 0)
            {
                has_victim = true;
                victim = v;
            }
        }
        else
            has_victim = false;

        getBlOffsetMask();
        getIndexMask();
        getTagMask();

        setup_Sets();
        make_Blocks();
    
        //resetting counters
        rdCnt=0;
        rdMCnt=0;
        wrCnt=0;
        wrMCnt=0;
        rdHCnt=0;
        wrHCnt=0;
        WB=0;
        TotMem=0;
    }

    //Constructor for victim type cache
    Cache(int a, int b)
    {
        is_victim = true;
        bl_size =a;
        size = b;
        assoc = size / bl_size;
        n_set = 1;
        has_victim = false;
        name = "Victim";

        getBlOffsetMask();
        getIndexMask();
        getTagMask();
        setup_Sets();
        make_Blocks();

    }

    void nLevel(Cache* nl) { nextLevel = nl;}
    int nLevelPresent() 
    {
        if (nextLevel == NULL)
            return 0;
        else 
            return 1;
    }

    int getSize() { return size;}
    void updateSize(int s) { size = s;}

    int getBlSize() { return bl_size;}
    void updateBlSize(int b) {bl_size = b;}

    int getNumBlk() { return n_blk;}
    int getNumSet() { return n_set;}
    int getWrPol() { return wr_pol;}
    int getAssoc() { return assoc;}
    void updateAssoc(int a) { assoc = a;}
    
    int getrdCnt() { return rdCnt;}
    void updaterdCnt() { rdCnt++;}
    
    int getwrCnt() { return wrCnt;}
    void updatewrCnt() { wrCnt++;}

    int getrdMCnt() { return rdMCnt;}
    void updaterdMCnt() { rdMCnt++;}

    int getwrMCnt() {return wrMCnt;}
    void updatewrMCnt() {wrMCnt++;}

    int getrdHCnt() { return rdHCnt;}
    void updaterdHCnt() { rdHCnt++;}

    int getwrHCnt() {return wrHCnt;}
    void updatewrHCnt() {wrHCnt++;}

    int getWB() { return WB;}
    void updateWB() { WB++;}//=bl_size;}

    int getTotMem() { return TotMem;}
    void updateTotMem() {TotMem++;}//=bl_size;}

    void hasVictim() { has_victim = true;}

    float getMR() 
    {
        if (assoc == 0)
            return 0;

        if (name == "L1")
            MR = ((float)rdMCnt - (victim->getrdCnt() - victim->getrdMCnt()) + wrMCnt 
                        - (victim->getwrCnt() - victim->getwrMCnt()))/(rdCnt + wrCnt);
        else 
            //From the standpoint of stalling the CPU, only reads to the higher level cause an issue.
            MR = ((float)rdMCnt)/rdCnt;
        return MR;
    }

    void getBlOffsetMask()
    {
        //Mask needs to be for the log2(bl_size) number of bits,
        //Subtracting 1 will give us the corresponding mask
        //as the indices start fro 0 and end at bl_size - 1.
        bl_offset_mask = bl_size - 1;
    }

    void getIndexMask()
    {
        int x = n_set - 1;
        //Indices start from 0 thus subtracting 1
        
        index_mask = x*bl_size;
        //This is to shift the mask by block size, thus givin the index mask
    }

    void getTagMask()
    {
        tag_mask = 0xFFFFFFFF - (bl_offset_mask + index_mask);
    }

    int setup_Sets()
    {
        CacheSets.resize(n_set);
        for (int i=0;i<n_set;i++)
        {
            CacheSets.at(i).SetSize(assoc);
            if (rep_pol) { CacheSets.at(i).cnt_set_init();}
        }
        return 0;
    }

    void make_Blocks()
    {
        for (int i=0;i<n_set;i++)
        {
            CacheSets.at(i).setup_Blocks(bl_size,rep_pol);
        }
    }

    void updateBlock(int addr, int index, int tag, int k, int rw, int swp)
    {
        int wb_tag = CacheSets.at(index).SetBlocks.at(k).getTag();
        
        int wb_addr = wb_tag*n_set*bl_size + index*bl_size ;
        if (has_victim && swp)
        {
            victim->request(wb_addr,1);
            victim->victim_dirty_update(wb_addr,CacheSets.at(index).SetBlocks.at(k).getDirty());
            
            goto exit_lp;
        }

        //Writeback, first, then eviction
        if (!wr_pol && CacheSets.at(index).SetBlocks.at(k).getDirty() &&
                CacheSets.at(index).SetBlocks.at(k).isValid())
        {
               updateWB();
               updateTotMem();
            if (nextLevel != NULL)
            {
                nextLevel->request(wb_addr,1);
                //if (is_victim)
                //{
                //    CacheSets.at(index).SetBlocks.at(k).updateValid(false);
                //} 
                    
            }
        }

exit_lp:
        CacheSets.at(index).SetBlocks.at(k).updateTag(tag);
        CacheSets.at(index).SetBlocks.at(k).updateValid(true);
        if (rw && !wr_pol)
        {
            if (has_victim)
            {
                if (swp == 1)
                {
                    CacheSets.at(index).SetBlocks.at(k).updateDirty(false);
                }
                else
                    CacheSets.at(index).SetBlocks.at(k).updateDirty(true);
            }
            else
                CacheSets.at(index).SetBlocks.at(k).updateDirty(true);
        }
        else
        {
            CacheSets.at(index).SetBlocks.at(k).updateDirty(false);
        }

        if (!rep_pol) { LRU_cnt_update(index,k);}
        else 
        { 
            CacheSets.at(index).SetBlocks.at(k).updateCnt
                ( CacheSets.at(index).getCntSet() +1 );
        }
       // if (rep_pol)
       // {
       //     CacheSets.at(index).SetBlocks.at
    }

    void LRU_cnt_update(int index, int k)
    {
        int init = CacheSets.at(index).SetBlocks.at(k).getCnt();
                
        CacheSets.at(index).SetBlocks.at(k).updateCnt(0);
        for (int i=0;i<assoc;i++)
        {
            if (i==k) {}
            else
            {
                if (CacheSets.at(index).SetBlocks.at(i).getCnt() < init) 
                { 
                    CacheSets.at(index).SetBlocks.at(i).updateCnt(1 + 
                    CacheSets.at(index).SetBlocks.at(i).getCnt());
                }
            }
        }
    }

    bool checkHit(int index, int tag, int rw)
    {
        bool hit;
        //int k;
        for (int i=0;i<assoc;i++)
        {
            hit = (tag == CacheSets.at(index).SetBlocks.at(i).getTag() 
                    && CacheSets.at(index).SetBlocks.at(i).isValid() == true);
            if (hit)
            {
                if (!rep_pol)
                {
                    LRU_cnt_update(index,i);
                }
                else
                {
                    CacheSets.at(index).SetBlocks.at(i).updateCnt
                        (CacheSets.at(index).SetBlocks.at(i).getCnt() + 1);
                }

                if (rw && !wr_pol)
                {
                    CacheSets.at(index).SetBlocks.at(i).updateDirty(true);
                }

                if (rw && wr_pol)
                {
                    updateTotMem();
                }

                break;
            }

            
        }
        return hit;
    }

    bool check_evict(int index, int* k)
    {
        int t = assoc;
        //Check if any blocks are empty
        for (int i=0;i<assoc;i++)
        {
            if (CacheSets.at(index).SetBlocks.at(i).isValid() == false)
            {
                t=i;
                break;
            }
        }

        if (k) {*k = t;}
        if (t!= assoc)
            return false;
        else return true;
    }

    void enterCache(int addr, int index,int tag, int rw, int swp)
    {
        int k;
        int *p;
        p = &k;
        bool evict = check_evict( index, p);
        //If it is not present, and any block is empty fill the block
        if (!evict)
        {
            updateBlock(addr,index,tag,k,rw,swp);
            //Debug
        }

        //If block is not empty, evict one of the blocks.
        if (evict)
        {
            //LRU eviction code
            if (!rep_pol) 
            { 
                for (int i=0;i<assoc;i++)
                {
                    if (CacheSets.at(index).SetBlocks.at(i).getCnt() == (assoc - 1))
                    {
                        k=i;
                        break;
                    }
                }
            }

            //LFU Eviction Code
            else
            {
                int x=CacheSets.at(index).SetBlocks.at(0).getCnt();
                k=0;
                for (int i=1;i<assoc;i++)
                {
                    if (x > CacheSets.at(index).SetBlocks.at(i).getCnt())
                    {
                        x = CacheSets.at(index).SetBlocks.at(i).getCnt();
                        k=i;
                    }
                }
                CacheSets.at(index).updateCntSet(CacheSets.at(index).SetBlocks.at(k).getCnt());
            }

            
            updateBlock(addr,index,tag,k,rw,swp);
        }
    }

    //void swap()
    //{
        
    int request(int addr,int rw)
    {
        //Exits if the assoc == 0 => cache level does not exist 
        if(assoc == 0)
            return 0;

        int index = (addr & index_mask)/getBlSize();
        //Mask will only get the required bits,
        //division will effectively shift the bits to the right.
        //Division is better because there is no need of using log if 
        //we use division instead of shirt operations.
        
        int tag = (addr & tag_mask)/(getBlSize() * getNumSet());
        //Masking and then shiftingby (index + block offset) bits

        //Check for Hit or Miss
        
        bool hit = checkHit(index,tag,rw);
        int l1_h = hit;
        int l2_h = 0;

        if(hit && !is_victim)
        {
            if(rw)
                updatewrHCnt();
            else 
                updaterdHCnt();
        }

        //If it is a miss, check the type of transaction. 
        if (!hit)
        {
            if(!is_victim)
            {
                if (!rw)
                    updaterdMCnt();
                else
                    updatewrMCnt();
            }
    
            if (has_victim)
            {
                if (rw)
                    victim->updatewrCnt();
                else 
                    victim->updaterdCnt();

                int v_tag = (addr & victim->tag_mask)/bl_size;

                bool v_hit = false;
                int v_bid=victim->getAssoc() + 1;
                int v_dirty;
                for (int i=0;i<victim->getAssoc();i++)
                {
                    if((v_tag == victim->CacheSets.at(0).SetBlocks.at(i).getTag()) 
                    && victim->CacheSets.at(0).SetBlocks.at(i).isValid())
                    {
                        v_bid = i;
                        v_hit = true ;
                        break;
                    }
                }
                //swp = 0, Normal access, not relation to victim cache
                //swp = 1, Swap with victim, no dirty bit
                //swp = 2, Swap with victim, with dirty bit
                //swp = 3, Swap with victim with dirty bit and Cache itself has a write access
                if(v_hit)
                { 
                    if(rw)
                        victim->updatewrHCnt();
                    else
                        victim->updaterdHCnt();
                    //get dirty bit from victim
                    //if (victim->check_evict(0,NULL))
                    victim->CacheSets.at(0).SetBlocks.at(v_bid).updateValid(false); 
                        

                    v_dirty = 1+victim->CacheSets.at(0).SetBlocks.at(v_bid).getDirty() + rw;
                    enterCache(addr,index,tag,1,v_dirty);
                    goto jmp_cnt;

                }

                else
                {
                    if (rw)
                        //(void)0;
                        victim->updatewrMCnt();
                    else
                        victim->updaterdMCnt();
                    int *k;
                    bool test = check_evict(index,k);
                    if (!test) goto load_from_next_level;
                    else
                    {
                        enterCache(addr,index,tag,rw,3);
                        if (nextLevel != NULL)
                        {
                            //Sending request to the next level
                            nextLevel->request(addr,0);
                        }
                        goto jmp_cnt;

                    }
                        
                }

            }
            if (is_victim)
            {
                enterCache(addr,index,tag,rw,0);
                goto jmp_cnt;
            }

        //If it is read, enter cache
        //If it is write, check if WBWA and then enterCache.
load_from_next_level:
            if (!rw ||(rw && !getWrPol()))
            {
                updateTotMem();
                enterCache(addr,index,tag,rw,0);
                //cout << "L2? " << nextLevel << endl;
                if (nextLevel != NULL)
                {
                    //Sending request to the next level
                    l2_h = nextLevel->request(addr,0);
                }
            }
        //If it is write and WTNA
            //else
            //{
            //    updateTotMem();
            //    if (nextLevel != NULL)
            //    {
            //        nextLevel->request(addr,1);
            //    }
            //}

        }
    
jmp_cnt:
        if(!is_victim)
        {
            if (!rw) 
                updaterdCnt();
            else
                updatewrCnt();
        }
        return (l1_h + 2*l2_h);
    }
    void victim_dirty_update(int addr,bool dirty)
    {
        int tag = (addr & tag_mask)/(getBlSize() * getNumSet());
        for (int i=0;i<assoc;i++)
        {
            if (CacheSets.at(0).SetBlocks.at(i).getTag() == tag)
            {
                CacheSets.at(0).SetBlocks.at(i).updateDirty(dirty);
                break;
            }
        }

    }

    void print_info(const string& x)
    {
        cout << "  ===== Simulator Configuration =====  " << endl;
        cout << "  L1_BLOCKSIZE:                    " << bl_size << endl;
        cout << "  L1_SIZE:                         " << size << endl;
        cout << "  L1_ASSOC:                        " << assoc << endl;
        cout << "  L1_REPLACEMENT_POLICY:           " << rep_pol <<endl;
        cout << "  L1_WRITE_POLICY:                 " << wr_pol << endl;
        cout << "  trace_file:                      " << x <<endl;
        cout << "  ===================================  " << endl <<endl ;
        cout << "===== L1 Contents =====" << endl;
    }

    void print_contents()
    {
        if (size == 0)
            return;

        for (int i=0;i<n_set;i++)
        {
            cout << "set " << dec << i << setw(4) << right << ":";
            for (int j=0;j<assoc; j++)
            {
                //int c=0;
                //while(CacheSets.at(i).SetBlocks.at(c).getCnt()!=j) 
                //{
                //    c++;
                //}
                cout << setw(8) << right << hex << CacheSets.at(i).SetBlocks.at(j).getTag() << " ";

                if (CacheSets.at(i).SetBlocks.at(j).getDirty())
                    cout << "D";
                else
                    cout << " ";
            }
            cout << endl;
        }
    }

    void print_raw_op()
    {
        cout << "a. number of accesses :" << dec <<getrdCnt()<<endl;
        cout << "b. number of misses :" << dec << getrdMCnt()<<endl;
    }

    void print_perf_op()
    {
        cout << "  ==== Simulation results (performance) ==== " << endl;
        float miss_p, hit_t;
        miss_p = 20 + 0.5*((float)bl_size/16);
        hit_t = 0.25 + 2.5*((float)size/(512*1024)) + 0.025*((float)bl_size/16) + 0.025*assoc;

        float aat = hit_t + ((float)getMR()*miss_p);

        cout << "  1. average access time:" << setw(10) << right << dec <<aat<<" ns" <<endl;
    }
};

//class Victim: public Cache
//{
//    private:
//
//    public:
//        Victim(int a, int b, Cache* nl)
//        {
//            setVictim();
//            updateBlSize(a);
//            updateSize(b);          
//            updateAssoc(getSize()/getBlSize());
//            
//            setup_Sets();
//            make_Blocks();
//        }
//};
            
#endif
