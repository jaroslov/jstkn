JSTKN
-----

This is a small, vaguely conformant, JSON parser that demonstrates a non-recursive parser.
It relies on a hand-rolled stack to track the recursion state. The overhead for recursion
tracking is *at most* 1 bit per level of recursion --- we only need to track if we recursed
because we're in an array (0) or in an object (1).

The worst case overhead for recursion is when parsing a telescoping array, which has a 6%
overhead. In practice, most JSON files won't come anywhere close to approaching this limit.

The layout of the source is as follows:

    /lib
        jstkn.h     -- all of the library, like the STB sources, but awful
    /src
        json.cpp    -- test harness
    /test
        *.json      -- the amazing set of tests
    README.md
    license

TODO:

1. Proper UTF-8 validation.
2. Schema validation, via a schema-callback.

