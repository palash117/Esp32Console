#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <string>
#include <consoleversion.h>
#include <regex>
#include "pitches.h"
#include <statemachine.h>
using namespace std;
#define BUZZZER_PIN 18  // ESP32 pin GPIO18 connected to piezo buzzer

void tone1() {
  const int buzzerPin = 18;
  int melody[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
  };
  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(buzzerPin);
  }
}
std::vector<std::string> split(const std::string str, const std::string regex_str) {
  std::regex regexz(regex_str);
  return { std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
           std::sregex_token_iterator() };
}
std::string concatStrings(std::vector<std::string> v) {
  std::string str = "";
  for (std::string& s : v) {
    str = str + s;
  }
  return str;
}


class DisplayWord {
public:
  int marginLeft = 0;
  int marginRight = 0;
  std::string str = "";
  std::string seperator = "";
  std::string prefix = "";
  std::string suffix = "";
  bool selected = false;
  DisplayWord(){};
  DisplayWord(std::string strval) {
    str = strval;
  }
  void setMargins(int ml, int mr) {
    marginLeft = ml;
    marginRight = mr;
  }
  void setPrefix(std::string val) {
    prefix = val;
  }
  void setSuffix(std::string val) {
    suffix = val;
  }
};
class DisplayLine {
public:
  std::vector<DisplayWord> words = {};
  std::string seperator = "";
  bool isTopBar = false;
  bool selected = false;
  bool bold = false;
  bool hasFlag = false;
  bool flagValue = false;
  DisplayLine(){};
  DisplayLine(std::string str) {
    DisplayWord wrd = DisplayWord(str);
    addDisplayWord(wrd);
  };
  DisplayLine(std::string str, bool isBold, bool isSelected) {
    DisplayWord wrd = DisplayWord(str);
    addDisplayWord(wrd);
    bold = isBold;
    selected = isSelected;
  };
  DisplayLine(DisplayWord word) {
    addDisplayWord(word);
  };
  void setSeperator(std::string sptr) {
    seperator = sptr;
  }
  void addDisplayWord(DisplayWord obj) {
    words.push_back(obj);
  }
};
class DisplayPage {
public:
  DisplayLine topbar;
  std::vector<DisplayLine> lines = {};
  bool isDarkMode = true;
  bool isInverted = true;
  DisplayPage(){};
  DisplayPage(std::string str) {
    DisplayWord word = DisplayWord(str);
    DisplayLine line = DisplayLine(word);
    addDisplayLine(line);
  };
  void addTopBar(DisplayLine ln) {
    topbar = ln;
  }
  void addDisplayLine(DisplayLine line) {
    lines.push_back(line);
  };
};

DisplayLine getDateLine() {
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, sizeof(buffer), "%d-%m-%Y", timeinfo);

  std::string str(buffer);
  DisplayLine line = DisplayLine(str);
  return line;
}
DisplayLine getTimeLine(bool is24Hformat) {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);
  if (is24Hformat) {
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  } else {
    std::strftime(buffer, sizeof(buffer), "%I:%M:%S%p", timeinfo);
  }
  std::string str(buffer);
  DisplayLine line = DisplayLine(str);
  return line;
  
}

class Screen {
public:
  virtual DisplayPage display() = 0;
  virtual void tick() = 0;
  virtual void handleLeft() = 0;
  virtual void handleRight() = 0;
  virtual void handleOk() = 0;
  virtual void setBackHandlerRef(BackHandler*) = 0;
  virtual std::string screenNme() = 0;
  virtual void updateValue(std::string query, std::string value);
};

class TimerScreen : public Screen {
private:
  BackHandler* backHandler;
  std::string name = "Timer";
  bool isTimerSet = false;
  bool editMode = false;
  int minutes = 0;
  int seconds = 0;
  std::map<std::string, std::string> values = { { "minute", "00" }, { "second", "00" } };
  StateMachine* sm;

public:
  void updateValue(std::string query, std::string value){};
  TimerScreen(BackHandler* b) {
    backHandler = b;
    std::vector<std::string> otherStates = { "minute_edit", "second_selected", "second_edit", "ok_selected", "cancel_selected", "timer_running_cancel_selected", "timer_running_back_selected" };
    sm = new StateMachine("minute_selected", otherStates);
    EmptySmFunction* emptyFunction = new EmptySmFunction();
    InitStopWatchSmFunction* initFunction = new InitStopWatchSmFunction();


    sm->addPath("minute_selected", "second_selected", r_event, emptyFunction);
    sm->addPath("minute_selected", "minute_selected", l_event, emptyFunction);
    sm->addPath("minute_selected", "minute_edit", ok_event, emptyFunction);
    sm->addPath("minute_edit", "minute_edit", l_event, new IncrementMinuteSmFunction());
    sm->addPath("minute_edit", "minute_edit", r_event, new DecrementMinuteSmFunction());
    sm->addPath("minute_edit", "minute_selected", ok_event, emptyFunction);
    sm->addPath("second_selected", "ok_selected", r_event, emptyFunction);
    sm->addPath("second_selected", "minute_selected", l_event, emptyFunction);
    sm->addPath("second_selected", "second_edit", ok_event, emptyFunction);
    sm->addPath("second_edit", "second_edit", l_event, new IncrementSecondSmFunction());
    sm->addPath("second_edit", "second_edit", r_event, new DecrementSecondSmFunction());
    sm->addPath("second_edit", "second_selected", ok_event, emptyFunction);
    sm->addPath("ok_selected", "cancel_selected", r_event, emptyFunction);
    sm->addPath("ok_selected", "second_selected", l_event, emptyFunction);
    sm->addPath("ok_selected", "timer_running_cancel_selected", ok_event, new SetTimerQuerySmFunction(backHandler, "set_timer"));
    sm->addPath("cancel_selected", "cancel_selected", r_event, emptyFunction);
    sm->addPath("cancel_selected", "ok_selected", l_event, emptyFunction);
    sm->addPath("cancel_selected", "minute_selected", ok_event, new GoBackSmFunction(backHandler));

    sm->addPath("timer_running_cancel_selected", "timer_running_cancel_selected", r_event, emptyFunction);
    sm->addPath("timer_running_cancel_selected", "timer_running_back_selected", l_event, emptyFunction);
    sm->addPath("timer_running_cancel_selected", "minute_selected", ok_event, new SetTimerQuerySmFunction(backHandler, "cancel_timer"));
    sm->addPath("timer_running_cancel_selected", "minute_selected", rst_event, initFunction);


    sm->addPath("timer_running_back_selected", "timer_running_back_selected", l_event, emptyFunction);
    sm->addPath("timer_running_back_selected", "timer_running_cancel_selected", r_event, emptyFunction);
    sm->addPath("timer_running_back_selected", "timer_running_back_selected", ok_event, new GoBackSmFunction(backHandler));
    sm->addPath("timer_running_back_selected", "minute_selected", rst_event, initFunction);

    sm->setValues(&values);
  }

  void tick() {

    if (sm->getCurrentState().name == "timer_running_cancel_selected" || sm->getCurrentState().name == "timer_running_back_selected") {
      std::string timerEndtimeStr = values.at("timer_end_time");
      int timerEndTime = stoul(timerEndtimeStr);
      //unsigned long epoch = getTimeEpoch();
      unsigned long epoch = getUntetheredTimeEpoch();
      if (timerEndTime <= epoch) {
        sm->migrate(rst_event);
      }
      values["minute"] = formatDoubleDigits(int((timerEndTime - epoch) / 60));
      values["second"] = formatDoubleDigits(int((timerEndTime - epoch) % 60));
    };
  }
  DisplayPage display() {
    std::string currentState = sm->getCurrentState().name;
    DisplayPage page = DisplayPage();
    DisplayWord minuteWord = DisplayWord(values["minute"]);
    DisplayWord secondWord = DisplayWord(values["second"]);
    DisplayWord optionWord1 = DisplayWord("ok");
    DisplayWord optionWord2 = DisplayWord("cancel");
    bool isTimeBold = false;
    bool isOption1Bold = false;
    bool isOption2Bold = false;
    if (currentState == "minute_selected") {
      minuteWord.str = concatStrings({ "[", values["minute"], "]" });
      isTimeBold = true;
    } else if (currentState == "minute_edit") {
      minuteWord.str = concatStrings({ ">", values["minute"], "<" });
      isTimeBold = true;
    } else if (currentState == "second_selected") {
      secondWord.str = concatStrings({ "[", values["second"], "]" });
      isTimeBold = true;
    } else if (currentState == "second_edit") {
      secondWord.str = concatStrings({ ">", values["second"], "<" });
      isTimeBold = true;
    } else if (currentState == "ok_selected") {
      optionWord1.str = concatStrings({ "[", "ok", "]" });
      isOption1Bold = true;
    } else if (currentState == "cancel_selected") {
      optionWord2.str = concatStrings({ "[", "cancel", "]" });
      isOption2Bold = true;
    } else if (currentState == "timer_running_cancel_selected") {
      optionWord1.str = concatStrings({ "[", "cancel", "]" });
      optionWord2.str = "back";
      isTimeBold = true;
    } else if (currentState == "timer_running_back_selected") {
      optionWord1.str = "cancel";
      optionWord2.str = concatStrings({ "[", "back", "]" });
      isTimeBold = true;
    }


    DisplayLine lin1 = DisplayLine();
    lin1.addDisplayWord(minuteWord);
    lin1.addDisplayWord(secondWord);
    lin1.setSeperator(":");
    lin1.bold = isTimeBold;


    DisplayLine lin2 = DisplayLine();
    lin2.addDisplayWord(optionWord1);
    lin2.bold = isOption1Bold;

    DisplayLine lin3 = DisplayLine();
    lin3.addDisplayWord(optionWord2);
    lin3.bold = isOption2Bold;

    //  cout << minuteWord.str << " and " << secondWord.str << "\n";

    page.addDisplayLine(lin1);
    page.addDisplayLine(lin2);
    page.addDisplayLine(lin3);

    return page;
  }
  void handleLeft() {
    sm->migrate(l_event);
    return;
  };
  void handleRight() {
    sm->migrate(r_event);
    return;
  };
  void handleOk() {
    sm->migrate(ok_event);
    return;
  };
  void setBackHandlerRef(BackHandler* ref) {
    backHandler = ref;
  };
  std::string screenNme() {
    return name;
  }
};
class HomeScreen : public Screen {
private:
  BackHandler* backHandler;
  std::string name = "Home";
  bool is24HFormat = false;

public:
  void tick(){};
  void updateValue(std::string query, std::string value) {
    if (query == "24h fmt") {
      is24HFormat = (value == "true");
    }
  };
  DisplayPage display() {
    DisplayPage page = DisplayPage();
    DisplayLine tmLine = getTimeLine(is24HFormat);
    tmLine.bold = true;
    page.addDisplayLine(tmLine);
    page.addDisplayLine(getDateLine());
    return page;
  }
  void handleLeft() {
    return;
  };
  void handleRight() {
    return;
  };
  void handleOk() {
    backHandler->handleMenu();
  };
  void setBackHandlerRef(BackHandler* ref) {
    backHandler = ref;
  };
  std::string screenNme() {
    return name;
  }
};
class SettingsScreen : public Screen {
private:
  BackHandler* backHandler;
  std::string name = "Settings";
  std::vector<std::string> settingParams = { "DrkMd", "24h fmt", "Rotate","back" };
  std::map<std::string, std::string> values = {
    { "DrkMd", "true" },
    { "24h fmt", "false" },
    {"Rotate","true"},
  };
  int selectedIndex = 0;

public:

  SettingsScreen(BackHandler* b) {
    backHandler = b;
  }
  void tick(){};
  void handleLeft() {
    if (selectedIndex <= 0) {
      selectedIndex = 0;
    } else {
      selectedIndex = selectedIndex - 1;
    }
    return;
  };
  void updateValue(std::string query, std::string value){};
  void handleRight() {
    if (selectedIndex >= (settingParams.size() - 1)) {
      selectedIndex = settingParams.size() - 1;
    } else {
      selectedIndex = selectedIndex + 1;
    }
    return;
  };
  void handleOk() {
    if (selectedIndex == (settingParams.size() - 1)) {
      backHandler->handleBack();
      return;
    }
    std::string value = values.at(settingParams[selectedIndex]);
    if (value == "true") {
      value = "false";
    } else {
      value = "true";
    }
    backHandler->handleQuery(settingParams[selectedIndex], value);
    values[settingParams[selectedIndex]] = value;
    return;
  };
  void setBackHandlerRef(BackHandler* ref) {
    backHandler = ref;
  };
  std::string screenNme() {
    return name;
  }

  DisplayPage display() {
    int pageLimit = 4;
    DisplayPage page = DisplayPage();
    for (int i = 0; i < pageLimit; i++) {
      DisplayLine line = DisplayLine();
      int lineno = selectedIndex - 1 + i;
      if (lineno <= settingParams.size() - 1 && lineno >= 0) {
        DisplayWord word = DisplayWord(settingParams[lineno]);
        line.addDisplayWord(word);
        if (lineno < (settingParams.size() - 1)) {
          line.hasFlag = true;
          line.flagValue = (values[settingParams[lineno]] == "true");
        }
        if (lineno == selectedIndex) {
          // line.selected = true;
          line.bold = true;
        }
      }

      page.addDisplayLine(line);
    }
    return page;
  }
};
class AboutScreen : public Screen {
private:
  BackHandler* backHandler;
  std::string name = "About";

public:

  void updateValue(std::string query, std::string value){};
  void tick(){};
  DisplayPage display() {
    DisplayPage page = DisplayPage();

    DisplayLine l1 = DisplayLine(DisplayWord("Version:"));
    l1.addDisplayWord(DisplayWord(version));
    page.addDisplayLine(l1);
    return page;
  }
  void handleLeft() {
    return;
  };
  void handleRight() {
    return;
  };
  void handleOk() {
    backHandler->handleMenu();
  };
  void setBackHandlerRef(BackHandler* ref) {
    backHandler = ref;
  };
  std::string screenNme() {
    return name;
  }
};


class StopwatchScreen : public Screen {
private:
  BackHandler* backHandler;
  std::string name = "StpWtch";
  bool isTimerSet = false;
  bool editMode = false;
  int minutes = 0;
  int seconds = 0;
  std::map<std::string, std::string> values = {};
  StateMachine* sm;

public:

  StopwatchScreen(BackHandler* b) {
    backHandler = b;
    std::vector<std::string> otherStates = { "watch_back_selected", "watch_running_pause_selected", "watch_paused_resume_selected", "watch_paused_reset_selected", "watch_paused_back_selected", "watch_running_reset_selected", "watch_running_back_selected" };
    sm = new StateMachine("watch_start_selected", otherStates);
    EmptySmFunction* emptyFunction = new EmptySmFunction();

    StateMachineFunction* goBack = new GoBackSmFunction(backHandler);
    SwInitSmFunction* initFx = new SwInitSmFunction();
    SwPauseSmFunction* pauseFx = new SwPauseSmFunction();
    SwResumeSmFunction* resumeFx = new SwResumeSmFunction();
    SwStartSmFunction* startFx = new SwStartSmFunction();
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
  void updateValue(std::string query, std::string value){};

  void tick(){};

  DisplayLine getSwTimeLine() {
    DisplayLine ln = DisplayLine();
    ln.bold = true;
    std::string elapsedStr = values.at("sw_elapsed");
    std::string startedAtStr = values.at("sw_started_at");
    std::string swSetStr = values.at("sw_set");

    int elapsed = stoi(elapsedStr);
    unsigned long startedAt = stoul(startedAtStr);
    //unsigned long now = getTimeEpoch();
    unsigned long now = getUntetheredTimeEpoch();
    if (swSetStr == "true") {
      elapsed = elapsed + int(now - startedAt);
    }
    ln.addDisplayWord(DisplayWord(formatTime(int(elapsed / 3600), int((elapsed / 60) % 60), int(elapsed % 60))));
    ln.seperator = ":";
    ln.bold = true;
    return ln;
  }
  DisplayPage display() {
    std::string currentState = sm->getCurrentState().name;
    DisplayPage page = DisplayPage();

    if (currentState == "watch_start_selected") {
      DisplayLine ln = DisplayLine("start");
      ln.selected = true;
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    } else if (currentState == "watch_back_selected") {

      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(DisplayLine(DisplayWord("start")));
      page.addDisplayLine(ln);
    } else if (currentState == "watch_running_pause_selected") {

      DisplayLine ln = DisplayLine("pause");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    } else if (currentState == "watch_paused_resume_selected") {

      DisplayLine ln = DisplayLine("resume");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    } else if (currentState == "watch_paused_reset_selected") {


      DisplayLine ln = DisplayLine("reset");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("resume")));
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    } else if (currentState == "watch_paused_back_selected") {
      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("resume")));
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(ln);
    } else if (currentState == "watch_running_reset_selected") {
      DisplayLine ln = DisplayLine("reset");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("pause")));
      page.addDisplayLine(ln);
      page.addDisplayLine(DisplayLine(DisplayWord("back")));
    } else if (currentState == "watch_running_back_selected") {
      DisplayLine ln = DisplayLine("back");
      ln.selected = true;
      page.addDisplayLine(getSwTimeLine());
      page.addDisplayLine(DisplayLine(DisplayWord("pause")));
      page.addDisplayLine(DisplayLine(DisplayWord("reset")));
      page.addDisplayLine(ln);
    }


    return page;
  }
  void handleLeft() {
    sm->migrate(l_event);
    return;
  };
  void handleRight() {
    sm->migrate(r_event);
    return;
  };
  void handleOk() {
    sm->migrate(ok_event);
    return;
  };
  void setBackHandlerRef(BackHandler* ref) {
    backHandler = ref;
  };
  std::string screenNme() {
    return name;
  }
};

class Console : public BackHandler {
private:
  Screen* selectedScreen = nullptr;
  std::vector<Screen*> screens;
  bool isMenuSelected = false;
  bool isNotification = false;
  bool hasTone = false;
  int menuIndex = 0;
  std::map<std::string, std::string> consoleValues;
  bool displayOff = false;
  bool isDarkMode = true;
  bool is24HFormat = false;
  bool isInverted = true;


public:
  void isWifiConnected(bool val) {
    if (val) {
      consoleValues["wifi.connected"] = true;
    } else {
      consoleValues["wifi.connected"] = false;
    }
  }
  void handleQuery(std::string query, std::string value) {
    if (query == "set_timer") {
      consoleValues["timer"] = value;
    } else if (query == "set_notification") {
      std::vector<std::string> values = split(value, "&");
      isNotification = true;
      consoleValues["notification"] = "true";
      consoleValues["notification_header"] = values[0];
      consoleValues["notification_message"] = values[1];
      consoleValues["notification_tone"] = values[2];
      hasTone = true;
      consoleValues["notification_page"] = "0";
    } else if (query == "cancel_timer") {
      consoleValues["timer"] = "";
      std::vector<std::string> settingParams = { "DrkMd", "24h fmt" };

    } else if (query == "DrkMd") {
      if (value == "true") {
        isDarkMode = true;
      } else {
        isDarkMode = false;
      }
    } else if (query == "24h fmt") {
      if (value == "true") {
        is24HFormat = true;
      } else {
        is24HFormat = false;
      }
    }else if (query=="Rotate"){
      cout<<"rotate value is ";
      cout<<value;
      cout<<"\n";
      isInverted = value=="true";
      cout<<isInverted<<"\n";
    }
    for (int i=0;i<screens.size();i++){
      screens[i]->updateValue(query,value);
    }
  }

  void handleBackWithQuery(std::string query, std::string value) {
    handleQuery(query, value);
    handleMenu();
    return;
  }

  void handleBack() {
    handleMenu();
    return;
  }
  void handleMenu() {
    isMenuSelected = true;
    return;
  }

  void addScreen(Screen* scr) {
    scr->setBackHandlerRef(dynamic_cast<BackHandler*>(this));
    screens.push_back(scr);
    selectedScreen = screens[0];
  }
  /*   menu functions */

  void playTone() {
    if (hasTone) {
      hasTone = false;
      tone1();
    }
  }
  DisplayPage getPage() {

    DisplayPage page = DisplayPage();
    page.isDarkMode = isDarkMode;
    page.isInverted = isInverted;
    return page;
  }
  DisplayPage notificationDisplay() {
    int pageLimit = 4;
    int lineLimit = 21;
    DisplayPage page = getPage();
    std::string message = consoleValues["notification_message"];
    page.addDisplayLine(DisplayLine(message));
    playTone();
    return page;
  }
  DisplayPage menuDisplay() {
    //todo

    int pageLimit = 4;
    DisplayPage page = getPage();
    for (int i = 0; i < pageLimit; i++) {
      DisplayLine line = DisplayLine();
      int lineno = menuIndex - 1 + i;
      if (lineno <= screens.size() - 1 && lineno >= 0) {
        DisplayWord word = DisplayWord(screens[lineno]->screenNme());
        line.addDisplayWord(word);
        if (lineno == menuIndex) {
          line.selected = true;
          line.bold = true;
        }
      }

      page.addDisplayLine(line);
    }
    return page;
  }
  void menuHandleLeft() {
    if (menuIndex <= 0) {
      menuIndex = 0;
    } else {
      menuIndex = menuIndex - 1;
    }
    selectedScreen = screens[menuIndex];
  }
  void notificationHandleLeft(){};
  void notificationHandleRight(){};

  void menuHandleRight() {
    if (menuIndex >= (screens.size() - 1)) {
      menuIndex = screens.size() - 1;
    } else {
      menuIndex = menuIndex + 1;
    }
    selectedScreen = screens[menuIndex];
  }

  void notificationHandleOk() {
    isNotification = false;
    consoleValues["notification"] = "false";
  };
  void menuHandleOk() {
    isMenuSelected = false;
  };

  void checkTimer() {
    //   cout<<"check timer"<<"\n";
    // cout<<"timer"<<"\n";
    if (consoleValues.count("timer") && consoleValues.at("timer") != "") {
      unsigned long timerEpoch = stoul(consoleValues.at("timer"));
      Serial.println("checking for timer");
      cout << "checking for timer"
           << "\n";
      Serial.println(timerEpoch);
      cout << timerEpoch << "\n";
      Serial.println("current timer is ");
      cout << "current timer is "
           << "\n";
      Serial.println(getTimeEpoch());
      cout << getTimeEpoch() << "\n";
      //if (timerEpoch <= getTimeEpoch()) {
      if (timerEpoch <= getUntetheredTimeEpoch()) {
        Serial.println("timer completed");
        isNotification = true;
        consoleValues["timer"] = "";
        handleQuery("set_notification", "Timer Over&Complete&tone1");
      }
    }
  }
  void tick() {
    this->checkTimer();
    if (isMenuSelected) {
      return;
    }
    for (int i = 0; i < screens.size(); i++) {
      screens[i]->tick();
    }
  };


  DisplayPage display() {
    if (displayOff) {
      return DisplayPage();
    }
    if (isNotification) {
      return notificationDisplay();
    }
    if (isMenuSelected) {
      return addTopBar(menuDisplay());
    }
    return addTopBar(selectedScreen->display());
  }
  DisplayPage addTopBar(DisplayPage pg) {
    DisplayLine tp = DisplayLine();
    std::string header = selectedScreen->screenNme();
    if (isMenuSelected) {
      header = "Menu";
    }
    tp.bold = true;
    DisplayWord cnsl = DisplayWord(header);
    cnsl.setMargins(0, 0);


    tp.addDisplayWord(cnsl);

    if (consoleValues.count("timer") && consoleValues.at("timer") != "") {
      DisplayWord tmr = DisplayWord("T");
      tp.addDisplayWord(tmr);
    }
    tp.isTopBar = true;
    pg.addTopBar(tp);
    pg.isDarkMode = isDarkMode;
    pg.isInverted = isInverted;

    return pg;
  }



  void handleLeft() {
    if (isNotification) {
      return notificationHandleLeft();
    }
    if (isMenuSelected) {
      return menuHandleLeft();
    }
    selectedScreen->handleLeft();
  }

  void handleRight() {
    if (isNotification) {
      return notificationHandleRight();
    }
    if (isMenuSelected) {
      return menuHandleRight();
    }
    selectedScreen->handleRight();
  }

  void handleOk() {
    if (isNotification) {
      return notificationHandleOk();
    }
    if (displayOff) {
      displayOff = false;
      return;
    }
    if (isMenuSelected) {
      return menuHandleOk();
    }
    selectedScreen->handleOk();
  };

  void handleOkLong() {
    displayOff = !displayOff;
  }
};


Console* setupConsole() {
  setupTime();
  Console* console = new Console();
  HomeScreen* homeScreen = new HomeScreen();
  TimerScreen* timerScreen = new TimerScreen(console);
  AboutScreen* aboutScreen = new AboutScreen();
  StopwatchScreen* swScreen = new StopwatchScreen(console);
  SettingsScreen* stScreen = new SettingsScreen(console);
  console->addScreen(homeScreen);
  console->addScreen(timerScreen);
  console->addScreen(swScreen);
  console->addScreen(stScreen);
  console->addScreen(aboutScreen);
  return console;
}
