#pragma once
#include <string>
using namespace std;


enum class TaskState { // starile in care pot fi fiecare task, ready running si finished e destul de evident ce semnifica
    //am ales sa nu implic si starile proceselor ce sunt gata insa sistemul de operare inca nu a gestionat memoria din motiv de simplitate al proiectului
    //blocked poate fi ceva care tine procesul suspendat 
    Ready,
    Running,
    Blocked,
    Finished
};

class Task {
protected:

    int id;
    string name; //ptr logging si debug
    int priority; // voi folosi un max-heap unde voi avea nevoie de campul acesta ptr coada de prioritati
    int WCET; // worst case exec time

    int deadline;//constanta in unitati de timp 


    TaskState state;
    int remaining_time;
    int absolute_deadline; 
protected:
// setterele si getterele le voi scoate in cazul in care nu le voi folosi sau voi vrea sa ascund mai mult detaliile de implementare dupa definirea claselor derivate

    void setPriority(int p);
    void setState(TaskState s);
    void setRemainingTime(int t);
    void setAbsoluteDeadline(int d);

public: 
    Task(int id, const string& name, int priority, int WCET, int deadline);
    virtual ~Task() = 0;

    
    int getId() const;
    const string& getName() const;
    int getPriority() const;
    int getDeadline() const;
    TaskState getState() const;
    int getRemainingTime() const;
    int getAbsoluteDeadline() const;


    virtual bool isReadyAt(int current_time) const = 0;
    virtual void release(int current_time) = 0; //difera per tip de task, spre ex dau handle la debounce diferit ptr fiecare clasa
    virtual string getType() const = 0; 
};