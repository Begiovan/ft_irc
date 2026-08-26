FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Makefile ./
COPY includes ./includes
COPY src ./src

RUN make

FROM debian:bookworm-slim

RUN useradd --system --uid 10001 --home /app --shell /usr/sbin/nologin ftirc

WORKDIR /app
COPY --from=build /app/ircserv /app/ircserv
RUN chown -R ftirc:ftirc /app

USER ftirc
EXPOSE 6667/tcp

CMD ["sh", "-c", "exec /app/ircserv \"${IRC_PORT:-6667}\" \"${IRC_PASS:?Set IRC_PASS}\""]
