#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <cstring>
#include <map>
#include <deque>


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using std::string;
class JobsList;
class Command {
// TODO: Add your data members

 public:
  Command(const char* cmd_line,bool isalias,string key);
  virtual ~Command();
  virtual void execute()=0;
  int m_argc;
  char ** m_argv;
    bool m_isalias;
    string m_key;
  virtual int prepare() ;
  virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    JobsList* vec_jobs;
    const char * m_line;
 public:
  ExternalCommand(const char* cmd_line,JobsList* jobs,bool isalias,string key);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {

 public:
    std::string m_c1;
    std::string m_c2;
    string m_line;
 PipeCommand(const char* cmd_line);
 virtual ~PipeCommand() {}
 void execute() override;
};

class RedirectionCommand : public Command {

 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() = default;
  void execute() override;
  int prepare() override;
  void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
 public:
  ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ChangeDirCommand()= default;
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand()= default;
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand()= default ;
  void execute() override;
};

class chPromotCommand: public BuiltInCommand{
public:
    chPromotCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    virtual ~chPromotCommand()= default;
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    JobsList* vec_jobs;
public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() = default;
  void execute() override;
};




class JobsList {
 public:
  class JobEntry {
  public:
    int m_job_id;
    pid_t m_pid;
    string m_job_command;
      bool isalias=false;
      string key=" ";


  public:
      JobEntry(int id,pid_t pid, string cm):m_job_id(id),m_pid(pid),m_job_command(cm){};
      ~ JobEntry()=default;
  };
  std::vector<JobEntry> job_list;
  int max_id;

 public:
  JobsList();
  ~JobsList()=default;
  void addJob(string cmd, pid_t pid,bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  int JobNum();
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    JobsList* vec_jobs;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() = default;
  void execute() override;
};

class KillCommand : public BuiltInCommand {
    JobsList* vec_jobs;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand()= default;
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    JobsList* vec_jobs;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() = default;
  void execute() override;
};

class ListDirCommand : public BuiltInCommand {
public:
    ListDirCommand(const char *cmd_line);

    virtual ~ListDirCommand() {}

    void execute() override;
};

class GetUserCommand : public BuiltInCommand {
public:
    GetUserCommand(const char *cmd_line);

    virtual ~GetUserCommand() {}

    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class aliasCommand : public BuiltInCommand {

public:
    aliasCommand(const char *cmd_line);

    virtual ~aliasCommand() {}

    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char *cmd_line);

    virtual ~unaliasCommand() {}

    void execute() override;
};

class SmallShell {
 private:
    std::string m_prompt;
    string cur_dir;
    string prev_dir;
    int for_ground_pid;






  SmallShell();
 public:
    std::string m_line;
    std::string cstring;
    std::string fstring;
    int m_stdout;
    int m_fd;
    int f_time;
    JobsList * m_jobs_v;
    std::map<std::string,std::string> m_allias;
    std::deque<std::string> m_keys;
    void set_prev_dir(const string & path);
    string get_prev_dir();
    void set_for_ground_pid(int p);
    int get_for_ground_pid();
    void set_cur_dir(const string & path);
    string get_cur_dir();
    string get_prompt();
    void  set_prompt( std::string prompt);
  Command *CreateCommand(const char* cmd_line,bool isalias,string key);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};



#endif //SMASH_COMMAND_H_
