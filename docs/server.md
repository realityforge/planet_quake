

## Server Features

  * Referee controls. `freeze/unfreeze` for server side pausing. `lock/lockred/lockblue` to lock team joining.
  * Roles based rcon access. Set up to 24 rcon passwords each with a specific role assigned to it. Set the names of the roles with `sv_roles` (default: admin referee moderator). Role names must be alphanumeric with spaces in between, up to 24 names can be specified. Roles are set with `sv_roleName` where Name one of the role names set in `sv_roles`. Role capabilities are set with the name of the console command, e.g. `ban kick map`. Authorized users can access the entire command, that is, if they have `exec` permissions they can execute ANY script. If you want to limit executions to specific commands or combinations an alias has to be used and the alias would be listed in the `sv_roleName` instead of the command.
  * Event streaming. When specific things happen on the server, rcon can query the list of events and get a JSON style response of what happened since the last time it was checked. E.g. `{"type":1,"value":"q3dm1"}` where type `1` is SV_EVENT_MAPCHANGE. This is also used for the discord integration, and rankings. TODO: piped output for "pushing" events (as opposed to polling). TODO: add location information capable of making a heatmap
  * Persistent client sessions, that is, saving and restoring client score/health/location after a full disconnect and reconnect, use `sv_clSession` to specify how many seconds a session is valid to be restored.


## TODOs

  * Event history with demo streaming as a service in the browser for splicing all those sweet frags, SQS/Message Queue
  * Synchronized server/AI for offline and connection interruptions
