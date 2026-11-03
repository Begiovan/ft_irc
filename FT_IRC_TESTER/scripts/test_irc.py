#!/usr/bin/env python3
"""
ft_irc tester.

A clone-friendly terminal tester for 42 ft_irc projects. It can launch a local
ircserv binary or connect to an already running server, then run normal, bonus,
or global test suites. It also includes a tiny raw IRC terminal for manual
commands.
"""

import argparse
import os
import select
import socket
import subprocess
import sys
import time


# ---------------------------------------------------------------------------
# Defaults
# ---------------------------------------------------------------------------

HOST = os.getenv("IRC_TEST_HOST", "127.0.0.1")
PORT = int(os.getenv("IRC_TEST_PORT", "6667"))
PASSWORD = os.getenv("IRC_TEST_PASSWORD", "mypass")
BINARY = os.getenv("IRC_TEST_BINARY", "./ircserv")
TIMEOUT = float(os.getenv("IRC_TEST_TIMEOUT", "4.0"))

PASS = 0
FAIL = 0
SKIP = 0
CURRENT = None


# ---------------------------------------------------------------------------
# Terminal style
# ---------------------------------------------------------------------------

GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
DIM = "\033[2m"
RESET = "\033[0m"
BOLD = "\033[1m"


BLACK_WOLF = r"""
                    /\
                   /  \
              /\  / /\ \  /\
             /  \/ /  \ \/  \
            / /\  /____\  /\ \
           / /  \  .--.  /  \ \
          /_/    \( oo )/    \_\
                  | __ |
               ___\____/___
             .'            `.
            /   BLACK WOLF   \
            \__  FT_IRC TEST _/
               `-._______.-'
"""


def slow_print(text, delay=0.002, enabled=True):
    if not enabled:
        print(text)
        return
    for ch in text:
        print(ch, end="", flush=True)
        if ch != "\n":
            time.sleep(delay)
    print()


def clear_screen():
    if sys.stdout.isatty():
        os.system("clear" if os.name == "posix" else "cls")


def intro(args):
    if args.no_intro:
        return
    clear_screen()
    print(f"{BOLD}{BLUE}{BLACK_WOLF}{RESET}")
    slow_print(
        f"{BOLD}{CYAN}A terminal tester for 42 ft_irc servers.{RESET}",
        enabled=not args.fast,
    )
    if not args.fast:
        time.sleep(0.25)


def rule(color=CYAN):
    print(f"{color}{'-' * 58}{RESET}")


def banner(args):
    rule(MAGENTA)
    print(f"{BOLD}ft_irc tester{RESET}")
    print(f"{DIM}binary{RESET}   {args.binary}")
    print(f"{DIM}server{RESET}   {args.host}:{args.port}")
    print(f"{DIM}password{RESET} {args.password}")
    print(f"{DIM}mode{RESET}     {'external server' if args.no_start else 'launch ./ircserv'}")
    rule(MAGENTA)


def section(title):
    print(f"\n{BOLD}{CYAN}[ {title} ]{RESET}")


def result(name, ok, detail=""):
    global PASS, FAIL
    if ok:
        PASS += 1
        print(f"  {GREEN}[PASS]{RESET} {name}")
    else:
        FAIL += 1
        print(f"  {RED}[FAIL]{RESET} {name}")
        if detail:
            for line in detail.splitlines()[-6:]:
                print(f"         {YELLOW}{line}{RESET}")


def skipped(name, reason):
    global SKIP
    SKIP += 1
    print(f"  {YELLOW}[SKIP]{RESET} {name} {DIM}{reason}{RESET}")


def reset_counters():
    global PASS, FAIL, SKIP
    PASS = 0
    FAIL = 0
    SKIP = 0


# ---------------------------------------------------------------------------
# Config and sockets
# ---------------------------------------------------------------------------

def apply_args(args):
    global HOST, PORT, PASSWORD, BINARY, TIMEOUT, CURRENT
    HOST = args.host
    PORT = args.port
    PASSWORD = args.password
    BINARY = args.binary
    TIMEOUT = args.timeout
    CURRENT = args


def start_server():
    if CURRENT.no_start:
        if not can_connect():
            print(f"{RED}[ERROR]{RESET} Cannot connect to {HOST}:{PORT}.")
            print("Start your server first, or remove --no-start so the tester can launch it.")
            raise SystemExit(1)
        print(f"{GREEN}Connected to external server at {HOST}:{PORT}{RESET}")
        return None

    if not os.path.isfile(BINARY):
        print(f"{RED}[ERROR]{RESET} Binary {BINARY!r} not found.")
        print("Use --binary ./path/to/ircserv or set IRC_TEST_BINARY.")
        raise SystemExit(1)

    proc = subprocess.Popen(
        [BINARY, str(PORT), PASSWORD],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    time.sleep(0.6)
    if proc.poll() is not None:
        out, err = proc.communicate()
        print(f"{RED}[ERROR]{RESET} Server failed to start")
        print("STDOUT:", out.decode(errors="ignore"))
        print("STDERR:", err.decode(errors="ignore"))
        raise SystemExit(1)
    print(f"{GREEN}Server started (pid {proc.pid}){RESET}")
    return proc


def stop_server(proc):
    if proc is None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except subprocess.TimeoutExpired:
        proc.kill()


def can_connect():
    try:
        s = socket.create_connection((HOST, PORT), timeout=1.5)
        s.close()
        return True
    except OSError:
        return False


def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(3)
    s.connect((HOST, PORT))
    s.setblocking(False)
    return s


def send(s, line):
    s.sendall((line.rstrip("\r\n") + "\r\n").encode())


def recv_until(s, needle, timeout=None):
    timeout = TIMEOUT if timeout is None else timeout
    end = time.time() + timeout
    buf = ""
    while time.time() < end:
        left = max(0.01, end - time.time())
        r, _, _ = select.select([s], [], [], min(left, 0.2))
        if s in r:
            try:
                chunk = s.recv(4096)
            except socket.error:
                break
            if not chunk:
                break
            buf += chunk.decode(errors="ignore")
            if needle in buf:
                return True, buf
    return False, buf


def recv_any(s, needles, timeout=None):
    timeout = TIMEOUT if timeout is None else timeout
    end = time.time() + timeout
    buf = ""
    while time.time() < end:
        left = max(0.01, end - time.time())
        r, _, _ = select.select([s], [], [], min(left, 0.2))
        if s in r:
            try:
                chunk = s.recv(4096)
            except socket.error:
                break
            if not chunk:
                break
            buf += chunk.decode(errors="ignore")
            if any(needle in buf for needle in needles):
                return True, buf
    return False, buf


def drain(s, timeout=0.4):
    end = time.time() + timeout
    buf = ""
    while time.time() < end:
        r, _, _ = select.select([s], [], [], max(0.01, end - time.time()))
        if s in r:
            try:
                chunk = s.recv(4096)
            except socket.error:
                break
            if not chunk:
                break
            buf += chunk.decode(errors="ignore")
        else:
            break
    return buf


def register(s, nick, realname=None):
    realname = realname or nick
    send(s, f"PASS {PASSWORD}")
    send(s, f"NICK {nick}")
    send(s, f"USER {nick} 0 * :{realname}")
    ok, buf = recv_until(s, f"001 {nick}", timeout=3)
    return ok, buf


def safe_close(*sockets):
    for s in sockets:
        try:
            s.close()
        except OSError:
            pass


# ---------------------------------------------------------------------------
# Normal tests
# ---------------------------------------------------------------------------

def test_authentication():
    section("Authentication")

    c = connect()
    send(c, "PASS wrongpass")
    send(c, "NICK tmpnick")
    send(c, "USER tmpnick 0 * :tmp")
    ok, buf = recv_until(c, "464", timeout=2)
    if not ok:
        time.sleep(0.3)
        try:
            ok = c.recv(1) == b""
        except (BlockingIOError, socket.error):
            ok = False
    result("Wrong password -> 464 or connection closed", ok, buf)
    c.close()

    c = connect()
    send(c, "NICK nopasuser")
    send(c, "USER nopasuser 0 * :nopas")
    ok, buf = recv_until(c, "464", timeout=2)
    if not ok:
        time.sleep(0.3)
        try:
            ok = c.recv(1) == b""
        except (BlockingIOError, socket.error):
            ok = False
    result("Missing PASS -> 464 or connection closed", ok, buf)
    c.close()

    c = connect()
    ok, buf = register(c, "authtest")
    result("Correct PASS / NICK / USER -> 001 welcome", ok, buf)
    c.close()


def test_nick():
    section("NICK command")

    c = connect()
    ok, buf = register(c, "nicktester")
    result("Registration with valid nick", ok, buf)

    send(c, "NICK nicktester2")
    ok, buf = recv_until(c, "NICK", timeout=2)
    result("NICK change broadcast", ok, buf)

    c2 = connect()
    register(c2, "dupnick")
    c3 = connect()
    send(c3, f"PASS {PASSWORD}")
    send(c3, "NICK dupnick")
    send(c3, "USER dupnick 0 * :dup")
    ok, buf = recv_until(c3, "433", timeout=2)
    result("Duplicate nick -> 433 ERR_NICKNAMEINUSE", ok, buf)
    safe_close(c, c2, c3)

    c4 = connect()
    send(c4, f"PASS {PASSWORD}")
    send(c4, "NICK 123invalid!")
    send(c4, "USER bad 0 * :bad")
    ok, buf = recv_until(c4, "432", timeout=2)
    result("Invalid nick chars -> 432 ERR_ERRONEUSNICKNAME", ok, buf)
    c4.close()


def test_join():
    section("JOIN command")

    a = connect()
    b = connect()
    register(a, "joinalice")
    register(b, "joinbob")

    send(a, "JOIN #jointest")
    ok, buf = recv_until(a, "JOIN #jointest", timeout=2)
    result("JOIN channel -> receive JOIN confirmation", ok, buf)

    ok, buf = recv_until(a, "353", timeout=2)
    result("JOIN -> 353 RPL_NAMREPLY", ok, buf)

    ok, buf = recv_until(a, "366", timeout=2)
    result("JOIN -> 366 RPL_ENDOFNAMES", ok, buf)

    send(b, "JOIN #jointest")
    ok, buf = recv_until(a, "JOIN #jointest", timeout=2)
    result("Second user JOIN is broadcast to channel members", ok, buf)

    send(a, "JOIN nochannel")
    ok, buf = recv_until(a, "403", timeout=2)
    result("JOIN without # -> 403 ERR_NOSUCHCHANNEL", ok, buf)

    safe_close(a, b)


def test_privmsg():
    section("PRIVMSG / NOTICE")

    a = connect()
    b = connect()
    register(a, "pmsgalice")
    register(b, "pmsgbob")

    send(a, "JOIN #pmsgchan")
    send(b, "JOIN #pmsgchan")
    time.sleep(0.2)
    send(a, "PRIVMSG #pmsgchan :hello channel")
    ok, buf = recv_until(b, "PRIVMSG #pmsgchan :hello channel", timeout=3)
    result("PRIVMSG to channel delivered to other member", ok, buf)

    drain(a, 0.3)
    ok = "hello channel" not in drain(a, 0.3)
    result("PRIVMSG sender does not receive own message", ok)

    send(a, "PRIVMSG pmsgbob :hey bob")
    ok, buf = recv_until(b, "PRIVMSG pmsgbob :hey bob", timeout=3)
    result("PRIVMSG to user delivered privately", ok, buf)

    send(a, "NOTICE pmsgbob :notice text")
    ok, buf = recv_until(b, "NOTICE", timeout=2)
    result("NOTICE to user delivered", ok, buf)

    send(a, "PRIVMSG ghost_user_xyz :hi")
    ok, buf = recv_until(a, "401", timeout=2)
    result("PRIVMSG to unknown user -> 401 ERR_NOSUCHNICK", ok, buf)

    send(a, "PRIVMSG #nonexistent_chan_xyz :hi")
    ok, buf = recv_until(a, "40", timeout=2)
    result("PRIVMSG to non-existent channel -> 4xx error", ok, buf)

    safe_close(a, b)


def test_part():
    section("PART command")

    a = connect()
    b = connect()
    register(a, "partalice")
    register(b, "partbob")

    send(a, "JOIN #partchan")
    send(b, "JOIN #partchan")
    time.sleep(0.2)
    send(a, "PART #partchan :goodbye")
    ok, buf = recv_until(b, "PART", timeout=2)
    result("PART broadcast to remaining members", ok, buf)

    send(a, "PART #partchan")
    ok, buf = recv_until(a, "442", timeout=2)
    result("PART when not in channel -> 442 ERR_NOTONCHANNEL", ok, buf)

    safe_close(a, b)


def test_topic():
    section("TOPIC command")

    a = connect()
    b = connect()
    register(a, "topicalice")
    register(b, "topicbob")

    send(a, "JOIN #topicchan")
    send(b, "JOIN #topicchan")
    time.sleep(0.2)

    send(a, "TOPIC #topicchan :42 ft_irc project")
    ok, buf = recv_until(b, "TOPIC #topicchan :42 ft_irc project", timeout=2)
    result("TOPIC set by operator -> broadcast to channel", ok, buf)

    send(b, "TOPIC #topicchan")
    ok, buf = recv_until(b, "332", timeout=2)
    result("TOPIC query -> 332 RPL_TOPIC", ok, buf)

    send(a, "MODE #topicchan +t")
    time.sleep(0.2)
    send(b, "TOPIC #topicchan :hacked topic")
    ok, buf = recv_until(b, "482", timeout=2)
    result("+t mode: non-op TOPIC change -> 482 ERR_CHANOPRIVSNEEDED", ok, buf)

    safe_close(a, b)


def test_kick():
    section("KICK command")

    a = connect()
    b = connect()
    c = connect()
    register(a, "kickalice")
    register(b, "kickbob")
    register(c, "kickcharlie")

    send(a, "JOIN #kickchan")
    send(b, "JOIN #kickchan")
    send(c, "JOIN #kickchan")
    time.sleep(0.3)

    send(a, "KICK #kickchan kickbob :bye")
    ok, buf = recv_until(b, "KICK #kickchan kickbob", timeout=2)
    result("KICK by operator -> target receives KICK", ok, buf)

    ok, buf = recv_until(c, "KICK", timeout=2)
    result("KICK by operator -> broadcast to channel", ok, buf)

    send(c, "KICK #kickchan kickalice :try")
    ok, buf = recv_until(c, "482", timeout=2)
    result("KICK by non-operator -> 482 ERR_CHANOPRIVSNEEDED", ok, buf)

    send(a, "KICK #nonexistchan_xyz kickbob :nope")
    ok, buf = recv_until(a, "403", timeout=2)
    result("KICK on non-existent channel -> 403 ERR_NOSUCHCHANNEL", ok, buf)

    safe_close(a, b, c)


def test_invite():
    section("INVITE command")

    a = connect()
    b = connect()
    c = connect()
    register(a, "invalice")
    register(b, "invbob")
    register(c, "invcharlie")

    send(a, "JOIN #invchan")
    time.sleep(0.2)

    send(a, "INVITE invbob #invchan")
    ok, buf = recv_until(b, "INVITE", timeout=2)
    result("INVITE -> target receives INVITE message", ok, buf)

    ok, buf = recv_until(a, "341", timeout=2)
    result("INVITE -> inviter receives 341 RPL_INVITING", ok, buf)

    send(a, "MODE #invchan +i")
    time.sleep(0.2)
    send(c, "JOIN #invchan")
    ok, buf = recv_until(c, "473", timeout=2)
    result("+i mode: uninvited JOIN -> 473 ERR_INVITEONLYCHAN", ok, buf)

    send(a, "INVITE invcharlie #invchan")
    recv_until(c, "INVITE", timeout=2)
    send(c, "JOIN #invchan")
    ok, buf = recv_until(c, "JOIN #invchan", timeout=2)
    result("+i mode: invited user can JOIN", ok, buf)

    safe_close(a, b, c)


def test_mode():
    section("MODE command")

    a = connect()
    b = connect()
    c = connect()
    register(a, "modealice")
    register(b, "modebob")
    register(c, "modecharlie")

    send(a, "JOIN #modechan")
    send(b, "JOIN #modechan")
    send(c, "JOIN #modechan")
    time.sleep(0.3)

    send(a, "MODE #modechan +o modebob")
    ok, buf = recv_until(b, "MODE #modechan +o modebob", timeout=2)
    result("MODE +o grants operator to user", ok, buf)

    send(a, "MODE #modechan -o modebob")
    ok, buf = recv_until(b, "MODE", timeout=2)
    result("MODE -o removes operator from user", ok, buf)

    send(a, "MODE #modechan +k secret42")
    time.sleep(0.2)
    d = connect()
    register(d, "modedave")
    send(d, "JOIN #modechan")
    ok, buf = recv_until(d, "475", timeout=2)
    result("+k mode: JOIN without key -> 475 ERR_BADCHANNELKEY", ok, buf)

    send(d, "JOIN #modechan secret42")
    ok, buf = recv_until(d, "JOIN #modechan", timeout=2)
    result("+k mode: JOIN with correct key succeeds", ok, buf)

    send(a, "MODE #modechan -k")
    time.sleep(0.2)
    e = connect()
    register(e, "modeeve")
    send(e, "JOIN #modechan")
    ok, buf = recv_until(e, "JOIN #modechan", timeout=2)
    result("-k mode: JOIN without key succeeds after key removed", ok, buf)

    send(a, "MODE #modechan +l 5")
    time.sleep(0.2)
    f = connect()
    register(f, "modefrank")
    send(f, "JOIN #modechan")
    ok, buf = recv_until(f, "471", timeout=2)
    result("+l mode: JOIN beyond limit -> 471 ERR_CHANNELISFULL", ok, buf)

    send(a, "MODE #modechan -l")
    time.sleep(0.2)
    send(f, "JOIN #modechan")
    ok, buf = recv_until(f, "JOIN #modechan", timeout=2)
    result("-l mode: JOIN succeeds after limit removed", ok, buf)

    send(c, "MODE #modechan +t")
    ok, buf = recv_until(c, "482", timeout=2)
    result("MODE by non-operator -> 482 ERR_CHANOPRIVSNEEDED", ok, buf)

    send(a, "MODE #modechan +z")
    ok, buf = recv_until(a, "472", timeout=2)
    result("Unknown MODE flag -> 472 ERR_UNKNOWNMODE", ok, buf)

    safe_close(a, b, c, d, e, f)


def test_quit():
    section("QUIT command")

    a = connect()
    b = connect()
    register(a, "quitalice")
    register(b, "quitbob")

    send(a, "JOIN #quitchan")
    send(b, "JOIN #quitchan")
    time.sleep(0.2)

    send(a, "QUIT :see ya")
    ok, buf = recv_until(b, "QUIT", timeout=2)
    result("QUIT broadcast to channel members", ok, buf)

    safe_close(a, b)


def test_ping_pong():
    section("PING / PONG")

    c = connect()
    register(c, "pingtest")
    send(c, "PING :testserver")
    ok, buf = recv_until(c, "PONG", timeout=2)
    result("PING -> server replies with PONG", ok, buf)
    c.close()


def test_errors():
    section("General error codes")

    c = connect()
    register(c, "errtester")

    send(c, "FOOBARCOMMAND")
    ok, buf = recv_until(c, "421", timeout=2)
    result("Unknown command -> 421 ERR_UNKNOWNCOMMAND", ok, buf)

    send(c, "JOIN")
    ok, buf = recv_until(c, "461", timeout=2)
    result("JOIN with no params -> 461 ERR_NEEDMOREPARAMS", ok, buf)

    send(c, "KICK")
    ok, buf = recv_until(c, "461", timeout=2)
    result("KICK with no params -> 461 ERR_NEEDMOREPARAMS", ok, buf)

    c.close()


def test_operator_privileges():
    section("Operator privileges")

    a = connect()
    b = connect()
    register(a, "opalice")
    register(b, "opbob")

    send(a, "JOIN #opchan")
    send(b, "JOIN #opchan")
    time.sleep(0.2)

    send(b, "KICK #opchan opalice :haha")
    ok, buf = recv_until(b, "482", timeout=2)
    result("Channel creator has op; non-op KICK -> 482", ok, buf)

    send(a, "MODE #opchan +o opbob")
    time.sleep(0.2)
    c = connect()
    register(c, "opcharlie")
    send(c, "JOIN #opchan")
    time.sleep(0.2)
    send(b, "KICK #opchan opcharlie :kicked by bob")
    ok, buf = recv_until(c, "KICK #opchan opcharlie", timeout=2)
    result("Newly granted operator can KICK", ok, buf)

    safe_close(a, b, c)


def test_multiple_channels():
    section("Multiple channels")

    a = connect()
    b = connect()
    register(a, "mulalice")
    register(b, "mulbob")

    send(a, "JOIN #mulchan1")
    send(a, "JOIN #mulchan2")
    send(b, "JOIN #mulchan1")
    time.sleep(0.3)

    send(a, "PRIVMSG #mulchan1 :msg to chan1")
    send(a, "PRIVMSG #mulchan2 :msg to chan2")

    ok, buf = recv_until(b, "msg to chan1", timeout=2)
    result("Message in #mulchan1 received by member", ok, buf)

    drain(b, 0.4)
    ok, buf = recv_until(b, "msg to chan2", timeout=1)
    result("Message in #mulchan2 not received by non-member", not ok, buf)

    safe_close(a, b)


def test_regression_edge_cases():
    section("Regression / edge cases")

    c = connect()
    send(c, f"PASS {PASSWORD}")
    send(c, "USER nonick 0 * :No Nick")
    ok, buf = recv_until(c, "431", timeout=2)
    result("PASS + USER without NICK -> 431, no 001 welcome", ok and "001 *" not in buf, buf)

    send(c, "NICK late_nick")
    ok, buf = recv_until(c, "001 late_nick", timeout=2)
    result("NICK after USER completes delayed registration", ok, buf)
    c.close()

    a = connect()
    b = connect()
    outsider = connect()
    register(a, "edgealice")
    register(b, "edgebob")
    register(outsider, "edgemallory")

    send(a, "JOIN #edgekick")
    time.sleep(0.2)
    drain(a)
    send(a, "KICK #edgekick edgebob :not in channel")
    ok, buf = recv_until(a, "441", timeout=2)
    result("KICK target not in channel -> 441 ERR_USERNOTINCHANNEL", ok, buf)

    send(a, "JOIN #edgetopic")
    time.sleep(0.2)
    drain(a)
    send(a, "TOPIC #edgetopic :original topic")
    time.sleep(0.2)
    drain(a)
    send(outsider, "TOPIC #edgetopic :hacked topic")
    ok, buf = recv_until(outsider, "442", timeout=2)
    result("TOPIC by non-member -> 442 ERR_NOTONCHANNEL", ok, buf)

    send(a, "TOPIC #edgetopic")
    ok, buf = recv_until(a, "332", timeout=2)
    result("Rejected TOPIC does not change existing topic", ok and "original topic" in buf and "hacked topic" not in buf, buf)

    send(a, "JOIN #edgenotice")
    time.sleep(0.2)
    drain(a)
    send(outsider, "NOTICE #edgenotice :outside spam")
    data = drain(a, 0.8)
    result("NOTICE to channel from non-member is not delivered", "outside spam" not in data, data)

    send(a, "JOIN #edgenick")
    send(b, "JOIN #edgenick")
    time.sleep(0.3)
    drain(a)
    drain(b)
    send(a, "NICK edgealice")
    data_a = drain(a, 0.6)
    data_b = drain(b, 0.6)
    result(
        "NICK with same nickname is ignored",
        " NICK :edgealice" not in data_a and " NICK :edgealice" not in data_b,
        data_a + data_b,
    )

    send(a, "JOIN #edgemode")
    time.sleep(0.2)
    drain(a)
    for command, label in (
        ("MODE #edgemode +k", "MODE +k without key -> 461"),
        ("MODE #edgemode +o", "MODE +o without nick -> 461"),
        ("MODE #edgemode +l", "MODE +l without limit -> 461"),
    ):
        send(a, command)
        ok, buf = recv_until(a, "461", timeout=2)
        result(label, ok, buf)

    safe_close(a, b, outsider)


# ---------------------------------------------------------------------------
# Bonus and global tests
# ---------------------------------------------------------------------------

def test_bonus_dcc_relay():
    section("Bonus: DCC/file-transfer relay")

    a = connect()
    b = connect()
    register(a, "dccalice")
    register(b, "dccbob")

    payload = "\x01DCC SEND hello.txt 2130706433 9000 12\x01"
    send(a, f"PRIVMSG dccbob :{payload}")
    ok, buf = recv_until(b, "DCC SEND hello.txt", timeout=2)
    result("DCC SEND payload is relayed through PRIVMSG", ok, buf)

    payload = "\x01DCC CHAT chat 2130706433 9001\x01"
    send(a, f"PRIVMSG dccbob :{payload}")
    ok, buf = recv_until(b, "DCC CHAT chat", timeout=2)
    result("DCC CHAT payload is relayed through PRIVMSG", ok, buf)

    safe_close(a, b)


def test_bonus_bot_hook():
    section("Bonus: bot hook")

    bot_nick = CURRENT.bot_nick
    if not bot_nick:
        skipped("Bot command response", "set --bot-nick if your bonus includes a bot")
        return

    c = connect()
    register(c, "botprobe")
    drain(c)
    send(c, f"PRIVMSG {bot_nick} :!help")
    ok, buf = recv_any(c, ["PRIVMSG botprobe", "NOTICE botprobe"], timeout=3)
    result(f"Bot {bot_nick!r} replies to !help", ok, buf)
    c.close()


def test_fragmented_lines():
    section("Global: fragmented input")

    c = connect()
    c.sendall(f"PASS {PASSWORD}\r\nN".encode())
    time.sleep(0.1)
    c.sendall(b"ICK fraguser\r\nUSER fraguser 0 * :frag\r\n")
    ok, buf = recv_until(c, "001 fraguser", timeout=3)
    result("Registration succeeds when commands arrive fragmented", ok, buf)
    c.close()


def test_batched_commands():
    section("Global: batched commands")

    c = connect()
    c.sendall(
        (
            f"PASS {PASSWORD}\r\n"
            "NICK batchuser\r\n"
            "USER batchuser 0 * :batch\r\n"
            "JOIN #batchchan\r\n"
            "PRIVMSG #batchchan :batched hello\r\n"
        ).encode()
    )
    ok, buf = recv_until(c, "JOIN #batchchan", timeout=3)
    result("Server parses multiple commands in one TCP packet", ok and "001 batchuser" in buf, buf)
    c.close()


def test_many_clients():
    section("Global: many clients")

    sockets = []
    try:
        for i in range(CURRENT.clients):
            s = connect()
            ok, buf = register(s, f"load{i:02d}")
            result(f"Client load{i:02d} registers", ok, buf)
            if not ok:
                s.close()
                continue
            sockets.append(s)

        if len(sockets) < 2:
            skipped("Many-client channel broadcast", "not enough clients registered")
            return

        for s in sockets:
            send(s, "JOIN #loadchan")
        time.sleep(0.4)
        drain(sockets[-1])
        send(sockets[0], "PRIVMSG #loadchan :load hello")
        ok, buf = recv_until(sockets[-1], "load hello", timeout=3)
        result(f"Broadcast works with {len(sockets)} clients", ok, buf)
    finally:
        safe_close(*sockets)


def test_channel_lifecycle():
    section("Global: channel lifecycle")

    a = connect()
    b = connect()
    register(a, "lifealice")
    register(b, "lifebob")

    send(a, "JOIN #lifechan")
    time.sleep(0.2)
    send(a, "PART #lifechan")
    time.sleep(0.2)
    send(b, "JOIN #lifechan")
    ok, buf = recv_until(b, "JOIN #lifechan", timeout=2)
    result("Channel can be recreated after last user leaves", ok, buf)

    safe_close(a, b)


NORMAL_TESTS = (
    test_authentication,
    test_nick,
    test_join,
    test_privmsg,
    test_part,
    test_topic,
    test_kick,
    test_invite,
    test_mode,
    test_quit,
    test_ping_pong,
    test_errors,
    test_operator_privileges,
    test_multiple_channels,
)

BONUS_TESTS = (
    test_bonus_dcc_relay,
    test_bonus_bot_hook,
)

GLOBAL_EXTRA_TESTS = (
    test_regression_edge_cases,
    test_fragmented_lines,
    test_batched_commands,
    test_many_clients,
    test_channel_lifecycle,
)


# ---------------------------------------------------------------------------
# Runner and manual terminal
# ---------------------------------------------------------------------------

def run_suite(name, tests):
    reset_counters()
    print(f"\n{BOLD}{MAGENTA}Running {name} suite{RESET}")
    for test in tests:
        try:
            test()
        except (ConnectionRefusedError, ConnectionResetError, BrokenPipeError, OSError) as exc:
            result(test.__name__, False, f"{type(exc).__name__}: {exc}")
    print_summary()
    return FAIL == 0


def print_summary():
    total = PASS + FAIL + SKIP
    print()
    rule(MAGENTA)
    print(f"  Results: {GREEN}{PASS} passed{RESET} / {RED}{FAIL} failed{RESET} / {YELLOW}{SKIP} skipped{RESET} / {total} total")
    if FAIL == 0:
        print(f"  {GREEN}{BOLD}Suite finished cleanly.{RESET}")
    else:
        print(f"  {RED}{BOLD}{FAIL} test(s) failed.{RESET}")
    rule(MAGENTA)


def raw_terminal():
    section("Raw IRC terminal")
    print("Type IRC commands exactly as you want to send them.")
    print("Use /quit to leave this tester terminal.\n")

    nick = input("Nick [blackwolf]: ").strip() or "blackwolf"
    c = connect()
    ok, buf = register(c, nick)
    if buf:
        print(f"{DIM}{buf.rstrip()}{RESET}")
    if not ok:
        print(f"{RED}Registration did not receive 001. You can still type raw commands.{RESET}")

    try:
        while True:
            ready, _, _ = select.select([c, sys.stdin], [], [], 0.2)
            if c in ready:
                data = c.recv(4096)
                if not data:
                    print(f"\n{RED}Server closed the connection.{RESET}")
                    break
                print(f"{CYAN}{data.decode(errors='ignore').rstrip()}{RESET}")
            if sys.stdin in ready:
                line = sys.stdin.readline()
                if not line:
                    break
                line = line.rstrip("\n")
                if line == "/quit":
                    break
                send(c, line)
    finally:
        safe_close(c)


def choose_mode():
    print()
    print(f"{BOLD}{CYAN}Choose test mode{RESET}")
    print(f"  {GREEN}1{RESET}  normal    mandatory ft_irc checks")
    print(f"  {GREEN}2{RESET}  bonus     DCC relay and optional bot hook")
    print(f"  {GREEN}3{RESET}  global    normal + bonus + stress/regression checks")
    print(f"  {GREEN}4{RESET}  terminal  manual IRC command terminal")
    print(f"  {GREEN}5{RESET}  quit")
    answer = input(f"\n{BOLD}Select [1]: {RESET}").strip().lower() or "1"
    return {
        "1": "normal",
        "normal": "normal",
        "2": "bonus",
        "bonus": "bonus",
        "3": "global",
        "global": "global",
        "4": "terminal",
        "terminal": "terminal",
        "5": "quit",
        "quit": "quit",
    }.get(answer, "normal")


def suite_for_mode(mode):
    if mode == "normal":
        return "normal", NORMAL_TESTS
    if mode == "bonus":
        return "bonus", BONUS_TESTS
    if mode == "global":
        return "global", NORMAL_TESTS + BONUS_TESTS + GLOBAL_EXTRA_TESTS
    return None, None


def parse_args():
    parser = argparse.ArgumentParser(
        description="Terminal tester for 42 ft_irc servers.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--host", default=HOST, help="IRC server host")
    parser.add_argument("--port", type=int, default=PORT, help="IRC server port")
    parser.add_argument("--password", default=PASSWORD, help="IRC server password")
    parser.add_argument("--binary", default=BINARY, help="Path to ircserv binary")
    parser.add_argument("--timeout", type=float, default=TIMEOUT, help="Read timeout in seconds")
    parser.add_argument("--mode", choices=("menu", "normal", "bonus", "global", "terminal"), default="menu", help="Tester mode")
    parser.add_argument("--no-start", action="store_true", help="Connect to an already running server")
    parser.add_argument("--no-intro", action="store_true", help="Skip the ASCII intro")
    parser.add_argument("--fast", action="store_true", help="Speed up the ASCII intro")
    parser.add_argument("--clients", type=int, default=12, help="Client count for global load test")
    parser.add_argument("--bot-nick", default=os.getenv("IRC_TEST_BOT_NICK"), help="Optional bot nick for bonus bot test")
    return parser.parse_args()


def main():
    args = parse_args()
    apply_args(args)
    intro(args)
    banner(args)

    exit_ok = True
    srv = None
    try:
        mode = choose_mode() if args.mode == "menu" else args.mode
        if mode == "quit":
            return
        srv = start_server()
        if mode == "terminal":
            raw_terminal()
            return
        name, tests = suite_for_mode(mode)
        exit_ok = run_suite(name, tests)
    except KeyboardInterrupt:
        print("\nInterrupted.")
        exit_ok = False
    finally:
        stop_server(srv)

    sys.exit(0 if exit_ok else 1)


if __name__ == "__main__":
    main()
