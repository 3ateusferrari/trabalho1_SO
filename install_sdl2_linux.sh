#!/bin/bash

echo "Instalando SDL2 para Linux..."
echo

# Detectar distribuição
if command -v apt-get &> /dev/null; then
    echo "Detectada distribuição baseada em Debian/Ubuntu"
    echo "Instalando dependências..."
    sudo apt update
    sudo apt install -y build-essential
    sudo apt install -y libsdl2-dev libsdl2-ttf-dev
elif command -v dnf &> /dev/null; then
    echo "Detectada distribuição baseada em Fedora/RHEL"
    echo "Instalando dependências..."
    sudo dnf install -y gcc make
    sudo dnf install -y SDL2-devel SDL2_ttf-devel
elif command -v pacman &> /dev/null; then
    echo "Detectada distribuição baseada em Arch"
    echo "Instalando dependências..."
    sudo pacman -S --noconfirm base-devel
    sudo pacman -S --noconfirm sdl2 sdl2_ttf
else
    echo "Distribuição não suportada automaticamente."
    echo "Por favor, instale manualmente:"
    echo "- build-essential (ou equivalente)"
    echo "- libsdl2-dev"
    echo "- libsdl2-ttf-dev"
    exit 1
fi

echo
echo "Instalação concluída!"
echo
echo "Para compilar o jogo:"
echo "1. Navegue até a pasta src: cd src"
echo "2. Execute: make"
echo "3. Execute: ./jogo"
echo 