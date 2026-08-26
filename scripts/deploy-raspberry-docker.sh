#!/usr/bin/env bash
set -euo pipefail

APP_DIR="${APP_DIR:-$HOME/42_ft_irc}"
BRANCH="${BRANCH:-main}"
SERVICE="${SERVICE:-ft-irc}"

cd "$APP_DIR"

echo "== Repo =="
git fetch origin "$BRANCH"
git pull --ff-only origin "$BRANCH"
git log --oneline -1

if [ ! -f .env ]; then
  echo "ERRORE: manca $APP_DIR/.env"
  echo "Crea il file con: cp .env.example .env"
  echo "Poi modifica IRC_PASS prima di avviare il deploy."
  exit 1
fi

set -a
# shellcheck disable=SC1091
. ./.env
set +a

PORT="${IRC_EXTERNAL_PORT:-${IRC_PORT:-6667}}"

echo "== Build Docker image =="
docker compose build "$SERVICE"

echo "== Start/restart container =="
docker compose up -d --no-deps "$SERVICE"

echo "== Container status =="
docker compose ps

echo "== Last logs =="
docker compose logs --tail=40 "$SERVICE"

echo "== Port check =="
if command -v ss >/dev/null 2>&1; then
  ss -ltn | grep -E ":${PORT}[[:space:]]" >/dev/null
else
  netstat -ltn | grep -E "[.:]${PORT}[[:space:]]" >/dev/null
fi

echo "Deploy OK: ft_irc ascolta sulla porta ${PORT}"
