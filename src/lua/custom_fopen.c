#include "lua.h"
#include "custom_fopen.h"
#include "lstate.h"

EspFile *esp_fopen(lua_State *L, const char* filename, const char *mode){
    lua_FopenF f = G(L)->custom_fopenf;
    
    return f(L, filename, mode);
}

EspFile *esp_tmpfile(lua_State *L){
    return esp_fopen(L, NULL, NULL);
}

int esp_fclose(EspFile *fp){
    if (fp->esp_fclose != NULL){
        return fp->esp_fclose(fp);
    }
    return -1;
}

int	esp_feof (EspFile *fp){
    if (fp->esp_feof != NULL){
        return fp->esp_feof(fp);
    }
    return -1;
}
size_t esp_fread (char * buffer, size_t _size, size_t _n, EspFile *fp){
    if (fp->esp_fread != NULL){
        return fp->esp_fread(buffer, _size, _n, fp);
    }
    return -1;
}

int	esp_getc (EspFile *fp){
    if (fp->esp_getc != NULL){
        return fp->esp_getc(fp);
    }
    return -1;
}
int	esp_ferror (EspFile *fp){
    if (fp->esp_ferror != NULL){
        return fp->esp_ferror(fp);
    }
    return -1;
}

int	esp_ungetc (int c, EspFile * fp){
    if (fp->esp_ungetc != NULL){
        return fp->esp_ungetc(c, fp);
    }
    return -1;
}


long esp_ftell ( EspFile * fp){
    if (fp->esp_ftell != NULL){
        return fp->esp_ftell(fp);
    }
    return -1;
}

int	esp_fseek (EspFile *fp, long a , int b){
    if (fp->esp_fseek != NULL){
        return fp->esp_fseek(fp, a, b);
    }
    return -1;
}

int	esp_fflush (EspFile *fp){
    if (fp->esp_fflush != NULL){
        return fp->esp_fflush(fp);
    }
    return -1;
}

int	esp_setvbuf (EspFile *fp, char *buff, int sz, size_t len){
    if (fp->esp_setvbuf != NULL){
        return fp->esp_setvbuf(fp, buff, sz, len);
    }
    return -1;
}

size_t esp_fwrite (const void *buffer , size_t _size, size_t _n, EspFile *fp){
    if (fp->esp_fwrite != NULL){
        return fp->esp_fwrite(buffer, _size, _n, fp);
    }
    return 0;
}

void esp_clearerr (EspFile *fp){
    if (fp->esp_clearerr != NULL){
        return fp->esp_clearerr(fp);
    }
}


int	esp_fprintf (EspFile *fp, const char *buff2, LUAI_UACNUMBER num){
    if (fp->esp_fprintf != NULL){
        return fp->esp_fprintf(fp, buff2, num);
    }
    return 0;
}
