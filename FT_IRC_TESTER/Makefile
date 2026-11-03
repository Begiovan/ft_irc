PYTHON ?= python3
BINARY ?= ./ircserv
HOST ?= 127.0.0.1
PORT ?= 6667
PASSWORD ?= mypass
MODE ?= menu
CLIENTS ?= 12
BOT_NICK ?=

.PHONY: all test normal bonus global terminal external help

all: test

test:
	@$(PYTHON) scripts/test_irc.py --mode $(MODE) --binary $(BINARY) --host $(HOST) --port $(PORT) --password $(PASSWORD) --clients $(CLIENTS) --bot-nick "$(BOT_NICK)"

normal:
	@$(PYTHON) scripts/test_irc.py --mode normal --binary $(BINARY) --host $(HOST) --port $(PORT) --password $(PASSWORD) --clients $(CLIENTS) --bot-nick "$(BOT_NICK)"

bonus:
	@$(PYTHON) scripts/test_irc.py --mode bonus --binary $(BINARY) --host $(HOST) --port $(PORT) --password $(PASSWORD) --clients $(CLIENTS) --bot-nick "$(BOT_NICK)"

global:
	@$(PYTHON) scripts/test_irc.py --mode global --binary $(BINARY) --host $(HOST) --port $(PORT) --password $(PASSWORD) --clients $(CLIENTS) --bot-nick "$(BOT_NICK)"

terminal:
	@$(PYTHON) scripts/test_irc.py --mode terminal --binary $(BINARY) --host $(HOST) --port $(PORT) --password $(PASSWORD)

external:
	@$(PYTHON) scripts/test_irc.py --no-start --mode $(MODE) --host $(HOST) --port $(PORT) --password $(PASSWORD) --clients $(CLIENTS) --bot-nick "$(BOT_NICK)"

help:
	@$(PYTHON) scripts/test_irc.py --help
