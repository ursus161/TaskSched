#include <iostream>
#include "SporadicTask.h"
#include "AperiodicTask.h"
#include "PeriodicTask.h"
using namespace std;


int main(){


    AperiodicTask aper(1,"NUME",0,20,40,22);
    //to do
    //constructori default 
    //supraincarcare la << si >>
    
    cout<<"test makefile "<<aper.getName()<<" "<<aper.getAbsoluteDeadline();
    return 0;
}