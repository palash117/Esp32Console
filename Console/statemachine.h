#include <interfaces.h>
#include <string>
#include <utils.h>
#include <timeprovider.h>

class StateMachineState {
public:
  std::string name = "";
  StateMachineState(){};
  StateMachineState(std::string str) {
    name = str;
  }
};
class StateMachineEvent {
public:
  std::string name = "";
  StateMachineEvent(){};
  StateMachineEvent(std::string str) {
    name = str;
  }
};
StateMachineEvent ok_event = StateMachineEvent("ok");
StateMachineEvent l_event = StateMachineEvent("l");
StateMachineEvent r_event = StateMachineEvent("r");
StateMachineEvent rst_event = StateMachineEvent("rst");

class StateMachineFunction {
public:
  virtual void doFx(std::map<std::string, std::string>*) = 0;
};
class StateMachinePath {
public:
  StateMachineState startState;
  StateMachineState endState;
  StateMachineEvent event;
  StateMachineFunction* fx;
  StateMachinePath(){};
  StateMachinePath(StateMachineState start, StateMachineState end, StateMachineEvent eventStr) {
    startState = start;
    endState = end;
    event = eventStr;
  }
  StateMachinePath(StateMachineState start, StateMachineState end, StateMachineEvent eventStr, StateMachineFunction* fxn) {
    startState = start;
    endState = end;
    event = eventStr;
    fx = fxn;
  }
};
class StateMachine {
public:
  StateMachineState startState;
  StateMachineState currentState;
  std::vector<StateMachineState> states = {};
  std::vector<StateMachinePath> paths = {};
  std::vector<StateMachineEvent> events = {};
  std::map<std::string, std::string>* values;
  StateMachine(std::string startStateStr, std::vector<std::string> otherStates) {
    startState = StateMachineState(startStateStr);
    states.push_back(startState);

    for (int i = 0; i < otherStates.size(); i++) {
      std::string statename = otherStates[i];
      if (statename == startState.name) {
        continue;
      }
      StateMachineState interimState = StateMachineState(statename);
      states.push_back(interimState);
    }
    currentState = startState;
  }
  void setValues(std::map<std::string, std::string>* m) {
    values = m;
  }

  void addPath(std::string startStateStr, std::string endStateStr, StateMachineEvent e) {
    StateMachineState ss;
    StateMachineState es;
    for (int i = 0; i < states.size(); i++) {
      StateMachineState stt = states[i];
      if (stt.name == startStateStr) {
        ss = stt;
      }
      if (stt.name == endStateStr) {
        es = stt;
      }
    }
    StateMachinePath path = StateMachinePath(ss, es, e);
    paths.push_back(path);
  }
  void addPath(std::string startStateStr, std::string endStateStr, StateMachineEvent e, StateMachineFunction* fx) {
    StateMachineState ss;
    StateMachineState es;
    for (int i = 0; i < states.size(); i++) {
      StateMachineState stt = states[i];
      if (stt.name == startStateStr) {
        ss = stt;
      }
      if (stt.name == endStateStr) {
        es = stt;
      }
    }
    StateMachinePath path = StateMachinePath(ss, es, e, fx);
    paths.push_back(path);
  }
  void migrate(StateMachineEvent ev) {
    for (int i = 0; i < paths.size(); i++) {
      StateMachinePath p = paths[i];
      if (p.startState.name == currentState.name && p.event.name == ev.name) {
        if (p.fx != NULL) {
          p.fx->doFx(values);
        }
        currentState = p.endState;
        break;
      }
    }
  }
  StateMachineState getCurrentState() {
    return currentState;
  }
  void reset() {
    currentState = startState;
  }
};

class GoBackSmFunction : public StateMachineFunction {
private:
  BackHandler* b;
public:
  GoBackSmFunction(BackHandler* bh) {
    b = bh;
  }
  void doFx(std::map<std::string, std::string>* m) {
    b->handleBack();
  }
};
class SetTimerQuerySmFunction : public StateMachineFunction {
private:
  BackHandler* b;
  std::string q;
  std::string v;
public:
  SetTimerQuerySmFunction(BackHandler* bh, std::string query) {
    b = bh;
    q = query;
  }
  void doFx(std::map<std::string, std::string>* m) {
    if (q == "set_timer") {
      std::string minutestr = m->at("minute");
      std::string secondstr = m->at("second");
      int minutes = stoi(minutestr);
      int seconds = stoi(secondstr);
      seconds = seconds + minutes * 60;
      unsigned long epoch = getTimeEpoch() + seconds;
      v = a_string(epoch);
      (*m)["timer_end_time"] = a_string(epoch);
      b->handleBackWithQuery(q, v);
    } else {
      b->handleBackWithQuery(q, v);
    }
  }
};
class SetQueryAndGoBackSmFunction : public StateMachineFunction {
private:
  BackHandler* b;
  std::string q;
  std::string v;
public:
  SetQueryAndGoBackSmFunction(BackHandler* bh, std::string query) {
    b = bh;
    q = query;
  }
  void doFx(std::map<std::string, std::string>* m) {
    std::string minutestr = m->at("minute");
    std::string secondstr = m->at("second");
    int minutes = stoi(minutestr);
    int seconds = stoi(secondstr);
    seconds = seconds + minutes * 60;
    unsigned long epoch = getTimeEpoch() + seconds;
    v = a_string(epoch);
    (*m)["timer_end_time"] = a_string(getTimeEpoch());
    b->handleBackWithQuery(q, v);
  }
};

class EmptySmFunction : public StateMachineFunction {
public:
  EmptySmFunction(){};
  void doFx(std::map<std::string, std::string>* m) {}
};
class InitStopWatchSmFunction : public StateMachineFunction {
public:
  InitStopWatchSmFunction(){};
  void doFx(std::map<std::string, std::string>* m) {

    (*m)["minute"] = "00";
    (*m)["second"] = "00";
  }
};
class IncrementMinuteSmFunction : public StateMachineFunction {
public:
  IncrementMinuteSmFunction(){};
  void doFx(std::map<std::string, std::string>* m) {
    std::string minuteStr = m->at("minute");
    int minuteVal = stoi(minuteStr);
    minuteVal++;
    if (minuteVal > 59) {
      return;
    }

    (*m)["minute"] = formatDoubleDigits(minuteVal);
  }
};
class IncrementSecondSmFunction : public StateMachineFunction {
public:
  IncrementSecondSmFunction(){};
  void doFx(std::map<std::string, std::string>* m) {
    std::string secondStr = m->at("second");
    int secondVal = stoi(secondStr);
    secondVal++;
    if (secondVal > 59) {
      return;
    }

    (*m)["second"] = formatDoubleDigits(secondVal);
  }
};

class DecrementMinuteSmFunction : public StateMachineFunction {
public:
  DecrementMinuteSmFunction(){};
  void doFx(std::map<std::string, std::string>* m) {

    std::string minuteStr = m->at("minute");
    int minuteVal = stoi(minuteStr);
    minuteVal--;
    if (minuteVal < 0) {
      return;
    }
    (*m)["minute"] = formatDoubleDigits(minuteVal);
  }
};
class DecrementSecondSmFunction : public StateMachineFunction {
public:
  DecrementSecondSmFunction(){};

  void doFx(std::map<std::string, std::string>* m) {

    std::string secondStr = m->at("second");
    int secondVal = stoi(secondStr);
    secondVal--;
    if (secondVal < 0) {
      return;
    }
    (*m)["second"] = formatDoubleDigits(secondVal);
  }
};


class SwInitSmFunction : public StateMachineFunction {
public:
  SwInitSmFunction(){};

  void doFx(std::map<std::string, std::string>* m) {
    (*m)["sw_set"] = "false";
    (*m)["sw_elapsed"] = "0";
    (*m)["sw_started_at"] = "0";
  }
};

class SwStartSmFunction : public StateMachineFunction {
public:
  SwStartSmFunction(){};

  void doFx(std::map<std::string, std::string>* m) {
    (*m)["sw_set"] = "true";
    (*m)["sw_elapsed"] = "0";
    (*m)["sw_started_at"] = a_string(getTimeEpoch());
  }
};

class SwPauseSmFunction : public StateMachineFunction {
public:
  SwPauseSmFunction(){};

  void doFx(std::map<std::string, std::string>* m) {

    std::string elapsedStr = m->at("sw_elapsed");
    std::string startedAtStr = m->at("sw_started_at");

    int elapsed = stoi(elapsedStr);
    unsigned long startedAt = stoul(startedAtStr);
    unsigned long now = getTimeEpoch();
    elapsed = elapsed + int(now - startedAt);
    (*m)["sw_elapsed"] = a_string(elapsed);
    (*m)["sw_set"] = "false";
  }
};

class SwResumeSmFunction : public StateMachineFunction {
public:
  SwResumeSmFunction(){};

  void doFx(std::map<std::string, std::string>* m) {

    (*m)["sw_set"] = "true";
    (*m)["sw_started_at"] = a_string(getTimeEpoch());
  }
};
