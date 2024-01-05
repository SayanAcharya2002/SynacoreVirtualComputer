#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <queue>

#define PROBLEM 695789
#define BEFORE 200

using namespace std;

class OutOfIndexRegister:public exception
{
  int accessing;
  int limit;
  mutable char* message;
public:
  OutOfIndexRegister(int accessing,int limit):accessing(accessing),limit(limit)
  {
    
  }
  const char* what()const noexcept override
  {
    message=new char[100];
    sprintf(message,"Number of registers: %d, but accessing: %d no register\n",limit,accessing);
    return message;
  }
  ~OutOfIndexRegister()
  {
    delete[] message;
  }
};

class StackUnderFlow:public exception
{
  mutable char* message;
public:
  StackUnderFlow()
  {
    
  }
  const char* what()const noexcept override
  {
    message=new char[100];
    sprintf(message,"Stack underflowed");
    return message;
  }
  ~StackUnderFlow()
  {
    delete[] message;
  }
};


class InvalidOpCode:public exception
{
  int code;
  mutable char* message;
public:
  InvalidOpCode(int code):code(code)
  {
    
  }
  const char* what()const noexcept override
  {
    message=new char[100];
    sprintf(message,"Invalid op-code given: %d",code);
    return message;
  }
  ~InvalidOpCode()
  {
    delete[] message;
  }
};

struct vm{
  using T=int;

  int byte=15;
  T mod;
  T tot_regi=8;
  T full=(((T)1)<<byte)-1;
  int max_inst_save=20;

  vector<T> memory=vector<T>((int)1e5,0);
  vector<T> regi=vector<T>(tot_regi,0);
  // T* regi;
  vector<T> inst;
  queue<int> last_some_inst;
  static vector<string> oper;
  static vector<T> args;
  stack<T> st;

  int count=0;
  int index=0;
  vm(vector<T> instruction,int max_inst_save=-1):inst(instruction),index(0),max_inst_save((max_inst_save==-1?20:max_inst_save))
  {
    //instructions are loaded into memory
    for(int i=0;i<instruction.size();++i)
    {
      memory[i]=instruction[i];
    }
    mod=full+1;
    // regi=&memory[0]+mod;
  };
  void restart_machine()
  {
    for(auto& x:regi)
    {
      x=0;
    }
    index=0;
  }
  void run_from_inst_no(int x)
  {
    int _index=x;
    while(_index<inst.size())
    {
      if(!do_op(_index))
      {
        break;
      }
    }
  }
  // void maintatin_queue(int indexing)//generally put the index
  // {
  //   if(last_some_inst.size()==max_inst_save)
  //   {
  //     last_some_inst.pop();
  //   }
  //   last_some_inst.push(indexing);
  // }
  // void show_all_saved_inst()
  // {
  //   while(last_some_inst.size())
  //   {
  //     debug(last_some_inst.front());
  //     last_some_inst.pop();
  //   }
  // }
  void run_machine()
  {
    while(index<inst.size())
    {
      // maintatin_queue(index);
      ++count;
      if(count>=PROBLEM-BEFORE)
      {
        debug(index);
      }
      auto hold_index=index;
      if(!do_op(index))
      {
        break ;
      }
      if(count>=PROBLEM-BEFORE)
      {
        if(is_operation_stack(inst[hold_index]))
        {
          print_stack();
        }
      }
    }
    cout<<"ran for:"<<count<<" instructions \n";
  }

  void debug(T index)
  {
    int x=args[inst[index]];
    cerr<<index<<": ";
    cerr<<oper[inst[index]]<<"("<<inst[index]<<") ";
    if(inst[index]==15)
    {
      cerr<<"(reg"<<get_register(inst[index+1])<<")["<<regi[get_register(inst[index+1])]<<"] ";
      if(is_register(inst[index+2]))
      {
        cerr<<"(reg"<<get_register(inst[index+2])<<")->"<<"{"<<regi[get_register(inst[index+2])]<<"}->["<<give_val_at_mem(inst[index+2])<<"] ";
      }
      else
      {
        cerr<<"mem_"<<inst[index+2]<<"["<<give_val_at_mem(inst[index+2])<<"]";
      }
      cerr<<"\n";
      return ;
    }
    else if(inst[index]==16)
    {
      if(is_register(inst[index+1]))
      {
        cerr<<"(reg"<<get_register(inst[index+1])<<")->{"<<regi[get_register(inst[index+1])]<<"}->["<<give_mem_slot(inst[index+1])<<"] ";
      }
      else
      {
        cerr<<"mem_"<<inst[index+1]<<"["<<give_mem_slot(inst[index+1])<<"] ";
      }
      
      if(is_register(inst[index+2]))
      {
        cerr<<"(reg"<<get_register(inst[index+2])<<")["<<handle_value(inst[index+2])<<"] ";
      }
      else
      {
        cerr<<handle_value(inst[index+2])<<" ";
      }
      cerr<<"\n";
      return ;
    }
    for(int i=1;i<x;++i)
    {
      
      if(is_register(inst[index+i]))
      {
        cerr<<"(reg"<<get_register(inst[index+i])<<")["<<regi[get_register(inst[index+i])]<<"] ";
        continue;
      }
      cerr<<inst[index+i]<<" ";
      if(inst[index]==19 || inst[index]==20)
      {
        cerr<<"("<<(char)inst[index+i]<<")"<<" ";
      }
      cerr<<" ";
    }
    cerr<<"\n";
  }
  void print_stack()
  {
    auto temp_st=st;
    vector<T> v;
    while(temp_st.size())
    {
      v.push_back(temp_st.top());
      temp_st.pop();
    }
    cerr<<"---------------------";
    cerr<<"\n";
    cerr<<"stack: ";
    for(auto it=v.rbegin();it!=v.rend();++it)
    {
      cerr<<*it<<" ";
    }
    cerr<<"\n";
    cerr<<"---------------------";
    cerr<<"\n";
  }
  bool is_operation_stack(T x)
  {
    if(x==2 || x==3 || x==17 || x==18)
    {
      return true;
    }
    return false;
  }
  //returns true if the instruction was successfully done. otherwise, returns false and the program is to be terminated
  bool do_op(int& index)
  {
    T x=inst[index];
    // if(count>=695650)
    // {
    //   debug(index);
    //   if(is_operation_stack(inst[index]))
    //   {
    //     print_stack();
    //   }
    // }
    
    switch(x)
    {
      case 0://ok
      {
        index+=1;
        return false;
        break;
      }
      case 1://maybe ok
      {
        // regi[get_register(inst[index+1])]=inst[index+2];
        // regi[get_register(inst[index+1])]=regi[get_register(inst[index+2])];
        regi[get_register(inst[index+1])]=handle_value(inst[index+2]);

        index+=3;
        break;
      }
      case 2://maybe ok
      {
        // st.push(inst[index+1]);
        // st.push(regi[get_register(inst[index+1])]);
        st.push(handle_value(inst[index+1]));
        index+=2;
        break;
      }
      case 3://ok
      {
        if(st.empty())
        {
          throw StackUnderFlow();
        }
        auto val=st.top();
        regi[get_register(inst[index+1])]=handle_value(val);
        st.pop();
        index+=2;
        break; 
      }
      case 4://now ok
      {
        // regi[get_register(inst[index+1])]=(regi[get_register(inst[index+2])]==regi[get_register(inst[index+3])]);
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])==handle_value(inst[index+3]));
        index+=4;
        break;
      }
      case 5://now ok
      {
        // regi[get_register(inst[index+1])]=(regi[get_register(inst[index+2])]>regi[get_register(inst[index+3])]);
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])>handle_value(inst[index+3]));
        index+=4;
        break;
      }
      case 6:
      {
        // index=inst[index+1];
        index=handle_value(inst[index+1]);
        break;
      }
      case 7:
      {
        if(handle_value(inst[index+1]))
        {
          index=handle_value(inst[index+2]);
        }
        else
          index+=3;
        break;
      }
      case 8:
      {
        if(!handle_value(inst[index+1]))
        {
          index=handle_value(inst[index+2]);
        }
        else
          index+=3;
        break;
      }
      case 9:
      {
        // regi[get_register(inst[index+1])]=(inst[index+2]+inst[index+3])%mod;
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])+handle_value(inst[index+3]))%mod;
        index+=4;
        break;
      }
      case 10://no overflow
      {
        // regi[get_register(inst[index+1])]=(inst[index+2]*inst[index+3])%mod;
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])*handle_value(inst[index+3]))%mod;
        index+=4;
        break;
      }
      case 11:
      {
        // regi[get_register(inst[index+1])]=(inst[index+2]%inst[index+3]);
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])%handle_value(inst[index+3]));
        index+=4;
        break;
      }
      case 12:
      {
        // regi[get_register(inst[index+1])]=(inst[index+2]&inst[index+3])&full;//just to be extra safe
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])&handle_value(inst[index+3]))&full;
        index+=4;
        break;
      }
      case 13:
      {
        // regi[get_register(inst[index+1])]=(inst[index+2]|inst[index+3])&full;//just to be extra safe
        regi[get_register(inst[index+1])]=(handle_value(inst[index+2])|handle_value(inst[index+3]))&full;
        index+=4;
        break;
      }
      case 14:
      {
        // regi[get_register(inst[index+1])]=full^inst[index+2];
        regi[get_register(inst[index+1])]=(full^handle_value(inst[index+2]))&full;
        index+=3;
        break;
      }
      case 15:
      {
        regi[get_register(inst[index+1])]=give_val_at_mem(inst[index+2]);
        index+=3;
        break;
      }
      case 16://wmem a b: b->a : here b is the register or literal
      {
        give_mem_slot(inst[index+1])=handle_value(inst[index+2]);
        index+=3;
        break;
      }
      case 17:
      {
        st.push(index+2);
        index=handle_value(inst[index+1]);
        break;
      }
      case 18:
      {
        if(st.empty())
        {
          index+=1;
          return false;
        }
        auto val=st.top();
        st.pop();
        index=handle_value(val);
        break;
      }
      case 19:
      {
        // cout<<char(regi[get_register(inst[index+1])]);
        cout<<char(handle_value(inst[index+1]));
        index+=2;
        break;
      }
      case 20:
      {
        char c;
        cin>>c;
        regi[get_register(inst[index+1])]=c;
        index+=2;
        break;
      }
      case 21:
      {
        index+=1;
        break;
      }
      default:
      {
        throw InvalidOpCode(x);
      }
    }
    return true;
  }
  bool valid_number(T x)
  {
    return x<=full+8;
  }
  bool is_register(T x)
  {
    return x>=mod;
  }
  bool is_literal(T x)
  {
    return x<=full;
  }
  int get_register(T x)
  {
    if(!is_register(x))
    {
      throw OutOfIndexRegister(x,tot_regi);
    }
    return x-mod;
  }
  T handle_value(T x)
  {
    if(is_literal(x))
    {
      return x;
    }
    else
    {
      return regi[get_register(x)];
    }
  }
  T give_val_at_mem(T x)
  {
    if(is_register(x))
    {
      return memory[regi[get_register(x)]];
    }
    else
    {
      return memory[x];
    }
  }
  T& give_mem_slot(T x)
  {
    if(is_register(x))
    {
      return memory[regi[get_register(x)]];
    }
    else
    {
      return memory[x];
    }
  }
};
vector<string> vm::oper={"halt","set","push","pop","eq","gt","jmp","jnonzero","jzero","add","mult","mod","and","or","not","rmem","wmem","call","ret","out","in","noop"};

vector<vm::T> vm::args={
  1,3,2,2,4,4,2,3,3,4,4,4,4,4,3,3,3,2,1,2,2,1
};


using T=int;
vector<T> read_all(FILE* fp)
{
  vector<T> ans;
  int count=0;
  while(count<30050)
  {
    ++count;
    // cout<<count++<<"\n";
    int val=0;
    int ret=fread(&val,1,2,fp);
    if(ret==-1)
    {
      break;
    }
    // cout<<val<<'\n';
    ans.push_back(val);
  }
  return ans;
}
int main()
{
  // vector<int> inst={9,32768,32769,4,19,32768,0};
  // vm machine(inst);
  // machine.run_machine();
  FILE *fp=fopen("challenge1.bin","rb");
  if(!fp){
    cerr<<"file not there"<<"\n";
    exit(-1);
  }
  // fseek(fp,0,SEEK_END);
  // cout<<ftell(fp)<<"\n";
  // getchar();

  vm machine(read_all(fp),100);
  // machine.run_from_inst_no(1730);
  machine.run_machine();
  // machine.show_all_saved_inst();
  // int x=1;
  // cout<<(int)*(char*)(&x)<<"\n";

  return 0;
}