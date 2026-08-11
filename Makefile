# UIF Freeroam - Kurzbefehle rund um Build und Betrieb

.PHONY: all setup compile run clean distclean help

all: compile

help:
	@echo "make setup      Includes holen und Pawn-Compiler bauen"
	@echo "make compile    Gamemode uebersetzen (-> gamemodes/uif.amx)"
	@echo "make run        Server starten"
	@echo "make clean      Uebersetztes Gamemode entfernen"
	@echo "make distclean  Zusaetzlich Toolchain und Buildreste entfernen"

setup:
	@./setup.sh

compile: gamemodes/uif.amx

# Jede Quelldatei loest eine Neuuebersetzung aus - Pawn kennt keine
# Teiluebersetzung, das gesamte Gamemode ist eine Uebersetzungseinheit.
gamemodes/uif.amx: gamemodes/uif.pwn $(wildcard src/*/*.inc)
	@./compile.sh

run: compile
	@./run.sh

clean:
	@rm -f gamemodes/uif.amx
	@echo "gamemodes/uif.amx entfernt."

distclean: clean
	@rm -rf .toolchain .build pawno/include/*.inc
	@echo "Toolchain und Includes entfernt - 'make setup' stellt sie wieder her."
