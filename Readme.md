**Project: Ouroboros**
======================

![Build Status](https://dev.azure.com/OuroDev/Source/_apis/build/status/Volume%202%20Source?branchName=develop)

Prerequisite Software
=====================

Building requires [Visual Studio 2019](https://visualstudio.microsoft.com/vs/). The Community edition will work fine.

If you just to run a server, you will also need [SQL Server 2017](https://www.microsoft.com/en-us/sql-server/sql-server-2017). Any version at least >= SQL Server 2008 will work fine.  
[SQL Server Management Studio](https://docs.microsoft.com/en-us/sql/ssms/download-sql-server-management-studio-ssms?view=sql-server-2017) should be installed as well to aid in setting up the database.

Download the necessary data archives from [links posted on the wiki](https://wiki.ourodev.com/Magnet_Links). Specifically "Volume 2 Issue 1.1 Server Data" and "Volume 2 Issue 1.1 Server Piggs".

Development Guide
=================
Coding Convention
-----------------

* **DO** use `const` and `static` and visibility modifiers to scope exposure of
   variables and methods as much as possible.

* **DON'T** use global variables where possible.

Style Guide
-----------

### Automated Formatting with `clang-format`

For all C/C++ files (`*.c`, `*.cpp`, `*.h`and `*.hpp`), we use `clang-format`
to apply our code formatting rules to **NEW** files. After adding new C/C++ files and
before merging, be sure to run `clang-format` by invoking it from the IDE of your choice.
In case of Visual Studio 2019 formatting is performed by _`File->Advanced->Format Document`_
or hitting _`Ctrl+K`_, _`Ctrl+D`_ (type _`Ctrl+K`_, AND THEN _`Ctrl+D`_ as it is a sequence).

This allows us apply formatting choices such as the use of [Allman style](
http://en.wikipedia.org/wiki/Indent_style#Allman_style) braces and the 160
character column width consistently.

Please stage the formatting changes with your commit, instead of making an extra
"Format Code" commit.

The [.clang-format](./.clang-format) file describes the style that is enforced
by invoking `clang-format`, which is based off the LLVM style with modifications closer to
the default Visual Studio style. See [clang-format style options](
http://releases.llvm.org/8.0.0/tools/clang/docs/ClangFormatStyleOptions.html)
for details.

### Naming Conventions

Naming conventions we use that are not automated include:

1. Don't use Hungarian notation (https://en.wikipedia.org/wiki/Hungarian_notation).
2. Use `camelCase` for variable, member/field, and function names.
3. Use `UPPER_SNAKE_CASE` for macro names and constants.
4. Prefer `lower_snake_case` file names for headers and sources.
5. Prefer full words for names over contractions (i.e. `memoryContext`, not
   `memCtx`).
6. Prefix names with `_` to indicate internal and private fields or methods
   (e.g. `_internal_field, _internal_method()`).
7. The single underscore (`_` ) is reserved for local definitions (static,
   file-scope definitions).
   e.g. static oe_result_t _parse_sgx_report_body(..).
8. Prefix `struct` definitions with `_`, and always create a `typedef` with the
   suffix `_t`:
```c
typedef struct _oe_private_key
{
    uint64_t magic;
    mbedtls_pk_context pk;
} oe_private_key_t;
```
8. Prefix Open Enclave specific names in the global namespace with `oe_` (e.g.
   `oe_result_t, oe_call_enclave`).

Above all, if a file happens to differ in style from these guidelines (e.g.
private members are named `m_member` rather than `_member`), the existing style
in that file takes precedence.

Note that we _no longer_ use `CamelCase` nor double underscores (`__`), but you
may find remnants and so again should prefer the local style. This is especially
the case for classes, which are still using `PascalCase`. For now, follow the
existing style. The project maintainers prefer to fix style issues in bulk using
automation, so avoid submitting PRs intended to fix only a few instances of the
inconsistent style.

For other files (`*.asm`, `*.S`, etc.) our current best guidance is consistency:

- When editing files, keep new code and changes consistent with the style in the
  files.
- For new files, it should conform to the style for that component.
- For new components, any style that is broadly accepted is fine.

### Example File


Building Servers and Client
=========================== 

## Downloading a Pre-built Archive

Build pipelines are triggered upon every commit to `master` and `develop`. You can find their output at [https://build.ourodev.com](build.ourodev.com).  
Skip to #2 if you use this option.

## 1) Building from Source

- Open `build/vs2019/master.sln`. Build against your chosen target platform and configuration.

## 2) Setting up SQL Server

- Enable TCP/IP in SQL Configuration Manager.
- Restart the SQL Server service.
- Create the necessary databases in SSMS using the schemas from `Assets/DBSchemas/`.
- If using an AuthServer, set up a File DSN with permissions to `cohauth`.

## 3) Running the Servers

Navigate to the `bin/` directory.
- Copy `Assets/ConfigFiles/*` here.
- Extract `data-v2i1.1.7z` and `piggs-v2i1.1.7z` here.
- Edit all server configuration files in `data/server/db/`. You should update any SqlLogin entries and define any server-specific variables you wish to use
- Run `MapServer.exe -productionmode -templates`
- If using an AuthServer, run `AuthServer.exe`
- Run `DBServer.exe -startall`.
- Run `Launcher.exe`.

Once all the services are up you should be able to connect using `Ouroboros.exe`. Keep an eye on things using ServerMonitor.

And most importantly, defer to [the wiki](https://wiki.ourodev.com) if you need in-depth instructions :)

<a href="https://wiki.ourodev.com/OuroDev_Discord"><img style="vertical-align:middle" src="https://discordapp.com/assets/fc0b01fe10a0b8c602fb0106d8189d9b.png" alt="Discord" height="30px" /></a>
