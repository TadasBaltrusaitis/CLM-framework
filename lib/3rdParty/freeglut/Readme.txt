freeglut 2.6.0-3.mp for MSVC

This package contains a freeglut import library, headers, and a Windows DLL,
allowing GLUT applications to be compiled on Windows using Microsoft Visual C++.

For more information on freeglut, visit http://freeglut.sourceforge.net/.


Installation

Create a folder on your PC which is readable by all users, for example
“C:\Program Files\Common Files\MSVC\freeglut\” on a typical Windows system. Copy
the “lib\” and “include\” folders from this zip archive to that location.

The freeglut DLL should either be placed in the same folder as your application,
or can be installed in a system-wide folder which appears in your %PATH%
environment variable. On a 32 bit Windows system this is typically
“C:\Windows\System32\”, and on a 64 bit Windows system this is typically
“C:\Windows\SysWOW64\”.


Compiling Applications

To create a freeglut application, create a new Win32 C++ project in MSVC. From
the “Win32 Application Wizard”, choose a “Windows application”, check the
“Empty project” box, and submit.

You’ll now need to configure the compiler and linker settings. Open up the
project properties, and select “All Configurations” (this is necessary to ensure
our changes are applied for both debug and release builds). Open up the
“general” section under “C/C++”, and configure the “include\” folder you created
above as an “Additional Include Directory”. If you have more than one GLUT
package which contains a “glut.h” file, it’s important to ensure that the
freeglut include folder appears above all other GLUT include folders.

Now open up the “general” section under “Linker”, and configure the “lib\”
folder you created above as an “Additional Library Directory”. A freeglut
application depends on the import libraries “freeglut.lib” and “opengl32.lib”,
which can be configured under the “Input” section, however it shouldn’t be
necessary to explicitly state these dependencies, since the freeglut headers
handle this for you. Now open the “Advanced” section, and enter “mainCRTStartup”
as the “Entry Point” for your application. This is necessary because GLUT
applications use “main” as the application entry point, not “WinMain”—without it
you’ll get an undefined reference when you try to link your application.

That’s all of your project properties configured, so you can now add source
files to your project and build the application. If you want your application to
be compatible with GLUT, you should “#include <GL/glut.h>”. If you want to use
freeglut specific extensions, you should “#include <GL/freeglut.h>” instead.

Don’t forget to either include the freeglut DLL when distributing applications,
or provide your users with some method of obtaining it if they don’t already
have it!


Problems?

If you have problems using these packages (runtime errors etc.), please contact
me via http://www.transmissionzero.co.uk/contact/, providing as much detail as
you can. Please don’t complain to the freeglut guys unless you’re sure it’s a
freeglut bug, and have reproduced the issue after compiling freeglut from the
latest SVN version—if that’s still the case, I’m sure they would appreciate a
bug report or a patch.


Changelog

2010–01–22: Release 2.6.0-3.mp

  • Rebuilt the DLL with a minimum OS version of 4.00, so it can work under
    Windows NT 4 and Windows 98. Previously it required at least Windows 2000.

2009-12-22: Release 2.6.0-2.mp

  • Updated documentation to take into account the fact that 32 bit DLLs should
    be placed in the “SysWOW64” folder on 64 bit Windows versions, rather than
    “System32”.
  • Some parts of the documentation rewritten to (hopefully) be easier to
    follow.
  • Updated the “freeglut_std.h” file to stay aligned with my MinGW package.
    There were some MinGW cross-compilation issues under Linux related with the
    fact that the #include of “Windows.h” didn’t match the case of the header
    file “windows.h”.

2009-11-29: Release 2.6.0-1.mp

  • First 2.6.0 MSVC release. I’ve built the package using Visual Studio 2008,
    and the only change I’ve made is to the DLL version resource—I’ve changed
    the description so that my MinGW and MSVC builds are distinguishable from
    each other (and other builds) using Windows Explorer.


Martin Payne
2010–01–22

http://www.transmissionzero.co.uk/
