# clrzmq &mdash; Official 0MQ Bindings for .NET and Mono
*(Formerly clrzmq2; legacy bindings have moved to [clrzmq-old][clrzmq-old])*

This project aims to provide the full functionality of the underlying ZeroMQ API to CLR projects.

Bundled libzmq version: **3.2.2-rc2**  
Legacy libzmq version supported: **2.2.0 (stable)**

## Getting Started

The quickest way to get started with clrzmq is by using the [NuGet package][clrzmq-nuget]. The NuGet packages include a copy of the native libzmq.dll, which is required to use clrzmq.

You may also build clrzmq directly from the source. See the Development Environment Setup instructions below for more detail.

To get an idea of how to use clrzmq, have a look at the following example.

### Example server

```c#
using System;
using System.Text;
using System.Threading;
using ZMQ;

namespace ZMQGuide
{
    class Program
    {
        static void Main(string[] args)
        {
            // ZMQ Context, server socket
            using (ZmqContext context = ZmqContext.Create())
            using (ZmqSocket server = context.CreateSocket(SocketType.REP))
            {
                server.Bind("tcp://*:5555");
                
                while (true)
                {
                    // Wait for next request from client
                    string message = server.Receive(Encoding.Unicode);
                    Console.WriteLine("Received request: {0}", message);

                    // Do Some 'work'
                    Thread.Sleep(1000);

                    // Send reply back to client
                    server.Send("World", Encoding.Unicode);
                }
            }
        }
    }
}
```

### Example client

```c#
using System;
using System.Text;
using ZMQ;

namespace ZMQGuide
{
    class Program
    {
        static void Main(string[] args)
        {
            // ZMQ Context and client socket
            using (ZmqContext context = ZmqContext.Create())
            using (ZmqSocket client = context.CreateSocket(SocketType.REQ))
            {
                client.Connect("tcp://localhost:5555");

                string request = "Hello";
                for (int requestNum = 0; requestNum < 10; requestNum++)
                {
                    Console.WriteLine("Sending request {0}...", requestNum);
                    client.Send(request, Encoding.Unicode);

                    string reply = client.Receive(Encoding.Unicode);
                    Console.WriteLine("Received reply {0}: {1}", requestNum, reply);
                }
            }
        }
    }
}
```

## Docs and Tutorials

For more information about 0MQ and clrzmq, see the following resources:

- The [0MQ Guide][zmq-guide] is a thorough overview of 0MQ and contains example code for most supported languages, including C#. Examples are stored in a [GitHub repository][zmq-example-repo].
- [ZeroMQ via C#: Introduction][zmq-cs-intro] is an excellent tutorial written by Manar Ezzadeen and offers an in-depth look at various 0MQ patterns implemented in C# with clrzmq.

More tutorials and API documentation are on the way.

## Deployment Notes

When using a packaged (NuGet/zip) release on Windows, clrzmq comes bundled with 32- and 64-bit versions of libzmq.dll that have been fully tested with the binding package.
The DLLs are embedded as manifest resources in the assembly and are extracted when initially loading the assembly.
To accommodate different deployment needs, several output paths may be used.

The libzmq.dll search/extract order is as follows:

1. libzmq.dll on System Path
   - Allows loading a custom libzmq.dll into system32, %PATH%, etc. (see [LoadLibrary][load-library] for search order)
2. Extract to assembly path
   - Full path to the currently executing clrzmq.dll
3. Extract to execution path
   - Full path to the executable calling clrzmq
4. Extract to temporary path
   - Current user's `%TEMP%\clrzmq-x.x.x.x`
   - Last ditch fallback: only severly restricted accounts will fail to extract to this path

### Tracing

If you run into problems related to loading the native library, you can enable tracing in your application as follows:

1. Add a trace listener to your application (see [TraceListener](http://msdn.microsoft.com/en-us/library/system.diagnostics.tracelistener.aspx) for an example)
2. Configure the `clrzmq` trace switch to use the `Info` level (3):

```xml
<configuration>
  <system.diagnostics>
    <switches>
      <add name="clrzmq" value="3" /><!-- Info -->
    </switches>
  </system.diagnostics>
</configuration>
```

### IIS Deployments

IIS deployments can be complicated due to the many possible identities and privilege levels that may be executing your application.
There are a few steps you can take to ensure the bundled native libraries can be properly extracted and used:

1. At the bare minimum, ensure the Application Pool identity has write access to its `%TEMP%` folder
2. Allow write access to your application's `bin` directory (i.e., the directory containing clrzmq.dll and your application assemblies)
3. If none of those options are suitable, you will need to obtain a compiled version of libzmq.dll for your platform and extract it to a system path (see [LoadLibrary][load-library] for candidates)

## Development Environment

On Windows/.NET, clrzmq is developed with Visual Studio 2010. Mono development is done with MonoDevelop 2.8+.

### Windows/.NET

clrzmq depends on `libzmq.dll`, which will be retrieved automatically via NuGet. If you require a specific version of libzmq, you can compile it from the [0MQ sources][libzmq].

#### clrzmq

1. Clone the source.
2. Run `build.cmd` to build the project and run the test suite.
3. The resulting binaries will be available in `/build`.

#### Alternate libzmq (optional)

If you want to use a custom build of `libzmq.dll`, perform the following steps:

1. Delete or rename the `src/.nuget/packages.config` file. This prevent the NuGet package from being retrieved.
2. Remove any folders matching `src/packages/libzmq-*` that may have been downloaded previously.
3. Copy the 32-bit and 64-bit (if applicable) build of `libzmq.dll` to `lib/x86` and `lib/x64`, respectively.

Note that PGM-related tests will fail if a non-PGM build of libzmq is used.

### Mono

**NOTE**: Mono 2.10.7+ is required **for development only**, as the NuGet scripts and executables require this version to be present.
If you choose to install dependencies manually, you may use any version of Mono 2.6+.

#### Mono 2.10.7+ configuration

NuGet relies on several certificates to be registered with Mono. The following is an example terminal session (on Ubuntu) for setting this up correctly.
This assumes you have already installed Mono 2.10.7 or higher.

```shell
$ mozroots --import --sync

$ certmgr -ssl https://go.microsoft.com
$ certmgr -ssl https://nugetgallery.blob.core.windows.net
$ certmgr -ssl https://nuget.org
```

This should result in a working Mono setup for use with NuGet.

#### libzmq

Either clone the [ZeroMQ repository][libzmq] or [download the sources][zmq-dl], and then follow the build/install instructions for your platform.
Use the `--with-pgm` option if possible.

#### clrzmq

1. Clone the source.
2. Run `nuget.sh`, which downloads any dependent packages (e.g., NUnitLite for acceptance tests).
3. Run `make` to build the project and run the test suite.
4. The resulting binaries will be available in `/build`.

**NOTE**: `clrzmq` only supports x86 builds on Mono at this time

## Issues

Issues should be logged on the [GitHub issue tracker][issues] for this project.

When reporting issues, please include the following information if possible:

* Version of clrzmq and/or how it was obtained (compiled from source, NuGet package)
* Version of libzmq being used
* Runtime environment (.NET/Mono and associated version)
* Operating system and platform (Win7/64-bit, Linux/32-bit)
* Code snippet demonstrating the failure

## Contributing

Pull requests and patches are always appreciated! To speed up the merge process, please follow the guidelines below when making a pull request:

* Create a new branch in your fork for the changes you intend to make. Working directly in master can often lead to unintended additions to the pull request later on.
* When appropriate, add to the AcceptanceTests project to cover any new functionality or defect fixes.
* Ensure all previous tests continue to pass (with exceptions for PGM tests)
* Follow the code style used in the rest of the project. ReSharper and StyleCop configurations have been included in the source tree.

Pull requests will still be accepted if some of these guidelines are not followed: changes will just take longer to merge, as the missing pieces will need to be filled in.

## License

This project is released under the [LGPL][lgpl] license, as is the native libzmq library. See LICENSE for more details as well as the [0MQ Licensing][zmq-license] page.

[clrzmq-old]: https://github.com/zeromq/clrzmq-old
[clrzmq-nuget]: http://packages.nuget.org/Packages/clrzmq
[libzmq]: https://github.com/zeromq/libzmq
[load-library]: http://msdn.microsoft.com/en-us/library/windows/desktop/ms684175(v=vs.85).aspx
[zmq-guide]: http://zguide.zeromq.org/page:all
[zmq-example-repo]: https://github.com/imatix/zguide/tree/master/examples/C%23
[zmq-cs-intro]: http://www.codeproject.com/Articles/488207/ZeroMQ-via-Csharp-Introduction
[zmq-dl]: http://www.zeromq.org/intro:get-the-software
[zmq-license]: http://www.zeromq.org/area:licensing
[issues]: https://github.com/zeromq/clrzmq/issues
[lgpl]: http://www.gnu.org/licenses/lgpl.html
