/** \file
  * Load basic comet libraries.
  */
/*
 * Copyright © 2000, 2001 Sofus Mortensen, Michael Geddes
 *
 * This material is provided "as is", with absolutely no warranty
 * expressed or implied. Any use is at your own risk. Permission to
 * use or copy this software for any purpose is hereby granted without
 * fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is
 * granted, provided the above notices are retained, and a notice that
 * the code was modified is included with the above copyright notice.
 *
 * This header is part of Comet version 2.
 * https://github.com/alamaison/comet
 */

#ifndef COMET_COMET_H
#define COMET_COMET_H

#include <comet/config.h>

/**
 @mainpage Comet

 See https://github.com/alamaison/comet

 See \ref comethistory.

 \section cometinfocontents Information Pages
 <ul>
 <li>\ref tlb2husage</li>
 <li>\ref tlb2handidl</li>
 <li>\ref cometcomptr</li>
 <li>\ref cometrawcomaccess</li>
 <li>\ref cometconnectionpoints</li>
 <ul>
 <li> \ref cometconnectionpointssource</li>
 <li> \ref cometconnectionpointsink</li>
 </ul>
 <li>\ref comettstring</li>
 <li>\ref cometdefineconfiguration</li>
 <li>\ref cometcatchallexception</li>
 <li>\ref cometclassfactory</li>
 <li>\link custom_registration Custom Registration\endlink</li>
 <li>\ref comethandleexception</li>
 <li>\ref cometcalllogging</li>
 <li>\ref cometautoexp</li>
 <ul><li>\ref cometautoexpusage</li></ul>
 </ul>
 */

/**
 * \page cometrawcomaccess Raw COM Acces
 * The designers of Comet have tried to avoid making the wrappers do too much
 * implicitly, prefering discrete but explicit ways of triggering functionality.
 *
 * In order to provide interaction of Comet classes with their COM counterparts,
 * a standard set of methods have been used, effectively providing a COM wrapper
 * <i>concept</i>.
 *
 * <ul>
 * <li><b>in()</b>: Return a pointer suitable for [in] parameters,</li>
 * <li><b>out()</b>: Return a pointer suitable for [out] parameters, first freeing memory allocated,</li>
 * <li><b>inout()</b>: Return a pointer suitable for [in,out] parameters.</li>
 * <li><b>detach()</b>: Detach and return the internal pointer from the object</li>
 * <li><b>in_ptr()</b>: {optional} Return a pointer suitable for [out] parameter where a pointer is required, first freeing memory allocated</li>
 * <li><b>get()</b>: {optional} Returns a pointer to the internal pointer (same as STL pointers)</li>
 * <li><b>auto_attach( RAW_TYPE )</b>: Cause an assignment to attach the object to the specified raw COM type.</li>
 * <li><b>create_reference( RAW_TYPE )</b>: Static method to create a propper wrapper reference to the specified raw object.</li>
 * <li><b>create_const_reference( RAW_TYPE )</b>: Static method to create a const wrapper reference to the specified raw object.</li>
 * </ul>
 */

/** \page cometcatchallexception Exception Catching Policy
In an ideal world, Microsoft would have adhered to the C++ standard, and made
<kbd>catch(...)</kbd> not catch system exceptions like 'Access Violation',
however this is not the case.

To make it worse, not only do system exceptions get caught by <kbd>catch(...)</kbd>,
they also <b>bypass the stack unwinding</b>.  This is quite problematic in an
environment where this mechanism is relied upon for resource management, commonly
'only' memory, but also including multithreading acces control mechanisms.

One of the big problems with Access Violations is that it is an unplanned
exception, and in many situations will leave data structures incomplete,
resources locked or other equally vicious artifacts. Essentially, it breaks
invariants.  One viable take on this is that if you don't know where an
exception came from, you shouldn't be catching it.

A big advantage of letting access violations and it's ilk (or any other
unplanned exception) through is that these will then get caught by the JIT (Just
In Time) handlers such as Dr. Watson and Compiler environments that allow
immediate or post-mortem debugging of the problem.  A lot of projects (like
Mozilla) have their own bug-reporting which can intercept the bugs at the top
level and generate their own crash dumps, and catching these exceptions is
actually going to bypass a very useful mechanism.

All this is to say that we have made a shift away from exception handling
policies that cause system expeptions to be caught, however in order to
allow as much flexibility as possible, an improved exception handling mechanism
has been added to the interface implementations and to the server implementation
that uses catch and rethrow to allow logging and custom handling of all exceptions.

The default action of the mechanisms will be to finally rethrow unknown
exceptions.  The good thing about this is that the rethrow behaves to a JIT handler
as if the expeption had never been caught.

*/

/** \page cometautoexp Comet AutoExp.Dat Support
There is a largely undocumented feature is Visual Studio 6 and 7 that allows
you to create a dll 'ADDIN' that provides custom formatting for variables in the
debugger.  CometAutoExp.dll allows you to view simple automation types in more
detail than currently provided internally by the debugger.

By enabling this, you will get to see real date/times when you view datetime_t, you
get to see SAFEARRAY types (including IIDs), and possibly their contents, you
will see full VARIANT types, including SAFEARRAYs.

The file in comet/src/cometautoexp/AutoExp.dat shows the entries that will
be needed to included in your own AutoExp.dat for this to work. (Usually
found at "c:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\AUTOEXP.DAT")

See \ref cometautoexpusage for more information on how to use this.
 */

/** \page cometdefineconfiguration Comet Configuation
 While Comet barely uses macros for the import work of creating a COM library,
 there are a number of preprocessor switches that are used to alter certain
 behaviours and select code.  While most of these refer to the compiler being used,
 there are a few that are intended to be used as external switches.

 <ul><li><b>COMET_ALLOW_DECLSPEC_PROPERTY</b> : Wrapper headers include declspec
 properties, allowing interface properties to be assigned-to and read as
 properties.</li> <li><b>COMET_ASSERT_THROWS</b> : Comet throws
 comet::assert_failed when internal assertions fail in debug builds.</li>
 <li><b>COMET_ASSERT_THROWS_ALWAYS</b> : Comet throws comet::assert_failed when
 internal assertions fail in both debug and release builds.</li>
 <li><b>COMET_LOGFILE</b> : See \ref cometcalllogging.</li>
 <li><b>COMET_LOGFILE_DEFAULT</b> : See \ref cometcalllogging. </li>
 <li><b>COMET_NO_MBCS</b> : Disable comet multi-byte-character-set conversion handling.</li>
 <li><b>COMET_NO_MBCS_WARNING</b> : Turn off the warning indicating that sdt::string is not compatible with MBCS.</li>
 <li><b>COMET_USE_RAW_WRAPPERS</b> : Tells com_ptr not to use the comet wrappers but to return raw interface pointers. </li>
  </ul>

  There are also a number of macros that are either utitlity macros, reflect compiler differences
  or Comet version differences that may be convenient to use within your own code.

  <ul>
  <li><b>COMET_ASSERT(expression)</b> : Run-time assert.</li>
  <li><b>COMET_STATIC_ASSERT(static-expression)</b> : Compile-time assert.</li>
  <li><b>COMET_NOTUSED(info)</b> : Specify unused argument.</li>
  <li><b>COMET_FORCEINLINE </b> : Use before a method to apply force-inline if available. </li>
  <li><b>COMET_VARIANT_TRUE,COMET_VARIANT_FALSE</b> : Use in place of VARIANT_TRUE,VARIANT_FALSE to avoid type warnings.</li>
  <li><b>COMET_MAJOR_VER </b> : Major comet version</li>
  <li><b>COMET_MINOR_VER </b> : Minor comet version</li>
  <li><b>COMET_BUILD </b> : Comet version to compare against (yyyymmdd of distribution)</li>
  <li><b>COMET_STRICT_TYPENAME </b> : Used for when stricter compilers require 'typename' keyword and VC6 has an error.</li>
  <li><b>COMET_CONST_TYPE(vartype, varname, value)</b> : Define a typed constant for a class/struct (or use enum if not supported) </li>
  </ul>

 */

/** \page comethistory Comet History
\par Version 1 gamma 32 ( )
\li Support for VC++2005 (Sofus Mortensen)
\li Numerous bug fixes and better support for Intel C++ (Richard Crossley)
\li Fixed off-by-one bug regarding array sizes (Yunusov Bulat)

\par Version 1 beta 31 ( )
\li safearray.h - Fix a few omissions/bugs (Lijun Qin)
\li comet_lint.lnt - Add pclint options file. (Michael Geddes)
\li server.h - Fix singlton_autorelease class (Ralf Jürgensen)
\li datetime.h - Fix bug in century non-leap-years. (Michael Geddes)
\li datetime.h - Use julian date conversions from boost - to drastically improve split/join efficiency (Michael Geddes)
\li datetime.h - Make invalid() rather than valid() the default (Michael Geddes)
\li date.h - Make use of above improvements in datetime_t (Michael Geddes)
\li datetime.h/date.h - Add ability to construct/assign invalid/null dates from dt_null, dt_invalid (Michael Geddes)
\li datetime.h - Fixed bug in to_unixtime (Lijun Qin)
\li array.h - Fixed typename problem (Sofus Mortensen)
\li safearray.h - Fixed problem with sa_debug_iterator and MSVC 7.1 (Sofus Mortensen)
\li uuid_fwd.h - Fixed bug in !operator (Sofus Mortensen)

\par Version 1 beta 30 (2004-3-5)
\li Support for VC7.1 partial specialisation. (Lijun Qin)
\li datetime.h - Bugfix in datetime_t conversion (Gordon Donaldson)
\li safearray.h - Separate safearray_t::resize into resize and resize_bound to avoid ambiguous conversions. (Michael Geddes)
\li safearray.h - Fix bug in safearray_t:resize_bound using an incorrect copy. (Michael Geddes)
\li safearray.h - Add safearray_t debug iterator for debug builds. (Michael Geddes)
\li safearray.h - Add more comments for safearray_t (Michael Geddes)
\li safearray.h - Support more container-type methods (erase,push_front,push_back,pop_front,pop_back) (Michael Geddes)
\li server.h - Add auto-release singleton (Michael Geddes)
\li uuid.h - uuid_t::operator! no longer reversed (Lijun Qin)
\li uuid.h - fix ambiguous ++ operator usage. (Lijun Qin, Bulat Raditch, Michael Geddes)
\li handle.h - Extension of handles for GDI objects for cometw (Michael Geddes/Vladimir Voinkov/Lindgren Mikael)
\li bstr.h - Throw comparison errors if they occur. (Michael Geddes)
\li cp.h - add 'is_connected' method to connection points. (Lijun Qin)
\li common.h - Support for pclint (Michael Geddes, Gabriel Barta)
\li datetime.h - Support for timezones (Michael Geddes)
\li datetime.h - Fix up buffer handling (Michael Geddes, Lijun Qin)
\li variant.h - Support for strings with embedded nulls (Michael Geddes)
\li tlb2h - Support for bool in structs and in safearrays (Michael Geddes)
\li tlb2h - Support for wrapper renaming of specific methods on specific interfaces/struct/unions. (Michael Geddes)
\li tlb2h - Dual interfaces inherited from an interface in a different type-library now works when called by dispatch (Michael Geddes)
\li tlb2h - Output of HTML now better restricted to only required interfaces. (Michael Geddes)
\li tlb2h - Obey COM rules that require [out] pointers to be initialised on failure (Michael Geddes)
\li tlb2h - Support implementation of an interface that inherits from an interface from a different typelibrary (Michael Geddes)
\li cometautoexp - Support for more types, and better handling of strings. (std::basic_string, COLEDateTime) (Michael Geddes)
\li date.h - Added dateonly_t class. (Michael Geddes)
\li uuid.h - Make the strings accessors consistant (Michael Geddes, Lijun Qin)

\par Version 1 beta 29 (2003-12-30)
\li Added loads of documentation, reduced doxygen warnings to 1(Michael Geddes)
\li Added CometAutoExp.dll extended support for AutoExp.dat
\li Remove unnecessary catch(...) - use specific catches (Michael Geddes)
\li Support create_const_reference in all wrapper classes. (Michael Geddes)
\li tlb2h - Enhance support for error information (interface/coclass name) (Michael Geddes)
\li tlb2h - Add support for logging calls &amp; exceptions. (Michael Geddes)
\li tlb2h - Support for special characters in help strings. (Michael Geddes)
\li tlb2h - Few tweaks to get correct output. (Michael Geddes)
\li tlb2h - Option to show version (Michael Geddes)
\li bstr.h - Work with _STLP_DEBUG wrappers (Michael Geddes)
\li bstr.h - Constructor works with MBCS (Michael Geddes)
\li bstr.h - Support for {} round uuid_t (Michael Geddes)
\li cp.h - Add convenience methods. (Michael Geddes)
\li datetime.h - Constructor to supply date and time (Michael Geddes)
\li datetime.h - Support output streams. (Michael Geddes)
\li error.h - Support for source, iid and helpfile in errors. (Michael Geddes)
\li interface.h - Provide extra comtype definitions. (Michael Geddes)
\li interface.h - Provide convenience macro to define extra comtype with IID_ instances. (Michael Geddes)
\li ptr.h - Add identity_ptr class that references an identity IUnknown. All other comparisons QI on <b>both</b> pointers. (Michael Geddes)
\li server.h - Add support for singelton classes. (Michael Geddes)
\li tlbinfo.h - Fix up reference counting on struct wrappers. (Michael Geddes)
\li tlbinfo.h - Support ITypeInfo2 (Michael Geddes)
\li variant.h - Added explicit converts (as_uchar(), as_uint(), as_double() etc) (Michael Geddes)
\li variant.h - Added is_empty(), is_null() and is_nothing() (Michael Geddes)

\par Version 1 beta 28 (2003-6-18)
\li Support for Visual Studio 2003

\par Version 1 beta 27 (2003-3-5)
\li tlb2h - bug fix variant_t members of dispinterfaces (Michael Geddes)
\li tlb2h - support for safearrays of interfaces (Michael Geddes)

\par Version 1 beta 26 (2003-2-1)
\li tlb2h - major update. See http://groups.yahoo.com/group/tlb2h/message/706 (Michael Geddes)
\li Enum.h - allow copy policies with members (Michael Geddes)
\li datetime.h - fixed bug in timeperiod_t::split (Michael Geddes)

\par Version 1 beta 25 (2002-11-28)
\li Connection point implementation is now customisable via traits. See http://groups.yahoo.com/group/tlb2h/message/688 (Michael Geddes)
\li Fixed bug in str_formattime in datetime.h (Sofus Mortensen).
\li Fixed bug in add_months in datetime.h (Michael Geddes).

\par Version 1 beta 24 (2002-11-13)
\li Major update of tlb2h. See http://groups.yahoo.com/group/tlb2h/message/659 and http://groups.yahoo.com/group/tlb2h/message/661. (Michael Geddes)
\li Fixed problem with variant_t::operator==. (Kyle Alons)
\li Fixed bug in bstr_t::s_str(), bstr_t::is_empty(), bstr_t::length() (Mikael Lindgren, Michael Geddes)
\li safearray_t: 1. Changed the constructor that takes two iterators to not have a default lowerbound (to be consistant with the other constructor)
2. Changed resize to allow specifying a lower-bound.
3. Changed the default lb to be 0 in the case where none is specified and where the safearray is NULL. (Michael Geddes)

\par Version 1 beta 23 (2002-9-1)
\li Bug fixes to namespace handling. (Michael Geddes)
\li Added friendly error message for missing tlbinf32.dll. (Sofus Mortensen)
\li Worked around MSVC6 internal compiler error when contructing an enum. (Michael Geddes)
\li Bug fixes to currency.h (Michael Geddes)
\li Bug fixes to datetime.h (Michael Geddes)

\par Version 1 beta 22 (2002-8-19)
\li Update of tlb2h - better handling of namespace mangling, and support for
unwrapping the type of aliases without unwrapping the name of the aliases (Michael Geddes).
\li Fixed bug in uuid_fwd. (John Harrison)
\li Added oleidl_comtypes.h. (Gabriel Barta)
\li Restored old functionality of try_cast - will no longer throw when casting a null pointer. (Sofus Mortensen)
\li New auto-locking implementation of safearray_t::create_reference and safearray_t::create_const_reference. (Michael Geddes, Sofus Mortensen)
\li Included first drop (pre-alpha version) of cometw. (Vladimir Voinkov)

\par Version 1 beta 21 (2002-6-21)
\li Fixed null pointer bug in com_error::what(). (Kyle Alons)
\li Fixed bug to do with the unloading of typelibraries when refcount reaches zero. (Michael Geddes)
\li Added support for MBCS to std::string and const char* conversions for bstr_t. Only active when _MBCS is defined. (Michael Geddes)
\li Fixed locking/unlocking bug. (Vladimir Voinkov, Sofus Mortensen)
\li Fixed bug in safearray_t::create_reference(variant_t&). (Michael Geddes)
\li Various fixes to datetime and currency. (Michael Geddes)
\li Added constants COMET_VARIANT_TRUE and COMET_VARIANT_FALSE in order to avoid warnings. (Michael Geddes, Sofus Mortensen)
\li registry::value has been extended with overload for method str and dword, where a default value can be supplied if the key/value does not exist. (Sofus Mortensen)
\li Various patches for BCC. (Michael Geddes)
\li Solved problem with vtables gaps when inheriting from those interfaces. (Michael Geddes)
\li Removed a few level 4 warnings. (Michael Geddes)
\li Added experimental HTML generation to tlb2h. (Sofus Mortensen)

\par Version 1 beta 20 (2002-4-9)
\li tlb2h will fall back to using raw structs for structs with types that cannot be wrapped. (Sofus Mortensen)
\li Added application wizard for MSVC.NET. See tlb2h/src/Comet. (Sofus Mortensen)
\li Eliminated a few warnings. (Sofus Mortensen)

\par Version 1 beta 19 (2002-3-26)
\li Added caching of IRecordInfo to sa_traits for wrapped structs and ITypeInfo to IDispatch implementations. (Sofus Mortensen)
\li Fixed problem with safearray_t iterators on MSVC7. (Sofus Mortensen)
\li Fixed bug regarding range checking for safearray_t::at being off by one. (Sofus Mortensen)
\li Added range checking assertion (debug only) to safearray_t::operator[]. (Sofus Mortensen)
\li Changed safearray_t constructor, so that lower bound of one is no longer assumed. Instead lower bound must be specified. (Sofus Mortensen)

\par Version 1 beta 18 (2002-3-25)
\li Major revision of bstr_t. (Sofus Mortensen)
\li Added experimental support for wrapped structs to tlb2h and safearray. (Sofus Mortensen)
\li Fixed problem with having a coclass as a connection point sink. (Sofus Mortensen)
\li Revised treatment of [in, out] parameters in tlb2h. The old implementation could cause double deletion of [in. out] parameters when an exception was thrown. (Sofus Mortensen)
\li Revised all wrapper classes, in order for create_reference to create a mutable reference and create_const_reference an immutable reference. (Sofus Mortensen)
\li Revised locking in safearray_t. safearray_t::create_reference and safearray_t::create_const_reference is no longer doing automatic locking. (Sofus Mortensen)
\li tlb2h now generates #import alike smart pointer typedef. Ie. interface IFoo will be matched with a typedef com_ptr<IFoo> IFooPtr. (Sofus Mortensen)
\li Added support for datetime_t and currency_t to safearray_t. (Sofus Mortensen)
\li Added t_str() to bstr_t for converting to tstring. (Sofus Mortensen)

\par Version 1 beta 17 (2002-2-18)
\li Fixed bug in atl_module. (Michael Geddes)
\li Documentation update for bstr.h. (Michael Geddes)
\li bstr_t now supports strings with embedded nulls. Thanks to Eric Friedman for reporting this. (Sofus Mortensen)
\li Removed use of _alloca in bstr.h. (Sofus Mortensen)

\par Version 1 beta 16 (2002-2-12)
\li Fixed bug in functors bstr::less, bstr::less_equal, bstr::greater, bstr::greater_equal, bstr::equal_to and bstr::not_equal_to. (Michael Geddes)
\li tlb2h: switched ResolveAliases off per request from Michael Geddes.
\li tlb2h: Removed dummy wrapper methods (see http://groups.yahoo.com/group/tlb2h/message/476). (Sofus Mortensen)

\par Version 1 beta 15 (2002-2-1)
\li Added lw_lock class based on Brad Wilson's LightweightLock (see http://www.quality.nu/dotnetguy/archive/fog0000000007.aspx). Many thanks to Brad for making LightweightLock open source. (Sofus Mortensen)
\li Comet now works with MSVC7. (Sofus Mortensen)
\li Removed functor.h. (Sofus Mortensen)
\li Fixed serious bug in bstr_base - causing memory corruption when concat'ing bstr's. (Michael Geddes)
\li Fixed obvious typo in locking_ptr. (Vladimir Voinkov)
\li Removed unnecessary include dependency from scope_guard.h. (Vladimir Voinkov)
\li Fixed compatibility problem with STLport for bstr comparison functors. (Michael Geddes)
\li Removed level 4 warnings from currency.h. (Michael Geddes).
\li Fixed problem with the -c option in tlb2h. (Michael Geddes).
\li Fixed bug in ATL_module.h. (Michael Geddes)
\li impl_dispatch is now lazy loading TypeInfo. Better performance for dual interfaces where client is not using IDispatch. (Sofus Mortensen)
\li Fixed various bugs in tlb2h. (Sofus Mortensen)

\par Version 1 beta 14 (2002-1-17)

\li Fixed problem with dispinterfaces and [in, out] enum parameters. (Sofus Mortensen)
\li Added simple system to prevent compiling out-of-date tlb2h generated headers. (Sofus Mortensen)
\li Fixed bug in impl_dispatch. (Michael Geddes, Sofus Mortensen)

\par Version 1 beta 13 take 4 (2002-1-11)

\li Fixed problem with dispinterfaces and enum parameters reported by Kyle Alons. (Sofus Mortensen)
\li Fixed serious bug in iterators for safearray_t reported by Steve Broeffle. The bug was caused by a MSVC compiler bug. (Sofus Mortensen)
\li Fixed bug operator in operator[] in iterators for safearray_t. (Sofus Mortensen)

\par Version 1 beta 13 take 3 (2002-1-9)

\li Fixed bug in uuid_t introduced in beta 13 take 2. (Sofus Mortensen)

\par Version 1 beta 13 take 2 (2002-1-7)

\li Beta 13 couldn't compile with _UNICODE defined. Fixed now. (Sofus Mortensen)

\par Version 1 beta 13 (2002-1-7)

\li Added basic support for writing exe servers in Comet. (Mikael Lindgren, Sofus Mortensen)
\li Added simple command line parser class to new header comet/cmd_line_parser.h (Mikael Lindgren)
\li Added new header comet/tstring.h. Defines tstring, tistream, tostream, etc, etc. (Sofus Mortensen)
\li Fixed various bugs in uuid_t. (Sofus Mortensen)

\par Version 1 beta 12 (2001-12-20)

\li Revision of embedded_object. embedded_object now takes parent type as first argument, arguments 2, 3, 4, .. denotes interfaces to implement.
    embedded_object_1 has been removed. embedded_object2 has been added, extends embedded_object with functionality to
    disconnect a child object from its parent.  (Sofus Mortensen with help from Kyle Alons).
\li Added new file dispatch.h with specialisation of wrap_t<IDispatch> with similar (but superior) functionality as CComDispatchDriver.
\li Added new class dynamic_dispatch to dispatch.h. Use dynamic_dispatch to dynamically implement IDispatch -  adding methods and properties at runtime. (Sofus Mortensen)
\li Changed interface of sink_impl so that unadvise no longer takes an argument. Additionally the destructor will unadvise if necessary. (Sofus Mortensen)
\li Added new file uuid.h with GUID/UUID/CLSID wrapper called uuid_t. (Sofus Mortensen)
\li Changed tlb2h to support new wrapper uuid_t. (Sofus Mortensen)
\li Fixed potential thread safety problem regarding class factories. (Sofus Mortensen, Paul Hollingsworth)

\par Version 1 beta 11 (2001-11-16)

\li Major fix to tlb2h. tlb2h now supports properties in dispinterfaces. (Sofus Mortensen)
\li Module constants are now static. (Sofus Mortensen)
\li tlb2h now skips non-COM interfaces that do not derive (directly or transitively) from IUnknown. (Sofus Mortensen)
\li Fixed problem with error messages ending up as garbage in tlb2h. (Kyle Alons)
\li Various fixes regarding safearray_t. (Michael Geddes and Sofus Mortensen)
\li Various other minor fixes. (Sofus Mortensen)

\par Version 1 beta 10 (2001-10-17)

\li class coclass changed so that the 3, 4, ... template parameters specifify additional interfaces to implement.
    Example:
    \code
    template<> coclass_implementation<Foo> : public coclass<Foo, thread_model::Both, IBar>
    \endcode
    (Sofus Mortensen)
\li Aggregating another component is now done by adding class aggregates to the type list of interfaces. First template argument of class aggregates is the coclass
    to aggregating. The 2, 4, ... template arguments specify which interfaces to aggregate. If no interfaces have been specified, all interfaces will be aggregated.
    Example:
    \code
    template<> coclass_implementation<Foo> : public coclass<Foo, thread_model::Both, aggregates<Bar, IBar> >
    \endcode
    (Sofus Mortensen)
\li Added cmp method to bstr_base which a.o.t. can be configured for case insensitive comparision. (Michael Geddes)
\li Added comparison functors based on cmp to bstr_base, less, less_equal, equal_to, etc. Example of usage:
    \code
        std::set<bstr_t, bstr_t::less<cf_ignore_case> > mySet;
    \endcode
    (Sofus Mortensen)
\li Fixed bugs in assignment operators for datetime_t. (Sofus Mortensen, Mikael Lindgren)
\li In ptr.h changed COMET_ALLOW_DECLSPEC_PROPERTY to COMET_USE_RAW_WRAPPERS. (Michael Geddes)
\li In common.h added workaround for VARIANT_TRUE resulting in a level 4 warning. (Michael Geddes)
\li Changed server.h, so that a compiler error will occur if there are unimplemented coclasses. In case you deliberately want
    unimplemented coclass define COMET_ALLOW_UNIMPLEMENTED_COCLASSES in std.h. (Sofus Mortensen)
\li Added various helper functions to util.h. (Sofus Mortensen)
\li Added support for aggregating the free threaded marshaler. Example:
    \code
    template<> coclass_implementation<Foo> : public coclass<Foo, thread_model::Both, FTM>
    \endcode
    (Sofus Mortensen)
\li Various bug fixes and changes. (Sofus Mortensen)

\par Version 1 beta 9 (2001-9-23)

\li Now wrapper properties and methods are only available through com_ptr::operator->. (Sofus Mortensen)
\li Added ostream<> outputting facility to both variant_t and bstr_t. (Sofus Mortensen)
\li Added std::string conversions to variant_t. (Sofus Mortensen)
\li Fixed various bugs in tlb2h regarding dispinterfaces. (Sofus Mortensen).
\li Fixed bug in com_ptr - try_cast'ing from variant_t to com_ptr did not throw an exception on error. (Sofus Mortensen)
\li Made com_ptr constructor from variant_t and assignment from variant_t private in com_ptr to prevent misuse. Use com_cast or try_cast. (Sofus Mortensen)

\par Version 1 beta 8 (2001-9-19)

\li Fixed bugs in tlb2h regarding datetime_t. (Sofus Mortensen)
\li Wrapper properties are now only available when accessing an interface through com_ptr::operator->. (Sofus Mortensen)
\li Classes currency_t and datetime_t are now exception safe. (Sofus Mortensen)
\li Added conversions for variant_t from/to currency_t and datetime_t. (Sofus Mortensen)
\li Added conversions for std::wstring to variant_t, solving various ambiguity problems. (Sofus Mortensen)
\li Re-arranged various header files, solving include problems. (Sofus Mortensen)

\par Version 1 beta 7 (2001-9-7)

\li Added support for locking to safearray_t. (Michael Geddes)
\li Improved support in tlb2h for methods taking arrays as argument. (Sofus Mortensen)
\li Fixed bug in tlb2h regarding methods that were the parameters were wrongly given a default value. (Sofus Mortensen)
\li Fixed bug in tlb2h regarding type "void *". (Sofus Mortensen)
\li Fixed various bugs in datetime.h. (Michael Geddes)
\li Added COMET_ASSERT - a replacement for _ASSERTE. (Sofus Mortensen)

\par Version 1 beta 6 take 2 (2001-8-20)

\li Fixed dumb, dumb _ASSERTE bug in tlb2h (Sofus Mortensen).

\par Version 1 beta 6 (2001-8-19)

\li Changed distribution package - now including scripts for generating docs and the source for both tlb2h and the Comet App. wizard (Sofus Mortensen).
\li Wrapped string constant in server.h with _T( ) for UNICODE compatibility (Sofus Mortensen).
\li Tlb2h.exe now generates vtable fillers to cater for interfaces with gaps in the vtable. Such are typically produced by VB (Michael Geddes and Sofus Mortensen).
\li Bug fixes to date_t (Michael Geddes).
\li Elimination of compiler warnings in safearray_t and registry.h (Michael Geddes).

\par Version 1 beta 5 (2001-8-14)

\li tlb2h.exe now generates corrects defaults for [optional] VARIANT. variant_t has been updated in order to support this (Sofus Mortensen).
\li Fixed problem in tlb2h.exe with using COMET_ALLOW_DECLSPEC_PROPERTY when importing msado20.tlb (Sofus Mortensen).
\li Fixed problem in tlb2h.exe with aliases being treated as records (Michael Geddes).
\li Several bug fixes to nutshell generation (Michael Geddes).
\li Changed tlb2h, so that the generated headers only #include the headers needed (Michael Geddes).

\par Version 1 beta 4 (2001-7-25)

\li datetime_t has been updated and support for datetime_t has been added to tlb2h.exe (Michael Geddes).
\li typelist::index_of bug fix (Michael Geddes, Sofus Mortensen, thanks to Eric Friedman for reporting this).
\li typelist::type_at bug fix (Michael Geddes).
\li create_reference added to safearray_t (Michael Geddes).
\li Experimental attach_from / attach_to added to safearray_t (Michael Geddes).
\li Bug fix in variant.h (Sofus Mortensen).

\par Version 1 beta 3 (2001-7-14)

\li MSVC7 beta 2 compatibility fixes (Sofus Mortensen).
\li Various bug fixes in tlb2h (Sofus Mortensen).
\li Added wizard option to tlb2h for generating skeleton implementation of coclasses (Michael Geddes).
\li Various bug fixes to safearray.h (Michael Geddes).
\li variant_t now supports converion to/from saferray_t (Michael Geddes, Sofus Mortensen)
\li com_ptr can now be used to wrap a pointer to coclass_implementation (Michael Geddes).
\li enum.h and cp.h has been updated to support the changes to com_ptr (Michael Geddes).
\li Preliminary version of datetime_t has been added (Michael Geddes).

\par Version 1 beta 2 (2001-7-04)

\li Major documentation update (Michael Geddes).
\li Bug fix in tlb2h.cpp (Michael Geddes).
\li Replaced operator bool in com_ptr with a more bullet proof alternative (Sofus Mortensen).
\li Updated functor.h to make use of partial specialisation for compilers that support it (Michael Geddes).

\par Version 1 beta 1 (2001-6-29)

\li Added currency support to variant_t (Michael Geddes).
\li Documentation update for server.h (Michael Geddes).
\li Various bug fixes to do with agg-objects (Michael Geddes).
\li Added GetClassObject implementation to atl_module (Michael Geddes).


\par Version 1 alpha 10 (2001-6-26):

\li CURRENCY wrapper updated by Michael Geddes.
\li tlb2h has been updated by Michael Geddes (again!). This time with options to specify which namespace to put wrappers in, and an options to emit symbols only for faster compilation.
\li atl_module.h documentation updated by Michael Geddes.

\par Version 1 alpha 9 (2001-6-21):

\li CURRENCY wrapper added by Michael Geddes.
\li Nutshell wrappers didn't work in alpha 8. Now fixed.
\li tlb2h has been updated by Michael Geddes for better command line handling. It now supports multiple files including wildcards, and support for specifying an output directory.
\li Server implementation updated by Michael Geddes to support loading type libraries that has not yet been registered.
\li try_cast no longer throws on null pointers, and comet::com_ptr::operator->() now throws on null pointers. (Michael Geddes)

\par Version 1 alpha 8 (2001-6-10):

\li Compatibility patches for GCC/MINGW submitted by Michael Geddes.
\li Updated com_error with methods for accessing wrapped HRESULT value and IErrorInfo (source, helpfile, etc).
\li Various minor bug fixes.

\par Version 1 alpha 7 take 3 (2001-3-31):

\li Added converters for CURRENCY to variant_t..
\li Updated functor.h.

\par Version 1 alpha 7 take 2 (2001-3-28):

\li Fixed bug concerning using com_cast and try_cast with raw interface pointers.
\li Fixed bug in bstr.h
\li Fixed dependency on comet/stl.h in comet/enum.h


\par Version 1 alpha 7 take 1 (2001-3-26):

\li Support for dispinterfaces has been added by Michael Geddes and Mikael Lindgren.
\li BCC compatibility patches contributed by Michael Geddes.
\li Support for multiple connection points added by Michael Geddes.
\li Added generalised functor library that works with MSVC (Not really COM related)
\li comet::make_list\<\> has changed behaviour. Instead of writing make_list\<IFoo, IBar\>, you have to write \link comet::make_list make_list\<IFoo,IBar\>::result \endlink. This has been done in order to shorten error messages relating typelists.
\li Added several helper classes to comet/typelist.h for manipulation of typelists.
\li Fixed various "ambiguous conversion" errors


\par Version 1 alpha 6 (2001-3-7):

\li Fixed disastrous bug in comet::com_ptr.
\li Re-organisation of header files.
\li Support for Intel C++ 5.0 added.
\li Fixed bug in regkey.
\li Various bugs fixes.


\par Version 1 alpha 5 take 2 (2001-1-26):

\li Fixed bug in ptr.h


\par Version 1 alpha 5 (2001-1-16):

\li Support for implementing aggregateable components. (Michael Geddes)
\li comet::com_ptr is no longer using common base class.
\li Various bug fixes.


\par Version 1 alpha 4.2 (2000-12-17):

\li Fixed bug in critical_section.


\par Version 1 alpha 4.1 (2000-12-14):

\li Fixed bug in variant.h
\li Added support for DATE and CURRENCY to tlb2h. (We might need wrapper classes for those).
\li Fixed missing inline in common.h.


\par Version 1 alpha 4 (2000-12-12):

\li Generated wrappers, interfaces, etc are now placed in namespace corresponding to typelibrary name by tlb2h.
\li Fixed bugs in variant.h
\li The generated wrappers are no longer using .get_raw() and .get_raw_ptr(). Instead the wrapper methods .in(), .in_ptr(), .inout() and .out() are being used.
\li Support for constants placed in type library modules. (See Joav Kohn's posting on ATL discuss).
\li Support for version numbering for ProgID's. (Thanks to Mikael Lindgren).
\li Updated the wizard.


\par Version 1 alpha 3 (2000-11-29):

\li Added sanity checking to safearray_t to capture type mismatches.
\li Added in() method to all wrapper classes for use when calling raw interfaces.
\li The generated header files are now using a normal include guard instead of #pragma once.
\li Various minor fixes.


\par Version 1 alpha 2 (2000-11-20):

\li Nutshell generation added to tlb2h.exe.
\li Added STL compatible SAFEARRAY wrapper called comet::safearray_t<T>.
\li Updated registry class.
\li New BSTR wrapper, comet::bstr<boolean> with optional reference counting.
\li comet::bstr_t is now a typedef of comet::bstr<false>.
\li ATL support added.


\par Version 1 alpha 1 (2000-10-29):

\li First alpha release of Comet.


\par Version 0.9.7 (2000-10-19):

\li typedef bug fix in tlb2h.exe
\li Bug fixes in comet::variant_t


\par Version 0.9.6 (2000-10-19):

\li Documentation updates.
\li Misc. bug fixes.


\par Version 0.9.5 (2000-10-5):

\li Documentation updates.
\li Paul Hollingsworth has been busy minimising include dependencies in the header files.
\li Added [out] and [in, out] adapters for dealing with raw interfaces.


\par Version 0.9.4 (2000-10-4):

\li Fixed more bugs in comet::com_ptr.
\li Added documentation on comet::com_ptr.


\par Version 0.9.3 (2000-9-26):

\li Fixed various bugs in comet::com_ptr.


\par Version 0.9.2 (2000-9-25):

\li Updated tlb2h to use const where appropiate.
\li Added support for unions.
\li Added support for one-dimensional arrays in structs.


\par Version 0.9.1 (2000-9-21):

\li Experimental support in tlb2h.exe for optional method arguments (ie. attribue optional or defaultvalue).


\par Version 0.9.0 (2000-9-19):

\li Paul Hollingsworth has contributed automatic implementation of IProvideClassInfo.
\li Better support for [in, out] parameters.
\li Fixed bugs in tlb2h's generation of connection point wrapper.
\li Removed progid from template parameters, instead override function get_progid().
\li Added version metadata to coclass and typelibrary wrappers.
\li Fixed bug in variant_t::variant_t(const comet::bstr_t&).


\par Version 0.8.3 (2000-9-12):

\li Removed DLL dependency for tlb2h.exe.


\par Version 0.8.2 (2000-9-7):

\li com_error / raise_exception_t has been revised.
\li Fixed bug in com_ptr::operator=(int)
\li Rearranged header inclusion introducing error_fwd.h.
\li Added misc. utility classes not necessarily related to tlb2h to tlb2h/utility.h.
\li Updated tlb2h.exe with more types.


\par Version 0.8.1 (2000-9-3):

\li Type conversions to/from bstr_t has been changed/tweaked.
\li bstr_t now supports concatenation through operator+ and operator+=.


\par Version 0.8.0 (2000-8-31):

\li Fixed several bugs in tlb2h.exe.
\li Added misc. utility classes not necessarily related to tlb2h to tlb2h/utility.h.


\par Version 0.7.10 (2000-8-25):

\li Updated AppWizard to generate uuid for type library.
\li Added trait class to com_server for servers with no embedded type library.


\par Version 0.7.9 (2000-8-21):

\li bstr_t is now capable of converting to std::string as well.
\li Rearranged util.h into list.h, common.h and util.h.


\par Version 0.7.8 (2000-8-18):

\li Added simple AppWizard for creating DLL projects.
\li Removed untie. Instead embedded_object now has a release_from_owner method.
\li structs defined in type libraries now automatically use CoTaskMem when using new/delete.


\par Version 0.7.7 (2000-8-15):

\li Changes to lifetime of embedded_object. Introducing tlb2h::untie, breaks tie between owner and embedded_object, so that the embedded_object can be destroyed before the owner.


\par Version 0.7.6 (2000-8-15):

\li Various changes to comet::bstr_t.


\par Version 0.7.5 (2000-8-13):

\li Interface inheritance problems solved.
\li Problem with [in] VARIANT arguments solved.
\li create_enum now works with embedded_objects.
\li comet::make_list now handles up to 40 elements.
\li Problem with IUnknown interfaces in coclasses solved.
\li Workaround for bug in Visual Studio editor causing VS to crash when editing certain template code.


\par Version 0.7.4 (2000-8-8):

\li Conversion from comet::com_ptr to comet::variant_t improved.
\li Updated example to demonstrate now working COM enumeration implementation on STL containers.


\par Version 0.7.3 (2000-8-8):

\li Nasty connection point bug fixed.
\li Added trivial VB client example program demonstrating connection point.
\li Added com_cast and try_cast for casting interface pointers. Implicit casting no longer allowed.
\li comet::com_error::what() is now working.
\li Misc. minor bug fixes/tweaks.


\par Version 0.7.2 (2000-8-3):

\li Updated the example with registration custom build.
\li Misc. bug fixes.

*/
//! Primary namespace for comet. Includes source and libraries.
namespace comet {
//! Namespace for comet implementation details.
/*! \internal
 */
namespace impl {
}
}
/** \defgroup Misc Miscelaneous utility classes.
 *@{
 */
//@}
/** \defgroup COMType Com type wrappers.
 *@{
 */
//@}
/*!\defgroup WinUtil Windows utility classes.
 *@{
 */
//@}


#endif
