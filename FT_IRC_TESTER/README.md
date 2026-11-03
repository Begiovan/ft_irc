# ft_irc Tester

Terminal tester for the 42 `ft_irc` project.

## Setup

Copy your compiled server binary into this tester directory and make sure it is named `ircserv`:

```sh
cp /path/to/your/ft_irc/ircserv ./ircserv
```

The tester expects this by default:

- binary: `./ircserv`
- host: `127.0.0.1`
- port: `6667`
- password: `mypass`

## Run

Start the tester:

```sh
make
```

This shows the ASCII intro and opens the terminal menu:

```text
1  normal    mandatory ft_irc checks
2  bonus     DCC relay and optional bot hook
3  global    normal + bonus + stress/regression checks
4  terminal  manual IRC command terminal
5  quit
```

When you choose a test mode, the tester starts:

```sh
./ircserv 6667 mypass
```

Then it runs the selected suite.

## Other Modes

Mandatory tests only:

```sh
make normal
```

Bonus tests only:

```sh
make bonus
```

Global tests:

```sh
make global
```

Manual IRC terminal:

```sh
make terminal
```

Inside the terminal, you can type raw IRC commands:

```irc
JOIN #test
PRIVMSG #test :hello
MODE #test +i
TOPIC #test :new topic
```

Use `/quit` to leave the tester terminal.

## Change Port Or Password

```sh
make PORT=4242 PASSWORD=secret
```

Use another binary path:

```sh
make BINARY=/path/to/ircserv
```

Test a server that is already running:

```sh
make external PORT=4242 PASSWORD=secret
```

If your bonus includes a bot:

```sh
make bonus BOT_NICK=mybot
```


Created and owned by Black Wolf.
