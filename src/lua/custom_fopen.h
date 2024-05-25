
#include "lua.h"


EspFile *esp_fopen(lua_State *L, const char* filename, const char *mode);
EspFile *esp_tmpfile(lua_State *L);
int esp_fclose(EspFile *fp);
int	esp_feof (EspFile *);
size_t esp_fread (char * buffer, size_t _size, size_t _n, EspFile *);
int	esp_getc (EspFile *fp);
int	esp_ungetc (int, EspFile *);
int	esp_ferror (EspFile *fp);
long esp_ftell ( EspFile *);
int	esp_fseek (EspFile *fp, long a , int b);
void esp_clearerr (EspFile *);
int	esp_fflush (EspFile *);


int	esp_setvbuf (EspFile *fp, char *buff, int, size_t);
size_t	esp_fwrite (const void *__restrict , size_t _size, size_t _n, EspFile *fp);
int	esp_fprintf (EspFile *__restrict, const char *__restrict, LUAI_UACNUMBER num);
