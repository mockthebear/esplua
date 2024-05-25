# ESP Lua

This lib allows easy integration between C++ and Lua on esp32 platforms.

## Memmory

Since lua alone occupy most of the esp32 ram, it is possible to initialize it using the psram
```cpp
lua = new EspLua(true); //Use true to alloc most of it on psram
```

## File system

If lua id directly ported to esp32 without any tweaks, some resources are unavaliable. Those are the `io.open`, `dofile` and `require` functions. This library fixes this by integrating any of the `SD` `FFat` or `SPIFFS` file system. So using `io` functions directly integrates with the defined file system.

Choose the correct file system with one of:
```cpp
lua->setFS_SD();
lua->setFS_FFat();
lua->setFS_SPIFFS();
```

## Automatic integration

Usually when integrating a function in C to be used on lua, you have to deal with alot of work converting the values in the lua stack to the c function values. This library use the powers of C++11 and templates to do this automatically:

```cpp
// The definition is uint16_t analogRead(uint8_t pin);  This call automatically infer the types and work to bing the function with the correct types
lua->FuncRegister("analogRead", analogRead);

float functionWithParameters(int a, int b, int c, float d){
	return (a + b + c)*d;
}
//Some functions can have optional parameters. In this declaration the optional param,eters are backwards oriented, so it is d=1.0f, c=3 and b=2
//The a parameter is obrigatory.
lua->FuncRegisterOptional("functionWithParameters", functionWithParameters, 1.0f, 3, 2);
//Can be called like: functionWithParameters(1);  functionWithParameters(1, 0), functionWithParameters(1, 0, 0) and functionWithParameters(1, 0, 0, 0.5);

//The set constant also work the same way!
lua->setConstant("OUTPUT", 0x3);
lua->setConstant("STRING", "Some text");
 ```  

# WIP

A few features are a still not working and others are WIP

* **Require** - Somehow, calling require, crashes. Use `dofile` instead
* **Writing files** - I still haven't implemented the write correctly.
* **OS related lib** - Well, since there is no actual OS.. It makes no sense.
* **STDOUT / STDERR** - Lua natively outputs on the stdout. I mapped to output on the serial. Not quite the best solution and not deeply tested. So use it with care