#include <iostream>
#include "tasks/SporadicTask.h"
#include "tasks/AperiodicTask.h"
#include "tasks/PeriodicTask.h"
using namespace std;


int main(){


    AperiodicTask aper(1,"NUME",0,20,40,22);
    //to do
    //constructori default 
    //restul claselor
    //supraincarcare la << si >>
    
    cout<<"test makefile "<<aper.getName()<<" "<<aper.getAbsoluteDeadline();
    return 0;
}