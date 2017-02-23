How to perform RPCs to the Intel Edison
=======================================

Modules may access the Intel Edison through RPCs. To accomplish this,
we are using [Embedded RPC (ERPC) by nxp](http://www.github.com/lab11/erpc). 

Without diving into the depths of the RPC library, this is what you
must do to send an RPC to the Intel Edison.

##1) Write an Interface Description Language (IDL) file for your RPC

The IDL file describes the arguments and return parameters of an RPC. We
use ERPCs description language for our IDLs. The [documentation and examples
for this language](://github.com/lab11/erpc/blob/master/doc/idl_reference.md).

Write an IDL file for your rpc and name it \<rpc_name\>.erpc.a

##2)Write the tock application using the RPC

Place the \<rpc_name\>.erpc file in the folder for you tock application.

Then you must initialize the RPC by telling the Intel Edison where the RPC
is in its filesystem. After initialization you can call any of the functions
defined in your \<rpc_name\>.erpc directly.

To use the RPC:

```c
#include \<rpc_name\>.h
.
.
.
int result = signpost_processing_init("path/to/rpc/on/edison");
.
.
.
int test = my_rpc(arg1,arg2);
```

##3)Fix up your Makefile

In the makefile of your tock application add the line:

```
DEFINED_IF_APP_USES_ERPC=1
````

##4)Write your RPC implementation

To call the RPC on the intel edison you must implement a python module which
provides a list of RPC services, each of which defines one or more RPC function
implementations. These functions and services much mast the those which you
defined in your IDL file. 

This package name should provide a list of services through a "get_services" function.

We are still working on examples for these python modules. 
