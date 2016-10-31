# UniversalServerLib
A high performance asynchronous TCP/UDP server. It supports Lua scripts.

Because it depends heavily on IO Completion port, it is Windows only.

## Features
* Based on Windows IOCP, highly scalable
* C++ implementation, some C++11 features used
* Full asynchronous, high performance
* Supports multiple subservers listening on different ports
* Client tools provided
* Simple HTTP support (and Websocket support)
* Lua script supported (Lua 5.3.3)
* Special data objects provided to share data across different threads
* Plug & play scripts modification, no restarting needed
* Realtime files and configurations changes detection
* Built-in supports for simple binary message formats
* RPC server enables custom scripts execution on the fly

## Performance
* more than 20,000 connections at the same time
* more than 20,000 packages received and sent per seconds. (packages processed in lua scripts)
* changes on scripts take effect immediately

>The data above is based on tests on a laptop computer at CPU utilization less than 50%.
>Compiled using MSVC with default Win32 x86 release configurations


## Server Management Tool<br/>
![STL set](https://github.com/huxia1124/UniversalServerLib/blob/master/images/tool.png)
