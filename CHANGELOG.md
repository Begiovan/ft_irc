# 09/11

## Step 6 stabilization pass

- Refactored server socket state to align with poll-driven flow:
    - `Server::_serverSocket` changed from `int` to `pollfd`
    - `Server::_fds` renamed to `Server::_sockets`
    - Updated all call sites to use `.fd` where raw file descriptors are required
- Consolidated disconnection flow around mark-and-sweep:
    - kept `markForDisconnect()` as the single entry point for deferred disconnects
    - sweep now clears `_markedForDisconnect` after processing
- Hardened poll loop behavior:
    - added explicit `poll()` return-value check
    - added handling branches for `POLLHUP | POLLERR | POLLNVAL` events
- Updated signal policy in `main`:
    - `SIGINT`/`SIGTERM` keep graceful shutdown path
    - `SIGPIPE` is now ignored via dedicated `sigaction(SIGPIPE, SIG_IGN)` setup
- Enforced IRC input line-size limit in `receiveClient()`:
    - max 512 bytes for complete lines
    - disconnect on oversized partial buffered input to prevent unbounded memory growth
- Updated Step 6 documentation:
    - created and iteratively aligned `STEP6.md` with real code status and line references
    - tracked closed items (high priority + problem 4) and remaining refinement points

# 04/11

## Refactor validation helpers

- Added `ACommand` validation utilities to centralize common checks and reply generation:
  - `hasEnoughParams_OrReply()`
  - `resolveChannel_OrReply()`
  - `findMember_OrReply()`
  - `findOperator_OrReply()`
  - `resolveNick_OrReply()`
- Updated command constructors to pass the command name, required parameter count and auth requirement to `ACommand`
- Reworked command execution paths to replace repetitive validation blocks with the new `_OrReply()` helpers
- Added `src/ACommand.cpp` to the project build so the shared helper implementations are linked into the binary

## API normalization pass

- Normalized command handlers and object APIs to reference-based parameters where the target is guaranteed to exist
- Kept pointer usage only for nullable lookups and optional values, while replacing repeated pointer dereference patterns with direct reference access
- Updated channel and client membership checks to use consistent reference semantics without changing the underlying ownership model
- Verified the project builds cleanly after the refactor and normalization pass

# 03/11

## Refactor workflow

- Started Step 1 of the refactor under review-by-steps mode
- The project now follows a strict checkpoint flow: one step at a time, with explicit approval before advancing to the next one
- This keeps the refactor safe and makes each change verifiable against the previous baseline

## Step 1 — output path and server identity

- Centralized CRLF handling in `Server::sendToClient` so every reply is terminated exactly once
- Added a single server identity constant via `SERVER_NAME`
- Added `Client::nickOrStar()` and `Client::prefix()` to keep source and prefix formatting consistent
- Kept IRC reply macros as pure payload builders without trailing line termination

# 19/10

## Channels

- Now channels names must start with # or &

## NickName

- Adjusted rules to fit IRC specs
    - Nicknames are now case-insensitive
    - Set special case rules
        - `{` uppercase of `[`
        - `}` uppercase of `]`
        - `|` uppercase of `\`
    - other adjustments to fit the BNF spec

## Commands
- Modified USER and NICK commands
    - Now they will fail with '464 ERR_PASSWDMISMATCH' if no PASS command was given before
        - From [RFC 2812, section 3.1](https://www.rfc-editor.org/info/rfc2812/#section-3.1): A "PASS" command [..] MUST precede the latter of the NICK/USER combination

# 15/10

## Server class
 - Refactored receiveClient() method
     - Readbility changes
     - Moved index parameter in fd parameter
     - Moved iterators to client's map in a Client pointer since only that was used
 - Added explicative comments and function descriptions

# 7/10

## Style
 - Formatted includes with tab in 42 style

## Server class
 - Moved some methods from public to private when public access specifier was useless
	- void	setupSocket(int port);
	- void	flushClient(int fd);
 - Added some minimal comments
