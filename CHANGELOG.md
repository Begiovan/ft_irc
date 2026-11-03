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
