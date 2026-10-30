#!/bin/bash
# As probas de Barullo Bombo na CI común (escoita-nucleo): chámase desde a raíz con
# mac, windows, linux ou sanitizadores, coa compilación en build/
set -eo pipefail
SO="$1"
EXE=""; [ "$SO" = windows ] && EXE=".exe"
CONF=Release; [ "$SO" = sanitizadores ] && CONF=RelWithDebInfo
RT="build/BBRenderTest_artefacts/$CONF/BBRenderTest$EXE"

echo "::group::Son (os motores, a afinación, a caída, o barrido, o MIDI, os presets e o estado)"
"$RT" proba.wav
echo "::endgroup::"
