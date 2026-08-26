#!/usr/bin/env bash
set -euo pipefail
APP_DIR="${APP_DIR:-$HOME/42_ft_irc}"
cd "$APP_DIR"
docker compose ps
docker compose logs --tail=80 ft-irc
