How to build the documentation
==============================

The documentation is written using *quickbook*. This is a technology made for boost,
building on *boostbook*... which in turn is based on *docbook*. It can generate
html, pdfs, and probably some more stuff... Here we are using it for html.

1. Install b2 (bjam) if you have not already.  
2. Install [boostbook](http://www.boost.org/doc/libs/1_61_0/doc/html/boostbook/getting/started.html)  
3. Install [quickbook](http://www.boost.org/doc/libs/1_61_0/doc/html/quickbook/install.html)  
4. Run b2 from this directory  

All of these may be done by downloading a recent copy of boost, and building things in the tools directory thereof.  

To assist you: My `user-config.jam` looks like this right now:

``
using gcc : : g++ ;
using boostbook
  : /home/chris/boostbook/docbook-xsl-1.75.2
  : /home/chris/boostbook/docbook-dtd-4.2
  ;
using xsltproc : /usr/bin/xsltproc ;
using doxygen : /usr/bin/doxygen ;
using fop : /home/chris/boostbook/fop-0.94/fop : : /usr/bin/java ;
using quickbook
  : /home/chris/bin/quickbook
  ;
``

Good luck!
