# ServerAPI

## Usage

The Server API can be run manually like any of the other server components, or you can install it as a Windows service with `ServerAPI -install`. `ServerAPI -remove` will remove the service. Note that the service by default installs itself to run as `LocalService`, which has a fairly low privilege level, so make sure to put the executable somewhere that `LocalService` can read it.

## Configuration

`ServerAPI.cfg` must be present in the same directory as the executable. The default TCP port for the webserver is `8913`; this can be changed with the optional `Port <port>` directive in the config.

The remainder of the configuration file must consist of lines of the form `Shard <name> <ip>`, e.g.:

```
Shard Homecoming 127.0.0.1
```

## API

Currently implemented API URLs:

* `/<shardname>/status` - basic server up/down status (this is also included in all other results as well)
* `/<shardname>/dbserver` - dbserver totals and player stats
* `/<shardname>/launchers` - list of launchers and their individual stats
* `/<shardname>/maps` - list of mapserver processes and their stats
* `/<shardname>/allstats` - all of the above

You can use `all` as the shard name to get information for all known shards.

## Building

The code provided is not complete. You will need to do work to integrate it into an existing I25 source tree.

### Directories and Projects

Two directories are included: `ServerCmd` and `ServerAPI`.
One project is included: `ServerAPI`.

### Dependencies

You will need `libmicrohttpd`, which can be found at <https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.63.tar.gz>.

You will need to add `libmicrohttpd` under the top-level `Libs` project in your Master Solution. This may require porting `libmicrohttpd` to your version of Visual Studio.

### Code Changes

The `ServerCmd/serverMonitorNet.c` file cannot be provided. To replace it:

1. Copy `ServerMonitor/serverMonitorNet.c` to `ServerCmd`.
2. Remove references to:
    * `destructor_shc`
    * `processMonitor.h`
    * `psapi`
    * `serverMonitorCrashMsg.h`
    * `serverMonitorEnts.h`
    * `shardMonitor.h`
3. Remove the following functions:
    * `launchRemoteDesktop()`
    * `startServersIfNotRunning()`
    * `svrMonLogHistory()`
    * `updateBadVersionColors()`
    * `updateMapItemColors()`
    * all `smc*` functions for remote debug/desktop
4. Remove `DELETE_LISTVIEW` from `svrMonClearAllLists()`.
5. Remove `allow_autostart` and the entire `if`/`else` statement referring to it from `svrMonConnect()`.
6. Remove the two `ListView` arguments and the `logName` argument from `handleRecvList()`. Remove references to those arguments throughout the function.
7. Add `state->last_received = timerSecondsSince2000();` at the top of `handleRecvList()`.
8. Comment out the two `if` statements (and any `else` clauses) where `servers_in_trouble_changed` is modified.
9. Add the `serverCmd.h` header.
10. Add the following function(s):

```
int svrMonAlive(ServerMonitorState *state)
{
       if (!svrMonConnected(state))
               return 0;
       return (timerSecondsSince2000() - state->last_received) < 30;
}
```

You will also need to copy and slightly modify `ServerMonitor/serverMonitorNet.h`.

.
