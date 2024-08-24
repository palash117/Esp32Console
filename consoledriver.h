#include<iostream>
#include <vector>
#include <map>
#include <functional>
#include <string>
#include <consoleversion.h>
#include <timeprovider.h>
using namespace std;

std::string concatStrings(std::vector<std::string> v){
  std::string str = "";
  for(std::string &s : v){
    str = str + s;
  }
  return str;
}
class BackHandler{
  public:
  virtual void handleBack()=0;
  virtual void handleMenu()=0;
  virtual void handleBackWithQuery(std::string, std::string) = 0;
};

class DisplayWord{
  public:
  int marginLeft=0;
  int marginRight=0;
  std::string str="";
  std::string seperator="";
  std::string prefix="";
  std::string suffix="";
  bool selected =false;
  DisplayWord(){
  };
  DisplayWord(std::string strval){
    str=strval;
  }
  void setMargins(int ml, int mr){
    marginLeft = ml;
    marginRight = mr;
  }
  void setPrefix(std::string val){
    prefix=val;
  }
  void setSuffix(std::string val){
    suffix=val;
  }
  
};
class DisplayLine{
  public:
  std::vector<DisplayWord> words={};
  std::string seperator="";
  bool selected = false;
  bool bold = false;
  DisplayLine(){};
  DisplayLine(std::string str){
    DisplayWord wrd = DisplayWord(str);
    addDisplayWord(wrd);
  };
  DisplayLine(std::string str, bool isBold, bool isSelected){
    DisplayWord wrd = DisplayWord(str);
    addDisplayWord(wrd);
    bold= isBold;
    selected = isSelected;
  };
  DisplayLine(DisplayWord word){
    addDisplayWord(word);
  };
  void setSeperator(std::string sptr){
    seperator = sptr;
  }
  void addDisplayWord(DisplayWord obj){
    words.push_back(obj);
  }
};
class DisplayPage{
  public:
  DisplayLine topbar;
  std::vector<DisplayLine> lines={};
  DisplayPage(){};
  DisplayPage(std::string str){
    DisplayWord word = DisplayWord(str);
    DisplayLine line = DisplayLine(word);
    addDisplayLine(line);
  };
  void addTopBar(DisplayLine ln){
    topbar = ln;
  }
  void addDisplayLine(DisplayLine line){
    lines.push_back(line);
  };
};

DisplayLine getDateLine(){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y",timeinfo);

  std::string str(buffer);
  DisplayLine line = DisplayLine(str);
  return line;
}
DisplayLine getTimeLine(){
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%H:%M:%S",timeinfo);
  std::string str(buffer);
  DisplayLine line = DisplayLine(str);
  return line;
}

class Screen{
  public :
  virtual DisplayPage display()=0;
  virtual void tick()=0;
  virtual void handleLeft()=0;
  virtual void handleRight()=0;
  virtual void handleOk()=0;
  virtual void setBackHandlerRef(BackHandler*)=0;
  virtual std::string screenNme()=0; 

};
class StateMachineState {
  public:
  std::string name="";
  StateMachineState(){};
  StateMachineState(std::string str){
    name = str;
  }
};
class StateMachineEvent{
  public:
  std::string name="";
  StateMachineEvent(){};
  StateMachineEvent(std::string str){
    name = str;
  }

};
StateMachineEvent ok_event = StateMachineEvent("ok");
StateMachineEvent l_event = StateMachineEvent("l");
StateMachineEvent r_event = StateMachineEvent("r");

class StateMachineFunction{
  public :
  virtual void doFx(std::map<std::string,std::string>*)=0;
};
class StateMachinePath {
  public:
  StateMachineState startState;
  StateMachineState endState;
  StateMachineEvent event ;
  StateMachineFunction*  fx;
  StateMachinePath(){
  };
  StateMachinePath(StateMachineState start, StateMachineState end, StateMachineEvent eventStr){
    startState = start;
    endState = end;
    event = eventStr;
  }
  StateMachinePath(StateMachineState start, StateMachineState end, StateMachineEvent eventStr, StateMachineFunction* fxn) {
    startState = start;
    endState = end;
    event = eventStr;
    fx=fxn;
  }
};
class StateMachine {
  public:
  StateMachineState startState;
  StateMachineState currentState;
  std::vector<StateMachineState> states = {};
  std::vector<StateMachinePath> paths = {};
  std::vector<StateMachineEvent> events = {};
  std::map<std::string, std::string> *values;
  StateMachine( std::string startStateStr, std::vector<std::string> otherStates){
    startState = StateMachineState(startStateStr);
    states.push_back(startState);

    for (int i=0;i<otherStates.size();i++){
      std::string statename = otherStates[i];
      if (statename == startState.name){
        continue;
      }
      StateMachineState interimState = StateMachineState(statename);
      states.push_back(interimState);
    }
    currentState = startState;
  }
  void setValues(std::map<std::string, std::string> *m){
    values = m;
  }

  void addPath(std::string startStateStr, std::string endStateStr, StateMachineEvent e){
    StateMachineState ss;
    StateMachineState es;
    for (int i=0;i<states.size();i++){
      StateMachineState stt = states[i];
      if (stt.name == startStateStr){
        ss = stt;
      }
      if (stt.name == endStateStr){
        es = stt;
      }
    }
    StateMachinePath path = StateMachinePath(ss,es,e);
    paths.push_back(path);
  }
  void addPath(std::string startStateStr, std::string endStateStr, StateMachineEvent e, StateMachineFunction* fx){
    StateMachineState ss;
    StateMachineState es;
    for (int i=0;i<states.size();i++){
      StateMachineState stt = states[i];
      if (stt.name == startStateStr){
        ss = stt;
      }
      if (stt.name == endStateStr){
        es = stt;
      }
    }
    StateMachinePath path = StateMachinePath(ss,es,e, fx);
    paths.push_back(path);
  }
  void migrate(StateMachineEvent ev){
    for (int i=0;i<paths.size();i++){
      StateMachinePath p = paths[i];
      if(p.startState.name == currentState.name && p.event.name == ev.name){
        if (p.fx!=NULL){
          p.fx->doFx(values);

        }
        currentState = p.endState;
        break;
      }
    }
  }
  StateMachineState getCurrentState(){
    return currentState;
  }
  void reset(){
    currentState = startState;
  }
};

class GoBackSmFunction: public StateMachineFunction{
  private:
  BackHandler* b;
  public:
  GoBackSmFunction( BackHandler* bh){
    b=bh;
  }
  void doFx(std::map<std::string,std::string>* m){
    b->handleBack();
  }
} ;
class SetQueryAndGoBackSmFunction: public StateMachineFunction{
  private:
  BackHandler* b;
  std::string q;
  std::string v;
  public:
  SetQueryAndGoBackSmFunction( BackHandler* bh, std::string query){
    b=bh;
    q = query;
  }
  void doFx(std::map<std::string,std::string>* m){
    std::string minutestr = m->at("minute");
    std::string secondstr = m->at("second");
    int minutes = stoi(minutestr);
    int seconds = stoi(secondstr);
    seconds = seconds + minutes * 60;
    v = to_string(seconds);
    b->handleBackWithQuery(q,v);
  }
}; 

class EmptySmFunction:public StateMachineFunction{
  public: 
  EmptySmFunction(){};
  void doFx(std::map<std::string,std::string>* m){}
};
class IncrementMinuteSmFunction:public StateMachineFunction{
  public: 
  IncrementMinuteSmFunction(){};
  void doFx(std::map<std::string,std::string>* m){
    std::string minuteStr = m->at("minute");
    int minuteVal = stoi(minuteStr);
    minuteVal++;
    if (minuteVal>59){
      return;
    }
    minuteStr = std::to_string(minuteVal);

    (*m)["minute"]= minuteStr;
  }
};
class IncrementSecondSmFunction:public StateMachineFunction{
  public:
  IncrementSecondSmFunction(){};
  void doFx(std::map<std::string,std::string>* m){
    std::string secondStr = m->at("second");
    int secondVal = stoi(secondStr);
    secondVal++;
    if (secondVal>59){
      return;
    }
    secondStr = std::to_string(secondVal);

    (*m)["second"]= secondStr;
  }
};

class DecrementMinuteSmFunction:public StateMachineFunction{
  public:
  DecrementMinuteSmFunction(){};
  void doFx(std::map<std::string,std::string>* m){

    std::string minuteStr = m->at("minute");
    int minuteVal = stoi(minuteStr);
    minuteVal--;
    if (minuteVal<0){
      return;
    }
    minuteStr = std::to_string(minuteVal);

    (*m)["minute"]= minuteStr;
  }
};
class DecrementSecondSmFunction: public StateMachineFunction{
  public:
  DecrementSecondSmFunction(){};

  void doFx(std::map<std::string,std::string>* m){

    std::string secondStr = m->at("second");
    int secondVal = stoi(secondStr);
    secondVal--;
    if (secondVal<0){
      return;
    }
    secondStr = std::to_string(secondVal);

      (*m)["second"]= secondStr;
    }
  };
class TimerScreen :public Screen{
  private:
  BackHandler* backHandler;
  std::string name = "Timer";
  bool isTimerSet = false;
  bool editMode = false;
  int minutes = 0;
  int seconds = 0;
  std::map<std::string, std::string> values={{"minute","0"},{"second","0"}};
  StateMachine* sm;

  public:
  TimerScreen(BackHandler* b){
    backHandler = b;
    std::vector<std::string> otherStates = { "minute_edit", "second_selected", "second_edit", "ok_selected","cancel_selected"};
    sm = new StateMachine("minute_selected",otherStates);
    EmptySmFunction* emptyFunction = new EmptySmFunction();


    sm->addPath("minute_selected", "second_selected", l_event, emptyFunction);
    sm->addPath("minute_selected", "cancel_selected", r_event, emptyFunction);
    sm->addPath("minute_selected", "minute_edit", ok_event, emptyFunction);
    sm->addPath("minute_edit", "minute_edit", l_event , new IncrementMinuteSmFunction() );
    sm->addPath("minute_edit", "minute_edit", r_event , new DecrementMinuteSmFunction());
    sm->addPath("minute_edit", "minute_selected", ok_event , emptyFunction);
    sm->addPath("second_selected", "ok_selected", l_event, emptyFunction);
    sm->addPath("second_selected", "minute_selected", r_event, emptyFunction);
    sm->addPath("second_selected", "second_edit", ok_event, emptyFunction);
    sm->addPath("second_edit", "second_edit", l_event , new IncrementSecondSmFunction());
    sm->addPath("second_edit", "second_edit", r_event , new DecrementSecondSmFunction());
    sm->addPath("second_edit", "second_selected", ok_event , emptyFunction);
    sm->addPath("ok_selected", "cancel_selected", l_event, emptyFunction);
    sm->addPath("ok_selected", "second_selected", r_event, emptyFunction);
    sm->addPath("ok_selected", "minute_selected", ok_event, new SetQueryAndGoBackSmFunction(backHandler, "set_timer"));
    sm->addPath("cancel_selected", "minute_selected", l_event, emptyFunction);
    sm->addPath("cancel_selected", "ok_selected", r_event, emptyFunction);
    sm->addPath("cancel_selected", "minute_selected", ok_event, new GoBackSmFunction(backHandler));
   sm->setValues(&values);



  }
  void tick(){};
  DisplayPage display(){
  std::string currentState  = sm->getCurrentState().name;
    DisplayPage page = DisplayPage();
    DisplayWord minuteWord = DisplayWord(values["minute"]);
    DisplayWord secondWord = DisplayWord(values["second"]);
    DisplayWord okWord = DisplayWord("ok");
    DisplayWord cancelWord = DisplayWord("cancel");
    cout<< "debug"<<"\n";
    cout<<"state is "<<currentState<<"\n";
    if(currentState=="minute_selected" ){ 
      cout<<"updating for minute_selected";
      minuteWord.str = concatStrings({"[",values["minute"], "]"});
    }else if (currentState == "minute_edit"){
      minuteWord.str = concatStrings({">",values["minute"], "<"});
    }else if (currentState == "second_selected"){
      secondWord.str = concatStrings({"[",values["second"], "]"});
    }else if (currentState == "second_edit"){
      secondWord.str = concatStrings({">",values["second"], "<"});
    }else if (currentState == "ok_selected"){
      okWord.str = concatStrings({"[","ok", "]"});
    }else if (currentState == "cancel_selected"){
      cancelWord.str = concatStrings({"[","cancel", "]"});
    }

    DisplayLine lin1 = DisplayLine();
    lin1.addDisplayWord(minuteWord);
    lin1.addDisplayWord(secondWord);
    lin1.setSeperator(" : ");

    DisplayLine lin2 = DisplayLine();
    lin2.addDisplayWord(okWord);
    lin2.addDisplayWord(cancelWord);
    lin2.setSeperator(" ");

    cout<< minuteWord.str<< " and " <<secondWord.str<<"\n";
    
    page.addDisplayLine(lin1);
    page.addDisplayLine(lin2);

    return page;
  }
  void handleLeft(){
    sm->migrate(l_event);
    return ;
  };
  void handleRight(){
    sm->migrate(r_event);
    return ;
  };
  void handleOk(){
    sm->migrate(ok_event);
    return ;
  };
  void setBackHandlerRef(BackHandler* ref ){
    backHandler = ref;
  };
  std::string screenNme(){
    return name;
  }
};
class HomeScreen :public Screen{
  private:
  BackHandler* backHandler;
  std::string name = "Home";

  public:
  void tick(){};
  DisplayPage display(){
    DisplayPage page = DisplayPage();
    DisplayLine tmLine = getTimeLine();
    tmLine.bold = true;
    page.addDisplayLine(tmLine);
    page.addDisplayLine(getDateLine());
    return page;
  }
  void handleLeft(){
    return ;
  };
  void handleRight(){
    return ;
  };
  void handleOk(){
    backHandler->handleMenu();
  };
  void setBackHandlerRef(BackHandler* ref ){
    backHandler = ref;
  };
  std::string screenNme(){
    return name;
  }
};
class SwInitSmFunction: public StateMachineFunction{
  public:
  SwInitSmFunction(){};

  void doFx(std::map<std::string,std::string>* m){
      (*m)["sw_set"]= "false";
      (*m)["sw_elapsed"]= "0";
      (*m)["sw_started_at"]= "0";
    }
  };

class SwStartSmFunction: public StateMachineFunction{
  public:
  SwStartSmFunction(){};

  void doFx(std::map<std::string,std::string>* m){
      (*m)["sw_set"]= "true";
      (*m)["sw_elapsed"]= "0";
      (*m)["sw_started_at"]= to_string(getTimeEpoch());
    }
  };

class SwPauseSmFunction: public StateMachineFunction{
  public:
  SwPauseSmFunction(){};

  void doFx(std::map<std::string,std::string>* m){
    
    std::string elapsedStr = m->at("sw_elapsed");
    std::string startedAtStr = m->at("sw_started_at");
    
      int elapsed = stoi(elapsedStr);
      unsigned long startedAt = stoul(startedAtStr);
      unsigned long now = getTimeEpoch();
      elapsed = elapsed + int(now - startedAt);
      (*m)["sw_elapsed"]= to_string(elapsed);
      (*m)["sw_set"]= "false";
    }
  };

class SwResumeSmFunction: public StateMachineFunction{
  public:
  SwResumeSmFunction(){};

  void doFx(std::map<std::string,std::string>* m){
    
      (*m)["sw_set"]= "true";
    (*m)["sw_started_at"]= to_string(getTimeEpoch());
  }
};

class StopwatchScreen :public Screen{
  private:
  BackHandler* backHandler;
  std::string name = "StopWatch";
  bool isTimerSet = false;
  bool editMode = false;
  int minutes = 0;
  int seconds = 0;
  std::map<std::string, std::string> values={};
  StateMachine* sm;

  public:
  StopwatchScreen(BackHandler* b){
    backHandler = b;
    std::vector<std::string> otherStates = {  "watch_back_selected","watch_running_pause_selected", "watch_paused_resume_selected","watch_paused_reset_selected","watch_paused_back_selected", "watch_running_reset_selected","watch_running_back_selected"};
    sm = new StateMachine("watch_start_selected",otherStates);
    EmptySmFunction* emptyFunction = new EmptySmFunction();

    StateMachineFunction *goBack = new GoBackSmFunction(backHandler); 
    SwInitSmFunction *initFx = new SwInitSmFunction();
    SwPauseSmFunction *pauseFx = new SwPauseSmFunction();
    SwResumeSmFunction *resumeFx = new SwResumeSmFunction();
    SwStartSmFunction *startFx = new SwStartSmFunction();
    initFx->doFx(&values);


    sm->addPath("watch_start_selected", "watch_running_pause_selected", ok_event, startFx);

    sm->addPath("watch_start_selected", "watch_back_selected", r_event, emptyFunction);
    
    sm->addPath("watch_back_selected", "watch_start_selected", l_event, emptyFunction);
    sm->addPath("watch_back_selected", "watch_start_selected", ok_event, goBack);

    //todo fx pause
    sm->addPath("watch_running_pause_selected", "watch_paused_resume_selected", ok_event, pauseFx);
    sm->addPath("watch_running_pause_selected", "watch_running_reset_selected", r_event, emptyFunction);

    sm->addPath("watch_running_reset_selected", "watch_start_selected", ok_event, initFx);
    sm->addPath("watch_running_reset_selected", "watch_running_pause_selected", l_event, emptyFunction);
    sm->addPath("watch_running_reset_selected", "watch_running_back_selected", r_event, emptyFunction);

    sm->addPath("watch_paused_resume_selected", "watch_running_pause_selected", ok_event, resumeFx);
    sm->addPath("watch_paused_resume_selected", "watch_paused_reset_selected", r_event, emptyFunction);
    

    sm->addPath("watch_paused_reset_selected", "watch_start_selected", ok_event, initFx);
    sm->addPath("watch_paused_reset_selected", "watch_paused_resume_selected", l_event, emptyFunction);
    sm->addPath("watch_paused_reset_selected", "watch_paused_back_selected", r_event, emptyFunction);


    sm->addPath("watch_paused_back_selected", "watch_paused_resume_selected", ok_event, goBack);
    sm->addPath("watch_paused_back_selected", "watch_paused_reset_selected", l_event, emptyFunction);


    sm->addPath("watch_running_back_selected", "watch_running_back_selected", ok_event, goBack);
    sm->addPath("watch_running_back_selected", "watch_running_reset_selected", l_event, emptyFunction);
   sm->setValues(&values);



  }
  void tick(){};
  DisplayLine getSwTimeLine(){
    DisplayLine ln =  DisplayLine();
    ln.bold= true;
    std::string elapsedStr = values.at("sw_elapsed");
    std::string startedAtStr = values.at("sw_started_at");
    std::string swSetStr = values.at("sw_set");
    
    int elapsed = stoi(elapsedStr);
    unsigned long startedAt = stoul(startedAtStr);
    unsigned long now = getTimeEpoch();
    if(swSetStr=="true"){
      elapsed = elapsed + int(now - startedAt);
    }
    ln.addDisplayWord(DisplayWord(to_string(int(elapsed/3600))));
    ln.addDisplayWord(DisplayWord(to_string(int(elapsed/60)%60)));
    ln.addDisplayWord(DisplayWord(to_string(int(elapsed)%60)));
    ln.seperator = ":";
    ln.bold =true;
    return ln;
  }
  DisplayPage display(){
  std::string currentState  = sm->getCurrentState().name;
    DisplayPage page = DisplayPage();
    /*

    std::vector<std::string> otherStates = {  "watch_back_selected","watch_running_pause_selected", "watch_paused_resume_selected","watch_paused_reset_selected","watch_paused_back_selected", "watch_running_reset_selected","watch_running_back_selected"};
    sm = new StateMachine("watch_start_selected",otherStates);

    */
    if(currentState=="watch_start_selected" ){ 
      DisplayLine ln = DisplayLine("start");
      ln.selected = true;
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    }else if (currentState == "watch_back_selected"){

      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(DisplayLine(DisplayWord("start")));
      page.addDisplayLine(ln);
    }else if (currentState == "watch_running_pause_selected"){

      DisplayLine ln = DisplayLine("pause");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    }else if (currentState == "watch_paused_resume_selected"){

      DisplayLine ln = DisplayLine("resume");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    }else if (currentState == "watch_paused_reset_selected"){


      DisplayLine ln = DisplayLine("reset");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("resume")));
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    }else if (currentState == "watch_paused_back_selected"){
      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("resume")));
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(ln);
    }else if (currentState == "watch_running_reset_selected"){
      DisplayLine ln = DisplayLine("reset");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("pause")));
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    }else if (currentState == "watch_running_back_selected"){
      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("pause")));
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(ln);
    }


    return page;
  }
  void handleLeft(){
    sm->migrate(l_event);
    return ;
  };
  void handleRight(){
    sm->migrate(r_event);
    return ;
  };
  void handleOk(){
    sm->migrate(ok_event);
    return ;
  };
  void setBackHandlerRef(BackHandler* ref ){
    backHandler = ref;
  };
  std::string screenNme(){
    return name;
  }
};
class AboutScreen :public Screen{
   private:
  BackHandler* backHandler;
  std::string name = "About";

  public:
  void tick(){};
  DisplayPage display(){
    DisplayPage page = DisplayPage();

    DisplayLine l1 =  DisplayLine(DisplayWord("Version:"));
    l1.addDisplayWord(DisplayWord(version));
    page.addDisplayLine(l1);
    return page;
  }
  void handleLeft(){
    return ;
  };
  void handleRight(){
    return ;
  };
  void handleOk(){
    backHandler->handleMenu();
  };
  void setBackHandlerRef(BackHandler* ref ){
    backHandler = ref;
  };
  std::string screenNme(){
    return name;
  }
};


class Console: public BackHandler{
  private:
  Screen* selectedScreen;
  std::vector<Screen*> screens;
  bool isMenuSelected= false;
  int menuIndex = 0;
  std::map<std::string,std::string> consoleValues;
  bool displayOff = false;
  

  public:

  void handleQuery(std::string query, std::string value){
    if(query == "set_timer"){
      consoleValues["timer"]=value;
    }
  }

  void handleBackWithQuery(std::string query, std::string value){
    handleQuery(query,value);
    handleMenu();
    return ;
  }

  void handleBack(){
    handleMenu();
    return ;
  }
  void handleMenu(){
    isMenuSelected = true;
    return;
  }

  void addScreen(Screen* scr){
    scr->setBackHandlerRef(dynamic_cast<BackHandler*> (this));
    screens.push_back(scr);
    selectedScreen = screens[0];
  }
  /*   menu functions */

  DisplayPage menuDisplay(){
    //todo
    DisplayPage page = DisplayPage();
    for (int i=0;i<screens.size();i++){
      DisplayLine line = DisplayLine();
      DisplayWord word = DisplayWord(screens[i]->screenNme());
      line.addDisplayWord(word);
      if(i==menuIndex){
        line.selected= true;
        line.bold= true;
      }
      page.addDisplayLine(line);
    }
    return page;

  }
  void menuHandleLeft(){
    if (menuIndex<=0){
      menuIndex=0;
    }else{
      menuIndex = menuIndex-1;
    }
    selectedScreen = screens[menuIndex];
  }

  void menuHandleRight(){
    if (menuIndex>=(screens.size()-1)){
      menuIndex = screens.size()-1;
    }else{
      menuIndex = menuIndex+1;
    }
    selectedScreen = screens[menuIndex];
  }

  void menuHandleOk(){
    isMenuSelected = false;
  };

  void tick(){
    if (isMenuSelected){
      return;
    }
    selectedScreen->tick();
  };


  DisplayPage display(){
    if (displayOff){
      return DisplayPage();
    }
    if (isMenuSelected){
      return addTopBar(menuDisplay());
    }
    return addTopBar(selectedScreen->display());
  }
  DisplayPage addTopBar(DisplayPage pg){
    DisplayLine tp = DisplayLine();
    std::string header = selectedScreen->screenNme();
    if (isMenuSelected){
      header ="Menu";
    }
    tp.bold=true;
    DisplayWord cnsl = DisplayWord(header);
    cnsl.setMargins(0,0);

    
    tp.addDisplayWord(cnsl);
    
    if (consoleValues.count("timer") && consoleValues.at("timer")!=""){
      DisplayWord tmr = DisplayWord("T");
      tp.addDisplayWord(tmr);
    }
    pg.addTopBar(tp);
    
    return pg;


  }
  


  void handleLeft(){
    if (isMenuSelected){
      return menuHandleLeft();
    }
    selectedScreen->handleLeft();
  }

  void handleRight(){
    if (isMenuSelected){
      return menuHandleRight();
    }
    selectedScreen->handleRight();
  }

  void handleOk(){
    if(displayOff){
      displayOff=false;
      return;
    }
    if (isMenuSelected){
      return menuHandleOk();
    }
    selectedScreen->handleOk();
  };

  void handleOkLong(){
    displayOff= !displayOff;
  }
};


Console* setupConsole(){
  setupTime();
  Console* console = new Console();
  HomeScreen*  homeScreen = new HomeScreen();
  TimerScreen* timerScreen = new TimerScreen(console);
  AboutScreen* aboutScreen = new AboutScreen();
 StopwatchScreen* swScreen = new StopwatchScreen(console);
  console->addScreen(homeScreen);
  console->addScreen(timerScreen);
 console->addScreen(swScreen);
  console->addScreen(aboutScreen);
  return console;
}
