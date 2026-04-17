# syntax=docker/dockerfile:1

FROM debian:bookworm-slim

ARG GBDK_VERSION=4.5.0
ARG TARGETARCH

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      make \
      curl \
      ca-certificates \
      tar \
      xz-utils \
    && rm -rf /var/lib/apt/lists/*

RUN case "${TARGETARCH}" in \
      arm64) TARBALL="gbdk-linux-arm64.tar.gz" ;; \
      *)     TARBALL="gbdk-linux64.tar.gz" ;; \
    esac && \
    curl -fsSL "https://github.com/gbdk-2020/gbdk-2020/releases/download/${GBDK_VERSION}/${TARBALL}" | tar -xz -C /opt

ENV PATH="/opt/gbdk/bin:${PATH}"
ENV GBDK_HOME="/opt/gbdk"

WORKDIR /bananas
