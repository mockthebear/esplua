#include "esplua.h"
#include "esp32-hal.h"
#include <SD.h>
#include <FFat.h>
#include <SPIFFS.h>

//Make sure that any lua scripts use the psram instead of the sram
static void *psram_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud;  (void)osize; 
  if (nsize == 0) { 
    heap_caps_free(ptr);
    return NULL;
  }else{
    return ps_realloc(ptr, nsize);
  }
}

int aux_esp_fclose(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  espf->close();
  //delete fp;
  return 0;
}

int aux_esp_feof(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->available() == 0 ? 1 : 0;
}

int aux_esp_getc(EspFile *fp){
  if (fp->lastUngetC != EOF){
    int aux = fp->lastUngetC;
    fp->lastUngetC = EOF;
    return aux;
  }
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  int aux = espf->read();
  if (aux == EOF){
    fp->hasError = 1;
  }
  return aux;
}

int aux_esp_ungetc(int n,EspFile *fp){
  fp->lastUngetC = n;
  if (n == EOF){
    return EOF;
  }
  return 0;
}

int aux_esp_fflush(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  espf->flush();
  return 0;
}

int aux_esp_ferror(EspFile *fp){
  return fp->hasError;
}

void aux_esp_clearerr(EspFile *fp){
  fp->hasError = 0;
}

long aux_esp_ftell(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->position();
}

int	aux_esp_fseek(EspFile *fp, long a , int b){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  SeekMode mode = SeekSet;
  if (mode == SEEK_SET){
    mode = SeekSet;
  }else if (mode == SEEK_CUR){
    mode = SeekCur;
  }else if (mode == SEEK_END){
    mode = SeekEnd;
  }
  return espf->seek(a, mode);
}

size_t aux_esp_fread(char * buffer, size_t _size, size_t _n, struct EspFile *fp){
  //Serial.printf("aux_esp_fread->%d\n", (int)fp);
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->readBytes(buffer, _size*_n);
}

size_t aux_esp_fwrite(const void * buffer, size_t _size, size_t _n, struct EspFile *fp){
  //Serial.printf("aux_esp_fwrite->%d\n", (int)fp);
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->write((const uint8_t*)buffer, _size*_n);
}
int aux_esp_fprintf(EspFile * fp, const char *buff, LUAI_UACNUMBER num){
  //Serial.printf("aux_esp_fprintf->%d\n", (int)fp);
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->printf(buff, num);
}

EspFile *custom_fopen_sd(lua_State *L, const char* filename, const char *mode){
  if (filename == NULL){
    return NULL;
  }
  static File myFile;
  myFile = SD.open(filename, mode);
  if (myFile){
    EspFile *fp = new EspFile;
    fp->lastUngetC = -1;
    fp->hasError = 0;
    fp->L = L;
    fp->actualFilePtr = (void*)&myFile;
    fp->esp_fclose = aux_esp_fclose;
    fp->esp_feof = aux_esp_feof;
    fp->esp_getc = aux_esp_getc;
    fp->esp_ungetc = aux_esp_ungetc;
    fp->esp_ferror = aux_esp_ferror;
    fp->esp_clearerr = aux_esp_clearerr;
    fp->esp_ftell = aux_esp_ftell;
    fp->esp_fseek = aux_esp_fseek;
    fp->esp_fflush = aux_esp_fflush;
    fp->esp_fread = aux_esp_fread;
    fp->esp_fwrite = aux_esp_fwrite;
    fp->esp_fprintf = aux_esp_fprintf;
    return fp;
  }
  return NULL;
}

EspFile *custom_fopen_ffat(lua_State *L, const char* filename, const char *mode){
  if (filename == NULL){
    return NULL;
  }
  static File myFile;
  myFile = FFat.open(filename, mode);
  if (myFile){
    EspFile *fp = new EspFile;
    fp->lastUngetC = -1;
    fp->hasError = 0;
    fp->L = L;
    fp->actualFilePtr = (void*)&myFile;
    fp->esp_fclose = aux_esp_fclose;
    fp->esp_feof = aux_esp_feof;
    fp->esp_getc = aux_esp_getc;
    fp->esp_ungetc = aux_esp_ungetc;
    fp->esp_ferror = aux_esp_ferror;
    fp->esp_clearerr = aux_esp_clearerr;
    fp->esp_ftell = aux_esp_ftell;
    fp->esp_fseek = aux_esp_fseek;
    fp->esp_fflush = aux_esp_fflush;
    fp->esp_fread = aux_esp_fread;
    fp->esp_fwrite = aux_esp_fwrite;
    fp->esp_fprintf = aux_esp_fprintf;
    return fp;
  }
  return NULL;
}

EspFile *custom_fopen_SPIFFS(lua_State *L, const char* filename, const char *mode){
  if (filename == NULL){
    return NULL;
  }
  static File myFile;
  myFile = SPIFFS.open(filename, mode);
  if (myFile){
    EspFile *fp = new EspFile;
    fp->lastUngetC = -1;
    fp->hasError = 0;
    fp->L = L;
    fp->actualFilePtr = (void*)&myFile;
    fp->esp_fclose = aux_esp_fclose;
    fp->esp_feof = aux_esp_feof;
    fp->esp_getc = aux_esp_getc;
    fp->esp_ungetc = aux_esp_ungetc;
    fp->esp_ferror = aux_esp_ferror;
    fp->esp_clearerr = aux_esp_clearerr;
    fp->esp_ftell = aux_esp_ftell;
    fp->esp_fseek = aux_esp_fseek;
    fp->esp_fflush = aux_esp_fflush;
    fp->esp_fread = aux_esp_fread;
    fp->esp_fwrite = aux_esp_fwrite;
    fp->esp_fprintf = aux_esp_fprintf;
    return fp;
  }
  return NULL;
}

void custom_serial(const char* msg){
  Serial.println(msg);
}

EspLua::EspLua(bool use_psram) {
  _errorCallback = nullptr;
  if (use_psram){
    _state = lua_newstate(psram_lua_alloc, NULL);
    lua_setallocf(_state, psram_lua_alloc, NULL);
  }else{
    _state = luaL_newstate();
  }
  luaL_openlibs(_state);

  lua_setserial_esp(_state, custom_serial); 
}

void EspLua::setFS_FFat(){
  lua_setfopenf_esp(_state, custom_fopen_ffat); 
}

void EspLua::setFS_SD(){
  lua_setfopenf_esp(_state, custom_fopen_sd); 
}

void EspLua::setFS_SPIFFS(){
  lua_setfopenf_esp(_state, custom_fopen_SPIFFS); 
}


void EspLua::FuncRegisterRaw(const char* name, int functionPtr(lua_State *L)){
  lua_register(_state, name, functionPtr);
}

bool EspLua::dostring(const char *script) {
  int error;
    
  error = luaL_loadstring(_state, script);
  if (error) {
      if (_errorCallback != nullptr){
        _errorCallback(lua_tostring(_state, -1));
      }
      lua_pop(_state, 1);
      return false;
  }

  if (lua_pcall(_state, 0, 0, 0)) {
    if (_errorCallback != nullptr){
        _errorCallback(lua_tostring(_state, -1));
      }
    lua_pop(_state, 1); 
    return false;
  }

  return true;
}

bool EspLua::loadFile(const char *script) {
  if (luaL_dofile(_state, script)){
    const char *error_message = lua_tostring(_state, -1);
    Serial.printf("Error loading file %s:\n%s", script, error_message);
    lua_pop(_state, 1);
    return false;
  }
  return true;
}

