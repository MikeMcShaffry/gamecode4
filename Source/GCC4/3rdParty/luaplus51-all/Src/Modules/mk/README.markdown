MK is a set of common plumbing that different [WSAPI](http://keplerproject.github.com/wsapi) applications
and frameworks can use to do things like share a single authentication system, easily create inter-application
links, and embed pieces of content created by different frameworks (mixing Orbit and Sputnik content in the same
page, for example).

Look in the `samples` and `test` folders to see examples of the APIs as they are right now. To run these you need
to install the two rockspecs in the `rockspecs` folders, which install MK and a branch of WSAPI which changes that MK needs
(these changes are in a branch because thay can potentially break existing WSAPI applications).
