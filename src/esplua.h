#ifndef LUA_ESP_H
#define LUA_ESP_H

#include "Arduino.h"
#include <string>
#include <type_traits>
#include <vector>
#include <sstream>

#define LUA_USE_C89
#include "lua/lua.hpp"
typedef std::function<int(lua_State*)> LuaCFunctionLambda;

class LuaManager{
    public:
        void static ClearRegisteredReferences(){
            for (auto &it : ptrs){
                delete it;
            }
            ptrs.clear();
        };
        static void AddReference(LuaCFunctionLambda *v){
            ptrs.emplace_back(v);
        }
        template <typename T> static T* GetSelf(){
            lua_getfield(L, 1, "__self");
            T** data = (T**)lua_touserdata(LuaManager::L, -1);
            if (!data){
                return nullptr;
            }
            return (*data);
        }
        template <typename T> static T** GetSelfReference(){
            lua_getfield(L, 1, "__self");
            T** data = (T**)lua_touserdata(LuaManager::L, -1);
            if (!data){
                return nullptr;
            }
            return (data);
        }
        static int ErrorHandler(lua_State* L){
            std::string err;
            size_t len;
            const char *c_str = lua_tolstring(L, -1, &len);
            if(c_str && len > 0)
                err.assign(c_str, len);


            lua_pop(L,1);

            lua_getglobal(L,"debug");
            lua_getfield(L, -1, "traceback");
            lua_remove(L,-2);

            lua_pushlstring(L, err.c_str(), err.length());
            lua_pushinteger(L, 1);
            lua_call(L, 2, 1);

            std::string error = lua_tostring(L, -1);
            lua_pop(L,1);


            lua_pushlstring(L, error.c_str(), error.length());
            return 1;
        }

        static bool Rawcall(int arg = 0,int returns = 0,int ext = 0){
            int ret = lua_pcall(L, arg, returns, ext );
            if (ret != 0){
                Serial.printf("Error: %s", lua_tostring(L, -1));
                return false;
            }
            return true;
        }
        static bool Pcall(int arg = 0,int returns = 0,int ext = 0){
            int previousStackSize = lua_gettop(L);
            int errorFuncIndex = previousStackSize - arg;
            lua_pushcclosure(L, &LuaManager::ErrorHandler, 0);
            lua_insert(L, errorFuncIndex);

            int ret = lua_pcall(L, arg, returns,  errorFuncIndex);

            lua_remove(L, errorFuncIndex);

            if (ret != 0){
                Serial.printf("Error: %s", lua_tostring(L, -1));
                return false;
            }
            return true;
    }
    static std::vector<LuaCFunctionLambda*> ptrs;
    static std::string lastCalled;
    static lua_State *L;
    static std::vector<std::function<void()>> eraseLambdas;

    static void ClearReferences(){
        for (auto &it : eraseLambdas){
            it();
        }
    }
};


template<typename T1> struct GenericLuaType{
     static bool Is(lua_State *L,int stack=-1){
        return false;
    };
};


template<typename T1> struct GenericLuaGetter{
     static T1 Call(lua_State *L,int stackPos = -1,bool pop=true){
        T1 pt;
        //Serial.printf("[GenericLuaGetter]Undefined type called: [%s]", LuaManager::lastCalled.c_str());
        if (lua_isnil(L,stackPos)){
            Serial.printf("[Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
        }else{
             if (!lua_istable(L,stackPos)){
                int ret = lua_tonumber(L,stackPos);
                T1 *pt2 = &pt;
                *((int*)pt2) = ret;
                if (pop)
                  lua_pop(L,1);
                return pt;
             }else{
                Serial.printf("[Warning]Argument %d is table on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
             }
        }
        if (pop)
            lua_pop(L,1);
        return T1();
    };

    static T1 Empty;
};

template<typename T1> struct GenericLuaReturner{
    static void Ret(T1 vr,lua_State *L,bool forceTable = false){
        Serial.printf("[GenericLuaReturner]Undefined type called.");
        lua_pushnil(L);
    };
};

/*
    void
*/
template<> struct GenericLuaReturner<void>{
     static void Ret(int vr,lua_State *L,bool forceTable = false){
        lua_pushnil(L);
    };
};


template<> struct GenericLuaReturner<uint16_t>{
     static void Ret(uint16_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<unsigned long>{
     static void Ret(unsigned long vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<>
    struct GenericLuaGetter<uint16_t> {
     static uint16_t Call(lua_State *L,int stackPos = -1,bool pop=true){
        uint16_t n = 0;
        //Serial.println("foi int_t");
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }
        n = lua_tonumber(L,stackPos);
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static uint16_t Empty;
};

template<>
    struct GenericLuaGetter<unsigned long> {
     static unsigned long Call(lua_State *L,int stackPos = -1,bool pop=true){
        unsigned long n = 0;
        //Serial.println("foi int_t");
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }
        n = lua_tonumber(L,stackPos);
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static unsigned long Empty;
};



template<>
    struct GenericLuaType<uint16_t>{
        static inline bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L,-1);
        };
};

template<>
    struct GenericLuaType<unsigned long>{
        static inline bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L,-1);
        };
};


/*
    uint8
*/
template<> struct GenericLuaReturner<char>{
     static void Ret(char vr,lua_State *L,bool forceTable = false){
        char aux[] = ".";
        aux[0] = vr;
        lua_pushstring(L,aux);
    };
};
template<>
    struct GenericLuaGetter<char> {
     static char Call(lua_State *L,int stackPos = -1,bool pop=true){
        char n = 0;
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static char Empty;
};
template<>
    struct GenericLuaType<char>{
        static bool Is(lua_State *L,int stack=-1){
            if (!lua_isstring(L, stack)){
                return false;
            }
            size_t len;
            luaL_checklstring(L, stack, &len);
            return (len == 1);
        };
};

/*
    uint8
*/
template<> struct GenericLuaReturner<uint8_t>{
     static void Ret(uint8_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<uint8_t> {
     static uint8_t Call(lua_State *L,int stackPos = -1,bool pop=true){
        uint8_t n = 0;
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static uint8_t Empty;
};
template<>
    struct GenericLuaType<uint8_t>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
/////////////

/*
    uint32
*/
template<> struct GenericLuaReturner<uint32_t>{
     static void Ret(uint32_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<uint32_t> {
     static uint32_t Call(lua_State *L,int stackPos = -1,bool pop=true){
        uint32_t n = 0;
        //Serial.println("foi int32_t");
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static uint32_t Empty;
};
template<>
    struct GenericLuaType<uint32_t>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
///////////////

/*
    uint64
*/
template<> struct GenericLuaReturner<uint64_t>{
     static void Ret(uint64_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<uint64_t> {
     static uint64_t Call(lua_State *L,int stackPos = -1,bool pop=true){
        uint64_t n = 0;
        //Serial.println("foi int64_t");
        if (lua_isnil(L,stackPos)){
            
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static uint64_t Empty;
};
template<>
    struct GenericLuaType<uint64_t>{
        static inline bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
/////////

/*
    int
*/
template<> struct GenericLuaReturner<int>{
     static void Ret(int vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<int> {
     static int Call(lua_State *L,int stackPos = -1,bool pop=true){
        int n = 0;
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }
        n = lua_tonumber(L,stackPos);
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static int Empty;
};
template<>
    struct GenericLuaType<int>{
        static inline bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
//////////


/*
    float
*/
template<> struct GenericLuaReturner<float>{
     static void Ret(float vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<float> {
     static float Call(lua_State *L,int stackPos = -1,bool pop=true){
        float n = 0;
        //Serial.println("foi float");
        if (lua_isnil(L,stackPos)){
            Serial.printf("[Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static float Empty;
};
template<>
    struct GenericLuaType<float>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
/////////

/*
    int16
*/
template<> struct GenericLuaReturner<int16_t>{
     static void Ret(int16_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};
template<>
    struct GenericLuaGetter<int16_t> {
     static int16_t Call(lua_State *L,int stackPos = -1,bool pop=true){
        int16_t n = 0;
       //Serial.println("foi int16_t");
        if (lua_isnil(L,stackPos)){
            Serial.printf("[Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = -1;
        }else{
            n = lua_tonumber(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static int16_t Empty;
};
template<>
    struct GenericLuaType<int16_t>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isnumber(L, stack);
        };
};
/////


/*
    bool
*/
template<> struct GenericLuaReturner<bool>{
     static void Ret(bool vr,lua_State *L,bool forceTable = false){
        lua_pushboolean(L,vr);
    };
};
template<>
    struct GenericLuaGetter<bool> {
     static bool Call(lua_State *L,int stackPos = -1,bool pop=true){
        bool n = false;
        if (lua_isnil(L,stackPos)){

            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = "nil";
        }else{
            n = lua_toboolean(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static bool Empty;
};
template<>
    struct GenericLuaType<bool>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isboolean(L, stack);
        };
};
///////

/*
    std::string
*/
template<> struct GenericLuaReturner<std::string>{
     static void Ret(std::string vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr.c_str());
    };
};
template<>
    struct GenericLuaGetter<std::string> {
     static std::string Call(lua_State *L,int stackPos = -1,bool pop=true){
        std::string n;
        if (lua_isnil(L,stackPos)){

            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = "nil";
        }else{
            n = lua_tostring(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static std::string Empty;
};
template<>
    struct GenericLuaType<std::string>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isstring(L, stack);
        };
};
////////


/*
    String
*/
template<> struct GenericLuaReturner<String>{
     static void Ret(String vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr.c_str());
    };
};
template<>
    struct GenericLuaGetter<String> {
     static String Call(lua_State *L,int stackPos = -1,bool pop=true){
        String n;
        if (lua_isnil(L,stackPos)){

            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
            n = "nil";
        }else{
            n = lua_tostring(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static String Empty;
};
template<>
    struct GenericLuaType<String>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isstring(L, stack);
        };
};
////////

/*
    const char *
*/
template<> struct GenericLuaReturner<const char *>{
     static void Ret(const char * vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr);
    };
};
template<>
    struct GenericLuaGetter<const char *> {
     static const char * Call(lua_State *L,int stackPos = -1,bool pop=true){
        const char *ch = "";
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
        }else{
            ch = lua_tostring(L,stackPos);
        }
        if (pop)
            lua_pop(L,1);
        return  ch;
    };
};
template<>
    struct GenericLuaType<const char *>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_isstring(L, stack);
        };
};
////////

/*
    std::vector<int>
*/
template<> struct GenericLuaReturner<std::vector<int>>{
     static void Ret(std::vector<int> vr,lua_State *L,bool forceTable = false){
        lua_newtable(L);
        auto index = 1;
        for (const auto& value : vr) {
            lua_pushinteger(L, value);  
            lua_rawseti(L, -2, index++);
        }
    };
};
template<>
    struct GenericLuaGetter<std::vector<int>> {
     static std::vector<int> Call(lua_State *L,int stackPos = -1,bool pop=true){
        std::vector<int> n;
        if (lua_isnil(L,stackPos)){
            Serial.printf("[LuaBase][Warning]Argument %d is nil on %s",lua_gettop(L), LuaManager::lastCalled.c_str());
        }else{
            lua_pushnil(L);
            while (lua_next(L, 1)) {
                if (lua_isnumber(L, -1)) {
                    n.push_back(lua_tointeger(L, -1));
                }
                lua_pop(L, 1);
            }
        }
        if (pop)
            lua_pop(L,1);
        return n;
    };
    static std::vector<int> Empty;
};
template<>
    struct GenericLuaType<std::vector<int>>{
        static bool Is(lua_State *L,int stack=-1){
            return lua_istable(L, stack);
        };
};



class LuaCaller{
    public:
        template <int N> static int BaseEmpty(lua_State *L){
            if (!L){
                return 0;
            }
            LuaCFunctionLambda **v = (LuaCFunctionLambda **)lua_touserdata(L, lua_upvalueindex(N));
            if (!v || !(*v)){
                lua_pushstring(L,"[LUA]could not call closure %d because null reference");
                lua_error (L);
                return 0;
            }
            (*(*v))(L);
            return 1;
        }

        template <int N> static int Base(lua_State *L){
            LuaCFunctionLambda **v = (LuaCFunctionLambda **)lua_touserdata(L, lua_upvalueindex(N));
            if (!v || !(*v)){
                lua_pushstring(L,"[LUA]could not call closure %d because null reference");
                lua_error (L);
                return 0;
            }
            int **self = LuaManager::GetSelfReference<int>();
            (*(*v))(L);
            return 1;
        };
};

template<typename ValueType> struct LuaTyper{
    static ValueType& GetTypeIfSame(ValueType aux,ValueType& karg){
        return karg;
    };
    template<typename K> static ValueType& GetTypeIfSame(ValueType aux,K& karg){
        ValueType *auxa;
        auxa = (ValueType *)&karg;
        return *auxa;
    };

};

template<class T> class LuaTypeConverterThing{
    public:

    /*static T& Convert(T &thing){
        return thing;
        static std::string GetTypeIfSame(std::string aux,Rect karg){
        return utils::format("{x = %f, y = %f, w = %f, h = %f}", karg.x, karg.y, karg.w, karg.h);
    };};*/
    static T Convert(T thing){
        return thing;
    };

    template<typename T2> static T& Convert(T2 &thing){
        static T st;
        T2 *aux = &thing;
        st = *((T*)(aux));
        return st;
    };
};


template<int N>
    struct readLuaValues {
    template<typename Tuple> static inline void Read(Tuple& tuple,lua_State *L,int stackpos = -1,int offsetStack=0) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        ValueType v = GenericLuaGetter<ValueType>::Call(L,stackpos);
        std::get<N-1>(tuple) = v;
        readLuaValues<N-1>::Read(tuple,L,stackpos,offsetStack);
    };



    template<typename Tuple,typename K,typename ... Opt> static inline void Read(Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K head, Opt ... tail) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        int argCountLua = lua_gettop(L)-offsetStack;
        if (N <= argCountLua){
            ValueType v = GenericLuaGetter<ValueType>::Call(L,stackpos);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(tuple,L,stackpos,offsetStack,head,tail...);
        }else{
            ValueType v = LuaTyper<ValueType>::GetTypeIfSame(ValueType(),head);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(tuple,L,stackpos,offsetStack, tail...);
        }
    };

     template<typename Tuple,typename K> static inline void Read(Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K headEnd) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        int argCountLua = lua_gettop(L)-offsetStack;
        if (N <= argCountLua){
            ValueType v = GenericLuaGetter<ValueType>::Call(L,stackpos);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(tuple,L,stackpos,offsetStack);
        }else{
            ValueType v = LuaTypeConverterThing<ValueType>::Convert(headEnd);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(tuple,L,stackpos,offsetStack);
        }
    };
};


template<int N, typename Ret> struct expander {
    template<typename ArgStructure,typename Function, typename... Args>
        static inline Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return expander<N-1,Ret>::expand(a,L,f, std::get<N-1>(a), args...);
    }

    template<typename ObjectName, typename ArgStructure,typename Function, typename... Args>
        static inline Ret expandClass(const ArgStructure& a,lua_State *L, ObjectName *obj, Function (ObjectName::*f), const Args&... args) {
            return expander<N-1,Ret>::expandClass(a, L, obj, f, std::get<N-1>(a), args...);
    }
};



template<typename Ret> struct finalCaller {
    template<typename F, typename... Args>
       static inline Ret functionCaller(const F& f,lua_State *L, const Args&... args) {
       Ret ret = f(args...);
       return (Ret)ret;
    };

    template<typename ObjectName, typename Fe, typename... Args>
       static inline Ret functionObjectCaller(Fe (ObjectName::*f),lua_State *L, ObjectName *obj, const Args&... args) {
       Ret ret = (obj->*f)(args...);
       return (Ret)ret;
    };
};

template<> struct finalCaller<void>{
    template<typename F, typename... Args>
       static inline void functionCaller(const F& f,lua_State *L, const Args&... args) {
       f(args...);
       return;
    };

    template<typename ObjectName, typename Fe, typename... Args>
        static inline void functionObjectCaller(Fe (ObjectName::*f),lua_State *L, ObjectName *obj, const Args&... args) {
        (obj->*f)(args...);
        return;
    };    
};

template<typename Ret> struct expander <0,Ret> {
    template<typename ArgStructure, typename Function, typename... Args>
        static inline Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return finalCaller<Ret>::functionCaller(f,L,  args...);
    }

     template<typename ObjectName, typename ArgStructure, typename Function, typename... Args>
        static inline Ret expandClass(const ArgStructure& a,lua_State *L,  ObjectName *obj, Function (ObjectName::*f), const Args&... args) {
            return finalCaller<Ret>::functionObjectCaller(f,L, obj, args...);
    }
};

template<>
    struct readLuaValues<0> {
        template<typename Tuple> static inline void Read(Tuple& tuple,lua_State *L,int stackpos=-1,int offsetStack=0) { };
        template<typename Tuple,typename K> static inline void Read(Tuple& tuple,lua_State *L,int stackpos,int offsetStack, K k) { };
        template<typename Tuple,typename K,typename ... Opt> static inline void Read(Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K k,Opt ... aux) { };
};


// ------------- UTILITY---------------
template<int...> struct index_tuple{};

template<int I, typename IndexTuple, typename... Types>
struct make_indexes_impl;

template<int I, int... Indexes, typename T, typename ... Types>
struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>
{
    typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type;
};

template<int I, int... Indexes>
struct make_indexes_impl<I, index_tuple<Indexes...> >
{
    typedef index_tuple<Indexes...> type;
};

template<typename ... Types>
struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...>
{};




template<int N,typename Tuple,typename ... Opt> static inline void TailOrganizer(Tuple& tuple,lua_State *L,int stackpos,int offsetStack,Opt ... tail) {
    readLuaValues<N>::Read(tuple,L,stackpos,offsetStack,tail...);
}

template<int N,class Ret,class Tupler, class... Args, int... Indexes >
Ret apply_helper( Tupler &arglist,lua_State *L2,int k1,int k2, index_tuple< Indexes... >, std::tuple<Args...>&& tup)
{
    return TailOrganizer<N,Tupler,Args...>(arglist,L2,k1,k2, std::forward<Args>( std::get<Indexes>(tup)) ...); 
}
template<int N,class Ret,class Tupler, class ... Args>
Ret apply(Tupler &arglist,lua_State *L2,int k1,int k2, std::tuple<Args...>&&  tup)
{
    return apply_helper<N,Ret,Tupler,Args...>(arglist, L2, k1, k2, typename make_indexes<Args...>::type(), tup);

}
template<int N,class Ret,class Tupler, class ... Args>
Ret apply(Tupler &arglist,lua_State *L2,int k1,int k2, const std::tuple<Args...>&  tup)
{
    return apply_helper<N,Ret,Tupler,Args...>(arglist, L2, k1, k2, typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
}



template <typename ... Types,typename ... Opt> void LambdaRegisterOpt(lua_State *L,std::string str, void func(Types ... args), Opt ... optionalArgs ){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);

        if (argCount > argMax){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }

        if (argCount < argNecessary){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }
        std::tuple<Types ...> ArgumentList;

        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(ArgumentList,L2,-1,0,tup);
        expander<sizeof...(Types),void>::expand(ArgumentList,L2,func);
        return 0;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};

template <typename T1,typename ... Types,typename ... Opt> void LambdaRegisterOpt(lua_State *L,std::string str, T1 func(Types ... args), Opt ... optionalArgs ){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);

        if (argCount > argMax){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }

        if (argCount < argNecessary){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }
        std::tuple<Types ...> ArgumentList;
            
        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(ArgumentList,L2,-1,0,tup);
        T1 rData = expander<sizeof...(Types),T1>::expand(ArgumentList,L2,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};


template<typename ObjectType, typename ... Types, typename ... Opt> void LambdaRegisterClassOpt(lua_State *L,std::string str, ObjectType *obj, void (ObjectType::*func)(Types ... args), Opt ... optionalArgs){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [obj,func,str,tup](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);

        if (argCount > argMax){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }

        if (argCount < argNecessary){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }
        std::tuple<Types ...> ArgumentList;

        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(ArgumentList,L2,-1,0,tup);
        expander<sizeof...(Types),void>::expandClass(ArgumentList,L2,obj,func);
        return 0;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};





template<typename T1, typename ObjectType, typename ... Types, typename ... Opt> void LambdaRegisterClassOpt(lua_State *L,std::string str, ObjectType *obj, T1 (ObjectType::*func)(Types ... args), Opt ... optionalArgs){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [obj,func,str,tup](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);

        if (argCount > argMax){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }

        if (argCount < argNecessary){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
        }
        std::tuple<Types ...> ArgumentList;

        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(ArgumentList,L2,-1,0,tup);
        T1 rData = expander<sizeof...(Types),T1>::expandClass(ArgumentList,L2,obj,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 0;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};


template<typename T1,typename ... Types> void LambdaRegister(lua_State *L,std::string str, T1 func(Types ... args) ){
    LuaCFunctionLambda f = [func,str](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argCount = sizeof...(Types);
        if (argCount > lua_gettop(L2)){
            Serial.printf("[LUA][6]Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,lua_gettop(L2));
        }
        if (argCount < lua_gettop(L2)){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,lua_gettop(L2));
        }

        std::tuple<Types ...> ArgumentList;
        readLuaValues<sizeof...(Types)>::Read(ArgumentList,L2,-1,0);
        T1 rData = expander<sizeof...(Types),T1>::expand(ArgumentList,L2,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};


template<typename ... Types> void LambdaRegister(lua_State *L,std::string str, void func(Types ... args) ){
    LuaCFunctionLambda f = [func,str](lua_State *L2) -> int {
        LuaManager::lastCalled = str;
        int argCount = sizeof...(Types);
        if (argCount > lua_gettop(L2)){
            Serial.printf("[LUA][6]Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,lua_gettop(L2));
        }
        if (argCount < lua_gettop(L2)){
            Serial.printf("[LUA][6]Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,lua_gettop(L2));
        }

        std::tuple<Types ...> ArgumentList;
        readLuaValues<sizeof...(Types)>::Read(ArgumentList,L2,-1,0);
        expander<sizeof...(Types),void>::expand(ArgumentList,L2,func);
        return 0;
    };
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    LuaManager::AddReference((*baseF));
    lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
    lua_setglobal(L, str.c_str());
};

/*
    Setting constants
*/

template<typename T>
typename std::enable_if<std::is_convertible<T, const char*>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushstring(L, value);
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushinteger(L, value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushnumber(L, value);
}


class EspLua {
  public:
    EspLua(bool use_psram=false);
    bool dostring(const char *script);
    bool dostring(const String *script){
        return dostring(script->c_str());
    }
    bool dostring(String &script){
        return dostring(&script);
    }

    void FuncRegisterRaw(const char* name, int readSdFile(lua_State *L));

    void SetErrorCallback(void Func(const char*)){
        _errorCallback = Func;
    }
    
    void SetPrintCallback(int Func (lua_State *)){
       lua_register(_state, "print", Func);
    }

    template<typename T> void setConstant(const std::string& name, const T& value) {
        
        pushLuaValue(_state, value);
        lua_setglobal(_state, name.c_str());
    }


    template<typename T1,typename ... Types>void FuncRegister(std::string str, T1 func(Types ... args)){
      LambdaRegister(_state,str, func );
    }

    template<typename T1,typename ... Types, typename ... Opt>void FuncRegisterOptional(std::string str, T1 func(Types ... args), Opt ... optionalArgs){
      LambdaRegisterOpt(_state, str, func , optionalArgs... );
    }

    
    template<typename ObjectName, typename T1,typename ... Types, typename ... Opt>void FuncRegisterFromObjectOpt(std::string str, ObjectName *obj, T1 (ObjectName::*func)(Types ... args), Opt ... optionalArgs){
      LambdaRegisterClassOpt(_state,str, obj, func, optionalArgs... );
    }


    template<typename... Args>
    std::string callLuaFunction(const std::string& functionName, Args&&... args) {
        lua_getglobal(_state, functionName.c_str());

        if (!lua_isfunction(_state, -1) || lua_isnil(_state, -1)) {
            lua_pop(_state, 1);
            if (_errorCallback != nullptr){
                _errorCallback("Called function is nil or not a function");
            }
            return "Error: Lua function '" + functionName + "' not found or is not a function.";
        }

        // Push all parameters onto the Lua stack
        (void)std::initializer_list<int>{(pushParam(std::forward<Args>(args)), 0)...};

        int numParams = sizeof...(args);

        if (lua_pcall(_state, numParams, 0, 0) != 0) {
            const char* errorMessage = lua_tostring(_state, -1);
            lua_pop(_state, 1);
            if (_errorCallback != nullptr){
                _errorCallback(errorMessage);
            }
            return std::string(errorMessage);
        }
        
        return "";
    };

    void setFS_FFat();
    void setFS_SD();
    void setFS_SPIFFS();
    bool loadFile(const char *fname);

    lua_State *GetState(){
        return _state;
    }
  private:
    void (*_errorCallback)(const char*);
    lua_State *_state;
    String addConstants();

    void pushParam(std::string parameter){
      lua_pushstring(_state, parameter.c_str());
    }
    void pushParam(int parameter){
      lua_pushnumber(_state, parameter);
    }

    void pushParam(float parameter) {
        lua_pushnumber(_state, parameter);
    }

    void pushParam(int64_t parameter) {
        lua_pushinteger(_state, parameter);
    }

    void pushParam(double parameter) {
        lua_pushnumber(_state, parameter);
    }

    void pushParam(char* parameter) {
        lua_pushstring(_state, parameter);
    }

    void pushParam(const char* parameter) {
        lua_pushstring(_state, parameter);
    }

    void pushParam(char parameter) {
        lua_pushinteger(_state, static_cast<int>(parameter));
    }

    void pushParam(uint8_t parameter) {
        lua_pushinteger(_state, parameter);
    }
};



#endif
