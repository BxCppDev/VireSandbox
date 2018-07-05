#!/bin/sh

sources=$PWD

mkdir -p _build.d
mkdir -p _install.d

# rm -f _build.d/*
rm -f _install.d/*

cd _build.d
cmake -DCMAKE_INSTALL_PREFIX=../_install.d \
      -DBOOST_ROOT="$(brew --prefix Boost)" \
      -DVire_DIR="$(virequery --cmakedir)" \
      -DBxRabbitMQ_DIR="$(bxrabbitmq-query --cmakedir)" \
      -DQt5Core_DIR="$(brew --prefix qt5-base)/lib/cmake/Qt5Core" \
      -DQt5Gui_DIR="$(brew --prefix qt5-base)/lib/cmake/Qt5Gui" \
      -DQt5Widgets_DIR="$(brew --prefix qt5-base)/lib/cmake/Qt5Widgets" \
      -DQt5Svg_DIR="$(brew --prefix qt5-base)/lib/cmake/Qt5Svg" \
      ..
make
make install
cd ..

echo ""
echo "#########################################################################"
echo ""
echo "  CMS tests :"
echo ""
echo "    install -> ./_install.d"
echo ""
echo "#########################################################################"
echo ""

