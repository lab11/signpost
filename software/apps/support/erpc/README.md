eRPC Library for Signpost
=========================

The Signpost platform use an [Embedded RPC](https://github.com/lab11/erpc)
library to communicate between Signpost modules and Linux.

The remote procedure calls (RPCs) are defined using in `.erpc` files. The
`erpcgen` tool will then automatically generate source and header files for
these RPCS.

For a simple example, see the [erpc test app](../../apps/tests/erpc_test).


Using RPCs
----------

You will need to list all your `.erpc` files in your application Makefile.
In most cases, the simple wildcard rule will suffice:

    ERPC_SRCS := $(wildcard *.erpc)

You application should `#include` headers as if the local `.erpc` file was
actually a header. That is:

    $ ls
    main.c test_arithmetic.erpc
    $ grep test_arithmetic main.c
    #include "test_arithmetic.h"

The needed header will be automatically generated (in the `build/` folder)
and its location will be added to the compiler's include path.



Build `erpcgen` and `liberpc`
-----------------------------

### erpcgen

Has dependencies on flex, bison, and libboost. Assuming these are installed:

    ./create_erpcgen

### liberpc

    ./create_liberpc

