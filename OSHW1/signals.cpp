#include <iostream>
#include <signal.h>
#include <csignal>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    SmallShell* s=&SmallShell::getInstance();
    std::cout<<"smash: got ctrl-C"<<std::endl;
    if(s->get_for_ground_pid()==-1){
        return;
    }
   
    int p=s->get_for_ground_pid();
    std::cout<<"smash: process "<<p<<" was killed"<<std::endl;
     kill(s->get_for_ground_pid(),SIGKILL);
    s->set_for_ground_pid(-1);
  return;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

