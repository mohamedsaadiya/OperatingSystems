
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <cstdlib>
#include <cstdio>
#include <sys/io.h>
#include <dirent.h>
#include <algorithm>
#include <pwd.h>
#include<grp.h>
#define _GNU_SOURCE
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <regex>
#include <string>
#include <cstring>

const std::string WHITESPACE = "\n\r\t\f\v";

using namespace std;

int Command::prepare() {
    return 0;
}

void Command::cleanup() {
    return;
}


#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif
bool  isNum(const char* num);
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}
string _rtrim_equal(const std::string &s) {
    size_t end = s.find_last_not_of('=');
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
string _ltrim_equal(const std::string &s) {
    size_t start = s.find_first_not_of('=');
    return (start == std::string::npos) ? " " : s.substr(start);
}

string _rtrim_pa(const std::string &s) {
    size_t end = s.find_last_not_of('/');
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
string _ltrim_pa(const std::string &s) {
    size_t start = s.find_first_not_of("/'");
    return (start == std::string::npos) ? " " : s.substr(start);
}
int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell()  {
    this->m_prompt= "smash";
    this->cur_dir="";
    this->prev_dir=" ";
    this->for_ground_pid=-1;
    this->m_jobs_v=new JobsList();
    this->f_time=0;
    this-> m_line="";
    this->cstring="";
    this->fstring="";
    this->m_stdout;
    this->m_fd;



}

SmallShell::~SmallShell() {
    delete m_jobs_v;


}

BuiltInCommand::BuiltInCommand(const char *cmd_line):Command(cmd_line,false," ") {}

string SmallShell::get_prompt() {
    return this->m_prompt;
}

void SmallShell::set_prompt(string prompt) {
    this->m_prompt=prompt;
}

string SmallShell::get_prev_dir() {
    return this->prev_dir;
}

void SmallShell::set_prev_dir(const string & path) {
    this->prev_dir=path;
}
void SmallShell::set_cur_dir(const string  & path){
    this->cur_dir=path;
}
string SmallShell::get_cur_dir(){
    return this->cur_dir;
}

void SmallShell::set_for_ground_pid(int p){
    this->for_ground_pid=p;
}
int SmallShell::get_for_ground_pid(){
    return this->for_ground_pid;
}


Command * SmallShell::CreateCommand(const char* cmd_line,bool isalias,string key) {
    //string firstWord;
    SmallShell* s=&SmallShell::getInstance();
    string  cmd_t= _trim(string(cmd_line));
    if(cmd_t.find_first_not_of(WHITESPACE) == string::npos)
    {
        return nullptr;
    }
    char line[200];
    strcpy(line, cmd_line);
    _removeBackgroundSign(line);
    string cmd_s = _trim(string(line));
    if(cmd_s.find('>')!= string::npos || cmd_s.find(">>")!= string::npos){
        return new RedirectionCommand(cmd_line);
    }
    if(cmd_s.find("|&")!= std::string::npos || cmd_s.find("|")!= std::string::npos) {

        return new PipeCommand(cmd_line);
    }


    if(cmd_s.find_first_not_of(WHITESPACE) == string::npos)
    {
        return nullptr;
    }
    std::string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (firstWord.compare("chmod")==0){
        return new ChmodCommand(cmd_s.c_str());
    }

    if (firstWord.compare("chprompt")==0) {
        return new chPromotCommand(cmd_s.c_str());
    }

    if(firstWord.compare("showpid")==0){
        return new ShowPidCommand(cmd_s.c_str());
    }

    if (firstWord.compare("pwd")==0){
        return new GetCurrDirCommand(cmd_s.c_str());
    }
    if( firstWord.compare("cd")==0){
        return new ChangeDirCommand(cmd_s.c_str());
    }
    if(firstWord.compare("jobs")==0){
        return new JobsCommand(cmd_s.c_str(),s->m_jobs_v);
    }
    if (firstWord.compare("fg")==0){
        return new ForegroundCommand(cmd_s.c_str(),s->m_jobs_v);
    }
    if(firstWord.compare("quit")==0){
        return new QuitCommand(cmd_s.c_str(),s->m_jobs_v);
    }
    if (firstWord.compare("kill")==0){
        return new KillCommand(cmd_s.c_str(),s->m_jobs_v);
    }
    if (firstWord.compare("getuser")==0) {
        return new GetUserCommand(cmd_s.c_str());
    }
    if (firstWord.compare("listdir")==0) {

        return new ListDirCommand(cmd_line);
    }
    if (firstWord.compare("alias")==0){
        return new aliasCommand(cmd_line);
    }
    if (firstWord.compare("unalias")==0){
        return new unaliasCommand(cmd_s.c_str());
    }
    if(s->m_allias.find(firstWord)!= s->m_allias.end()) {
       std::string value=m_allias.at(firstWord);
        string str=string(cmd_line).substr(firstWord.size());
        string newline=value+str;
        return s->CreateCommand(&newline[0],true,cmd_line);
    }
    if(isalias) {
        return new ExternalCommand(cmd_line,s->m_jobs_v,true,key);
    }
    return new ExternalCommand(cmd_line,s->m_jobs_v,false," ");

}

void SmallShell::executeCommand(const char *cmd_line) {


    Command* cmd;

    SmallShell* s=&SmallShell::getInstance();
    cmd = CreateCommand(cmd_line,false," ");
    if(cmd == nullptr) {
        return;
    }
    bool b;
    b= _isBackgroundComamnd(cmd_line);
    s->m_jobs_v->removeFinishedJobs();


    cmd->execute();
    delete cmd;
    if(!b) {
      s->set_for_ground_pid(-1);
    }

}

Command::Command(const char *cmd_line,bool isalias,string key) {
    this->m_isalias=isalias;
    this->m_key=key;
    this->m_argv=new char*[20];
    m_argc= _parseCommandLine(cmd_line,m_argv);
}

Command::~Command(){
    delete[] m_argv;
}

void chPromotCommand::execute() {
    if(m_argc==1){
        SmallShell::getInstance().set_prompt("smash");
        return;
    }
    else{
        SmallShell::getInstance().set_prompt(m_argv[1]);
        return;
    }
}

void ShowPidCommand::execute() {
    std::cout<<"smash pid is "<<getpid()<<std::endl;
}

void GetCurrDirCommand::execute() {
    char* buffer=new char[200];
    std::cout<<getcwd(buffer,200)<<std::endl;
    delete[] buffer;
}
aliasCommand::aliasCommand(const char *cmd_line):BuiltInCommand(cmd_line) {};
void aliasCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    if(m_argc==1) {
        for (const std::string& key : s->m_keys) {
            std::cout << key << "='" << s->m_allias[key] << "'" << std::endl;
        }
        return;
    }
    std::string line;
    int i=1;
    while(m_argv[i]) {
        line+=m_argv[i];
        line+=" ";
        i++;
    }
    size_t equal_pos = line.find('=');

    size_t quote_start_pos = line.find('\'');
    size_t quote_end_pos = line.rfind('\'');

    // Extract key and command
    std::string key = line.substr(0, equal_pos); // Extract key
    std::string command = line.substr(quote_start_pos + 1, quote_end_pos - quote_start_pos - 1);
   // std::regex invalid_pattern("[^a-zA-Z_]");

    // Check if the key contains any invalid characters
  //  if (std::regex_search(key, invalid_pattern)) {
  //      std::cerr << m_argv[1]<< std::endl;
     //   return;
  //  }
    for (char c: key) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != '_' &&
            !std::isdigit(static_cast<unsigned char>(c))) {
            std::cerr << "smash error: alias: invalid alias format" << std::endl;
            return;
            }
    }
    if (key=="cd"||key=="chprompt"||key=="showpid"||key=="pwd"||key=="pwd"||key=="jobs"||key=="fg"||key=="quit"||key=="kill"||key=="alias"||key=="unalias") {
        std::cerr<<"smash error: alias: "<<key<<" already exists or is a reserved command"<<std::endl;
        return;
    }
    if(s->m_allias.find(key)!= s->m_allias.end()) {
        std::cerr<<"smash error: alias: "<<key<<" already exists or is a reserved command"<<std::endl;
        return;
    }

    s->m_allias[key]=command;
    s->m_keys.push_back(key);
}
unaliasCommand::unaliasCommand(const char *cmd_line):BuiltInCommand(cmd_line) {};
void unaliasCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    if(m_argc==1) {
        std::cerr<<"smash error: unalias: not enough arguments"<<std::endl;
        return;
    }
    int i=1;
    while (m_argv[i]) {
        std::string key =m_argv[i];
        if(s->m_allias.find(key)== s->m_allias.end()) {
            std::cerr<<"smash error: unalias: "<<key<<" alias does not exist"<<std::endl;
            return;
        }
        i++;
    }
    i=1;
    while (m_argv[i]) {
        std::string key =m_argv[i];
        s->m_allias.erase(key);
        i++;
        s->m_keys.erase(std::remove(s->m_keys.begin(), s->m_keys.end(), key), s->m_keys.end());
    }
}
void ChangeDirCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    if(s->f_time ==0){
        char* buffer=new char[200];
        string  str= getcwd(buffer,200);
        s->set_cur_dir( str);
        s->set_prev_dir(str);
        delete[] buffer;
        s->f_time=1;

    }


    if(m_argc==1){
        return;
    }
    if(m_argc>2){
        std::cerr<<"smash error: cd: too many arguments\n";
        return;
    }
    if(string (m_argv[1]).compare("-") == 0 && SmallShell::getInstance().get_cur_dir()==SmallShell::getInstance().get_prev_dir()){
        std::cerr<<"smash error: cd: OLDPWD not set\n";
        return;
    }

    if (string (m_argv[1]).compare( "-")==0){


        int x=chdir(SmallShell::getInstance().get_prev_dir().c_str());
        if(x==-1) {
            perror("smash error: chdir failed");
            return;
        }
        string pv=  SmallShell::getInstance().get_cur_dir();
        SmallShell::getInstance().set_cur_dir(SmallShell::getInstance().get_prev_dir());
        SmallShell::getInstance().set_prev_dir(pv);
        return;

    }

    int y=chdir(m_argv[1]);
    if (y==-1) {
        perror("smash error: chdir failed");
        return;
    }
    if(string(m_argv[1]).compare("..")==0){
        char* buffer=new char[200];
        getcwd(buffer,200);


        string cv=  SmallShell::getInstance().get_cur_dir();
        SmallShell::getInstance().set_cur_dir(buffer);
        SmallShell::getInstance().set_prev_dir(cv);
        delete[] buffer;
        return;
    }
    string cv=  SmallShell::getInstance().get_cur_dir();
    char* buffer=new char[200];
    getcwd(buffer,200);
    SmallShell::getInstance().set_cur_dir(buffer);
    SmallShell::getInstance().set_prev_dir(cv);
    delete[] buffer;
    return;


}
JobsList::JobsList(): job_list(),max_id(0) {}

void JobsList::addJob(string cmd,pid_t pid, bool isStopped ) {

    removeFinishedJobs();
    JobEntry j(max_id+1,int(pid),cmd);
    job_list.emplace_back(j);
    max_id++;

    return;

}

void JobsList::removeFinishedJobs() {
    SmallShell* s=&SmallShell::getInstance();
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        pid_t x = waitpid(it->m_pid, nullptr,WNOHANG);
        if(x==-1){
            job_list.erase(it);
            --it;

        }
        if(x==it->m_pid){
            if (it->m_job_id==this->max_id){
                this->max_id=0;
            }
            s->m_jobs_v->removeJobById(it->m_job_id);
            --it;
        }

    }
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        if(it->m_job_id> max_id){
            max_id=it->m_job_id;
        }

    }
    return;
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        std::cout<<"[" << it->m_job_id << "]"<<" " << it->m_job_command<<std::endl;

    }

}

void JobsList::removeJobById(int jobId){
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {

        if(it->m_job_id==jobId){
            if(it->m_job_id==max_id) {
                max_id = 0;
            }
            job_list.erase(it);
            break;
        }

    }

    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        if(it->m_job_id> max_id){
            max_id=it->m_job_id;
        }

    }
}

JobsList::JobEntry * JobsList::getJobById(int jobId){
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        if(it->m_job_id==jobId){
            return &(*it);
        }

    }
    return nullptr;
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {

    *lastJobId= job_list.back().m_job_id;
    return  &job_list.back();
}



JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),vec_jobs(jobs) {
}
void JobsCommand::execute() {
    if(this->vec_jobs!= nullptr){
        this->vec_jobs->removeFinishedJobs();
        this->vec_jobs->printJobsList();
    }
    return;
}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),vec_jobs(jobs){};

void ForegroundCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    s->m_jobs_v->removeFinishedJobs();
   /* if(stoi(this->m_argv[1])<1)
    {
        std::cerr << "smash error: fg: invalid arguments\n";
        return;
    }*/
    if(this->m_argc>=2){
        if (this->m_argc > 2 ) {
            std::cerr << "smash error: fg: invalid arguments\n";
            return;
        }
        if(!isNum(this->m_argv[1])){
            std::cerr << "smash error: fg: invalid arguments\n";
            return;
        }
        int jobid=std::stoi(m_argv[1]);
        if(jobid<1)
        {
            std::cerr << "smash error: fg: invalid arguments\n";
            return;
        }
        if (this->vec_jobs->getJobById(stoi(this->m_argv[1])) == nullptr) {
            std::cerr << "smash error: fg: job-id "<< stoi(this->m_argv[1])<< " does not exist\n";
            return;
        }
    }
    if (s->m_jobs_v->job_list.empty()&& this->m_argc == 1) {
        std::cerr << "smash error: fg: jobs list is empty\n";
        return;
    }
    if (this->m_argc > 2 ) {
        std::cerr << "smash error: fg: invalid arguments\n";
        return;
    }


    if (this->m_argc == 1) {
        std::cout<< this->vec_jobs->getJobById(this->vec_jobs->max_id)->m_job_command<<" "<<this->vec_jobs->getJobById(this->vec_jobs->max_id)->m_pid<<"\n";
        s->set_for_ground_pid(this->vec_jobs->getJobById(this->vec_jobs->max_id)->m_pid);
        int p=this->vec_jobs->getJobById(this->vec_jobs->max_id)->m_pid;
        this->vec_jobs->removeJobById(this->vec_jobs->max_id);
        if(waitpid(p, nullptr,WUNTRACED)==-1){
            perror("smash error: waitpid failed");
            return;
        }



        s->set_for_ground_pid(-1);
        return;
    }
    if (!isNum(m_argv[1]) ) {
        std::cerr << "smash error: fg: invalid arguments\n";
        return;
    }
    int x = stoi(this->m_argv[1]);






    JobsList::JobEntry* temp=this->vec_jobs->getJobById(x);
    pid_t pid=temp->m_pid;
    s->set_for_ground_pid(pid);
    std::cout<<temp->m_job_command<<" " <<temp->m_pid<<"\n";
    this->vec_jobs->removeJobById(x);
    if(waitpid(pid, nullptr,WUNTRACED)==-1){
        perror("smash error: waitpid failed");
        return;
    }

    s->set_for_ground_pid(-1);

    return;

}

void JobsList::killAllJobs() {
    for(auto it = job_list.begin(); it != job_list.end(); ++it)
    {
        pid_t p=it->m_pid;
        int x=kill(p,SIGKILL);
        if(x == -1){
            perror("smash error: kill failed");
            return;
        }

    }
    return;
}

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line),vec_jobs(jobs) {}

void QuitCommand::execute() {
    vec_jobs->removeFinishedJobs();
    if(m_argc==1){
        exit(0);
    }
    std::cout<<"smash: sending SIGKILL signal to "<< vec_jobs->JobNum()<< " jobs:\n";
    vec_jobs->killAllJobs();

    for (auto it = vec_jobs->job_list.begin(); it != vec_jobs->job_list.end(); ++it) {
        std::cout << it->m_pid << ":" << " "<< it->m_job_command << std::endl;
        vec_jobs->removeJobById(it->m_job_id);
        --it;

    }
    exit(0);


    return;

}

int JobsList::JobNum() {
    int i=0;
    for (auto it = job_list.begin(); it != job_list.end(); ++it) {
        i++;
    }
    return i;

}
bool isNum(const char* num){
    int x;
    try{
        x= stoi(num);
    }
    catch(...){
        return false;
    }
    return true;
}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),vec_jobs(jobs){};

void KillCommand::execute() {
    int x;
    if(this->m_argc > 3)
    {
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;
    }
    if(!isNum(m_argv[1])){
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;
    }
    if(this->m_argc >= 3 && !isNum(m_argv[2]) ){
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;


    }
    if(this->m_argc >= 3) {
        int sig_num= stoi(this->m_argv[1])*-1;
        if( sig_num<0){
            std::cerr<<"smash error: kill: invalid arguments\n";
            return;
        }
        x = stoi(m_argv[2]);
        if(x<1)
        {
            std::cerr<<"smash error: kill: invalid arguments\n";
            return;
        }
        if (!vec_jobs->getJobById(x)) {
            std::cerr << "smash error: kill: job-id " << x << " does not exist\n";
            return;
        }

    }
    if(this->m_argc != 3){
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;
    }

    if(!isNum(m_argv[1])){
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;
    }
    int sig_num= stoi(this->m_argv[1])*-1;
    if( sig_num<0){
        std::cerr<<"smash error: kill: invalid arguments\n";
        return;
    }
    if(kill((this->vec_jobs->getJobById(x)->m_pid),sig_num)== -1){
        perror("smash error: kill failed");
        return;
    }
    std::cout<<"signal number "<<sig_num<< " was sent to pid "<< vec_jobs->getJobById(x)->m_pid<<"\n";

}
ExternalCommand::ExternalCommand(const char *cmd_line, JobsList *jobs,bool isalias,string key): Command(cmd_line,isalias,key),vec_jobs(jobs) {

    m_line=cmd_line;
}


void ExternalCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    char new_line[200];
    strcpy(new_line, m_line);
    char * new_argv[20];
    _parseCommandLine(new_line, new_argv);
    if (_isBackgroundComamnd(m_line)) {
        strcpy(new_line, m_line);
        _removeBackgroundSign(new_line);
        _parseCommandLine(new_line, new_argv);
    }
    pid_t p = fork();

    if (p == 0) {


        if(setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            exit(0);
        }
        string x=string(new_line);
        if ((x.find("?")!=std::string::npos)||(x.find("*")!=std::string::npos)){
            char b[]="/bin/bash";
            char c[]="-c";
            char* const y[4]={b,c,new_line, nullptr};
            if(execv(y[0],y)==-1){
                perror("smash error: execv failed");
                exit(0);
            }

        }
        else{
            if(execvp(new_argv[0],new_argv)==-1){
                perror("smash error: execvp failed");
                exit(0);
            }

        }


    }else{
        if (_isBackgroundComamnd(m_line)){
            SmallShell& smash = SmallShell::getInstance();
            if(m_isalias) {
                smash.m_jobs_v->addJob(m_key,p);
                return;
            }
            smash.m_jobs_v->addJob(m_line,p);
            return;

        }
        else{
            s->set_for_ground_pid(p);
            if(waitpid(p, nullptr, WUNTRACED) == -1)
            {
                perror("smash error: waitpid failed");
                return;
            }
            s->set_for_ground_pid(-1);
        }
    }
}

int RedirectionCommand::prepare() {
    SmallShell* s=&SmallShell::getInstance();
    int fd = fileno(stdout);
   s-> m_stdout= dup(fd);
    if(s->m_stdout==-1) {
        perror("smash error: close failed");
        return 0;
    }
    if(close(fd)==-1){
        perror("smash error: close failed");
        return 0;
    }
    if(s->m_line.find(">>")!=std::string::npos){
       s-> m_fd= open(s->fstring.c_str(), O_WRONLY | O_CREAT | O_APPEND ,0655);

        if(s->m_fd==-1){
            if(dup(s->m_stdout)==-1){
                perror("smash error: dup failed");
                return 0;
            }
            perror("smash error: open failed");
            return 0;

        }


    }else{
        s->m_fd= open(s->fstring.c_str(),O_WRONLY | O_CREAT | O_TRUNC, 0655);
        if(s->m_fd == -1){
            if(dup(s->m_stdout)==-1){
                perror("smash error: dup failed");
                return 0;
            }
            perror("smash error: open failed");
            return 0;

        }
    }



    return 1;
}

void RedirectionCommand::cleanup() {
    SmallShell* s=&SmallShell::getInstance();
    if(close(s->m_fd)==-1){
        perror("smash error: close failed");
        return;
    }
    if(dup(s->m_stdout)==-1){
        perror("smash error: dup failed");
        return;
    }
    if(close(s->m_stdout)==-1){
        perror("smash error: close failed");
        return;
    }
}

RedirectionCommand::RedirectionCommand(const char *cmd_line): Command(cmd_line,false," ") {
    SmallShell* s=&SmallShell::getInstance();
    s->m_line = cmd_line;

    int y = (int)s->m_line.find(">>");
    if (y != std::string::npos) {
        s->cstring = s->m_line.substr(0, y);
        s->fstring = s->m_line.substr(y + 2, s->m_line.size() - (y+2));
       // s->cstring = _trim(s->cstring);
        s->fstring = _trim(s->fstring);
       s-> fstring=s->fstring.substr(1,s->fstring.size());
    }else{
        int x = (int)s->m_line.find('>');
        if (x != std::string::npos) {
            s->cstring = s->m_line.substr(0, x);
            s->fstring = s->m_line.substr(x + 1, s->m_line.size() - (x + 1));
            s->fstring = _trim(s->fstring);
           // s->cstring = _trim(s->cstring);
            s->fstring=s->fstring.substr(1,s->fstring.size());


        }
    }

}


void RedirectionCommand::execute() {
    SmallShell* s=&SmallShell::getInstance();
    if(!prepare()){
        return;
    }
    SmallShell& x=SmallShell::getInstance();
   const char *command= s->cstring.c_str();
    x.executeCommand(command);
    cleanup();





}


ChmodCommand::ChmodCommand(const char *cmd_line) : BuiltInCommand(cmd_line){
}

void ChmodCommand::execute() {
    if(this->m_argc!= 3){
        std::cerr<<"smash error: chmod: invalid arguments \n";
        return;

    }
    mode_t m=stoi(string(m_argv[1]), nullptr,8);
    if(chmod(m_argv[2],m)==-1){
        perror("smash error: chmod failed");
        return;
    }
}

PipeCommand::PipeCommand(const char *cmd_line): Command(cmd_line,false," ") {
    this->m_line=string(cmd_line);


    if(this->m_line.find("|&")!= std::string::npos){
        int index=this->m_line.find("|&");
        this->m_c1=this->m_line.substr(0,index);
        this->m_c2=this->m_line.substr(index+2,string(cmd_line).size());


    }
    else {
        int index = this->m_line.find("|");
        this->m_c1 = this->m_line.substr(0, index);
        this->m_c2 = this->m_line.substr(index + 1, string(cmd_line).size());
    }

}

void PipeCommand::execute() {
    int my_pipe[2];
    SmallShell& s = SmallShell::getInstance();
    if(pipe(my_pipe) == -1)
    {
        perror("smash error: pipe failed");
        return;
    }
    int pid = fork();
    if(pid == -1)
    {
        if (close(my_pipe[0]) == -1)
        {
            perror("smash error: close failed");
        }
        if (close(my_pipe[1]) == -1)
        {
            perror("smash error: close failed");
        }
        perror("smash error: fork failed");
        return;
    }

    if(pid == 0)
    {
        int fd;
        if(setpgrp() == -1)
        {
            if (close(my_pipe[0]) == -1)
            {
                perror("smash error: close failed");
            }
            if (close(my_pipe[1]) == -1)
            {
                perror("smash error: close failed");
            }

            perror("smash error: setprgp failed");

            exit(0);
        }
        if(close(my_pipe[1]) == -1)
        {
            if(close(my_pipe[0]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: close failed");

            exit(0);
        }
        fd=dup(STDIN_FILENO);
        if(fd == -1)
        {
            if(close(my_pipe[0]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: dup failed");
            exit(0);
        }
        if(dup2(my_pipe[0], STDIN_FILENO) == -1)
        {
            if(close(my_pipe[0]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: dup2 failed");

            exit(0);
        }
        s.executeCommand(m_c2.c_str());
        if(close(my_pipe[0]) == -1)
        {
            perror("smash error: close failed");

            exit(0);
        }

        if(close(STDIN_FILENO) == -1)
        {
            perror("smash error: close failed");

            exit(0);
        }
        if(dup(fd) == -1)
        {
            perror("smash error: dup failed");

            exit(0);
        }
        if(close(fd) == -1)
        {
            perror("smash error: close failed");

            exit(0);
        }


        exit(0);
    }
    else
    {
        if(close(my_pipe[0]) == -1)
        {
            if(close(my_pipe[1]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: close failed");
            return;
        }
        int fd;
        int Channel;
        if(this->m_line.find("|&")!= std::string::npos) {
            Channel = STDERR_FILENO;
        }
        else {
            Channel = STDOUT_FILENO;
        }
        fd = dup(Channel);
        if(fd == -1)
        {
            if(close(my_pipe[1]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: dup failed");
            return;
        }
        if(dup2(my_pipe[1], Channel) == -1)
        {
            if(close(my_pipe[1]) == -1)
            {
                perror("smash error: close failed");
            }
            perror("smash error: dup2 failed");

            return;
        }

        s.executeCommand(m_c1.c_str());

        if(close(my_pipe[1]) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        if(close(Channel) == -1)
        {
            perror("smash error: close failed");
            return;
        }
        if(dup(fd) == -1)
        {
            perror("smash error: dup failed");
            return;
        }
        if(close(fd) == -1)
        {
            perror("smash error: close failed");
            return;
        }

        waitpid(pid, nullptr, 0);
    }
}

std::string get_username(uid_t uid) {
    struct passwd* pw = getpwuid(uid);
    return std::string(pw->pw_name);
}

std::string get_groupname(gid_t gid) {
    struct group* gr = getgrgid(gid);
    return std::string(gr->gr_name);
}

void get_user_and_group(pid_t pid) {
    struct stat statbuf;
    std::string proc_path = "/proc/" + std::to_string(pid);

    if (stat(proc_path.c_str(), &statbuf) == -1) {
        std::cerr << "smash error: getuser: process "<< pid <<" does not exist\n";
        return;
    }

    uid_t uid = statbuf.st_uid;
    gid_t gid = statbuf.st_gid;

    std::string username = get_username(uid);
    std::string groupname = get_groupname(gid);

    std::cout << "User: " << username << std::endl;
    std::cout << "Group: " << groupname << std::endl;
}
GetUserCommand::GetUserCommand(const char *cmd_line):BuiltInCommand(cmd_line) {};

void GetUserCommand::execute() {
    if(m_argc>2){
        std::cerr<<"smash error: cd: too many arguments\n";
        return;
    }
    pid_t pid=stoi(this->m_argv[1]);
    get_user_and_group(pid);

}

ListDirCommand::ListDirCommand(const char *cmd_line):BuiltInCommand(cmd_line) {};
void ListDirCommand::execute() {
    if(m_argc>2){
        std::cerr<<"smash error: listdir: too many arguments\n";
        return;
    }
    string path(this->m_argv[1]);
    //string path=s.substr(space);
    int fd=open(path.c_str(),O_RDONLY,0);
    if (!fd)
    {
        perror("smash error: listdir failed");
        exit(0);
    }
    int buf_size=4096;
    char buffer[buf_size];
    vector<string> files_name;
    vector<string> dirs_name;
    int nread;
    while((nread=syscall(SYS_getdents64,fd,buffer,buf_size))>0)
    {
        int bpos=0;
        while(bpos<nread)
        {
            struct dirent* d=(struct dirent*)(buffer+bpos);
            char d_type=*(buffer+bpos+d->d_reclen-1);
            if (std::string(d->d_name) == "." || std::string(d->d_name) == "..") {
                bpos += d->d_reclen;
                continue;
            }
            switch (d_type) {
                case DT_REG:
                        files_name.push_back(d->d_name);
                break;
                case DT_DIR:
                        dirs_name.push_back(d->d_name);
                break;
                default:
                    continue;

            }
            bpos += d->d_reclen;
        }
    }
    sort(files_name.begin(),files_name.end());
    sort(dirs_name.begin(),dirs_name.end());
    std::cout<<"file: ";
    for (const auto& str :files_name ) {
        std::cout<<str<<" ";
    }
    std::cout<<"\n";
    std::cout<<"directory: ";
    for (const auto& str :dirs_name ) {
        std::cout<<str<<" ";
    }
    std::cout<<"\n";
}