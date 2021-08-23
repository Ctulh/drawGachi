#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <functional>
#include <memory>


std::vector<int> colors{31,32,33,34,35,36};
int currentColor = colors[0];
std::vector<std::string> text;


class Mode{
 public:
  virtual void draw(std::vector<std::string>& text) const = 0;
  virtual ~Mode() {};
};

class EveryCharacter: public Mode{
  void draw (std::vector<std::string>& text) const override{
    for(int i=0;i<text.size();i++){
      for(int j=0;j<text[i].size();j++){
        if(j>=text[i].size()-(i+1)*(std::max(text[i].size(),text.size())/std::min(text[i].size(),text.size())))
        {std::cout<<"\033["<<colors[currentColor%colors.size()];
          currentColor++;
        }
        else
          std::cout << "\033[" << 37;
        std::cout<< "m"<<text[i][j]<<"\033[0m";

      }
    }
  }
};

class EveryLine: public Mode{
  void draw (std::vector<std::string>& text) const override{
    for(int i=0;i<text.size();i++){
        std::cout << "\033[" << colors[currentColor%colors.size()];
        currentColor++;
        std::cout<< "m"<<text[i]<<"\033[0m";
    }
  }
};

class Monochrome: public Mode{
  void draw (std::vector<std::string>& text) const override{
    for(int i=0;i<text.size() ;i++){
      std::cout<<text[i];
    }
  }
};

class Switchable{
 public:
  int current_mode = 0;
  virtual void add_mode(std::shared_ptr<Mode>&& mode)= 0;
  virtual void switch_mode() = 0;
  virtual ~Switchable() {};
  std::vector<std::shared_ptr<Mode>> modes;
};

class MainLoop:public Switchable{
 public:
  MainLoop(){}

  void add_mode(std::shared_ptr<Mode>&& mode) override{
    modes.emplace_back(std::move(mode));
  }

  void switch_mode() override{
    current_mode++;
    this->mode_ptr = modes[current_mode%modes.size()];
  }

  void pause_resume_loop(){
    activeFlag = !activeFlag;
  }

  [[noreturn]] void run(std::vector<std::string>& text){
    if (!mode_ptr)
      if(modes.size())
        mode_ptr=modes[0];
    while(true){
      if(activeFlag){
        system("clear");
        mode_ptr->draw(text);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds (500));
    }
  }
 private:
  std::shared_ptr<Mode> mode_ptr;
  bool activeFlag = true;
};

class KeyCallback{
  char keyChar;
  std::function<void(void)> action;
 public:
  KeyCallback(char keyChar,std::function<void(void)>&& action):
  keyChar(keyChar),
  action(std::move(action))
  {}

  char getChar(){return keyChar;}
  void runCallback(){action();}

};

class InputLoop{
  std::vector<KeyCallback> keys;
 public:
  [[noreturn]] void run(){
    char c;
    while(true){
     if(c=getchar()){
        for(auto& key: keys)
        {
          if(key.getChar()== c)
            key.runCallback();
        }
     }
    }
  }
  void AddKeyCallback(char keyChar,std::function<void(void)>&& action){
      keys.push_back({keyChar,std::move(action)});
  }
};

using namespace std;


int main() {



  FILE * filePointer;
  int bufferLength = 255;
  char buffer[bufferLength];
  int index = 0;
  filePointer = fopen("../file", "r");
  while(fgets(buffer, bufferLength, filePointer)) {
    text.push_back(buffer);
    index++;
  }
  fclose(filePointer);

  
  InputLoop input_loop;
  MainLoop main_loop;
  
  main_loop.add_mode(std::make_shared<Monochrome>(Monochrome()));
  main_loop.add_mode(std::make_shared<EveryLine>(EveryLine()));
  main_loop.add_mode(std::make_shared<EveryCharacter>(EveryCharacter()));

  input_loop.AddKeyCallback('s',[&main_loop](){main_loop.pause_resume_loop();});
  input_loop.AddKeyCallback('m',[&main_loop](){main_loop.switch_mode();});

  std::thread t(&InputLoop::run,input_loop);
  main_loop.run(text);
  t.join();

  return 0;
}
