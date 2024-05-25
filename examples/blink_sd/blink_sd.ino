#include <esplua.h>

//#define FILE_SYSTEM_SD
//#define FILE_SYSTEM_FFAT
#define FILE_SYSTEM_SPIFFS
#ifdef FILE_SYSTEM_SD
#include <SD.h>
#include "SPI.h"
#define SPI_CS 38
#elif defined(FILE_SYSTEM_FFAT)
#include <FFat.h>
#elif defined(FILE_SYSTEM_SPIFFS)
#include "SPIFFS.h"
#endif



void luaCallbackError(const char *errMsg)
{
  Serial.printf("Lua error: %s\n", errMsg);
}

void printEsp(std::string msg){
  Serial.printf("Calling printEsp %s\n", msg.c_str());
}

EspLua *lua;

const char *luaFile = "local PIN = 1\nfunction setup()\n  pinMode(PIN, OUTPUT)\n  printEsp('Hello from lua!')\nend\n\nfunction loop()\n  digitalWrite(PIN, HIGH)\n  delay(1000)\n  digitalWrite(PIN, LOW)\n  delay(1000)\n  printEsp('Elapsed '..millis()..' ms')\nend";

void setup() {
  Serial.begin(115200);
  Serial.println("Starting lua with psram!");
  lua = new EspLua(false);
  if (lua == nullptr){
    Serial.println("Failed to start lua. Maybe out of memmory?");
    for(;;){}
  }

  lua->SetErrorCallback(luaCallbackError);

  lua->FuncRegister("printEsp", printEsp);

  lua->FuncRegister("millis", millis);
  lua->FuncRegister("delay", delay);
  lua->FuncRegister("digitalWrite", digitalWrite);
  lua->FuncRegister("digitalRead", digitalRead);
  lua->FuncRegister("analogRead", analogRead);
  lua->FuncRegister("pinMode", pinMode);

  lua->setConstant("OUTPUT", OUTPUT);
  lua->setConstant("HIGH", HIGH);
  lua->setConstant("LOW", LOW);

  #ifdef FILE_SYSTEM_SD
  if(!SD.begin(SPI_CS, SPI)){
    Serial.println("Failed to start SD card");
    for(;;){}
  } 

  File file = SD.open("/init.lua", FILE_WRITE);
  lua->setFS_SD();
  #elif defined(FILE_SYSTEM_FFAT)

  if (!FFat.begin(true)) {
    Serial.println("Failed to start FFAT");
    for(;;){}
  }

  File file = FFat.open("/init.lua", FILE_WRITE);
  lua->setFS_FFat();
  #elif defined(FILE_SYSTEM_SPIFFS)


  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to spiffs");
    for(;;){}
  }

  File file = SPIFFS.open("/init.lua", FILE_WRITE);
  lua->setFS_SPIFFS();

  #endif

  file.println(luaFile);
  file.close();


  if (!lua->loadFile("/init.lua")){
    Serial.println("There is no init.lua in the file system :/");
    for(;;){}
  }


  std::string res = lua->callLuaFunction("setup");
  if(res != ""){
    Serial.printf("Failed to run setup: %s\n", res.c_str());
    for(;;){}
  }


}


void loop() {
  std::string res = lua->callLuaFunction("loop");
  if(res != ""){
    Serial.printf("Failed to run loop: %s\n", res.c_str());
    for(;;){}
  }

}
