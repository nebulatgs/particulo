# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.187.0/containers/codespaces-linux/.devcontainer/base.Dockerfile

FROM mcr.microsoft.com/vscode/devcontainers/universal:1-focal

# ** [Optional] Uncomment this section to install additional packages. **
USER root

RUN apt update && export DEBIAN_FRONTEND=noninteractive \
    && apt -y autoremove g++ gcc \
    && apt -y install --no-install-recommends clangd clang-format g++-10 ninja-build libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

USER codespace
