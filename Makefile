# Jebiga-Gaming - Kurzbefehle rund um Build und Betrieb

.PHONY: all setup compile run clean distclean help

all: compile

help:
	@echo "make setup      Includes holen und Pawn-Compiler bauen"
	@echo "make compile    Gamemode und Filterscripts uebersetzen"
	@echo "make run        Server starten"
	@echo "make clean      Uebersetztes Gamemode entfernen"
	@echo "make distclean  Zusaetzlich Toolchain und Buildreste entfernen"

setup:
	@./setup.sh

compile: gamemodes/jebiga.amx

# Jede Quelldatei loest eine Neuuebersetzung aus - Pawn kennt keine
# Teiluebersetzung, das gesamte Gamemode ist eine Uebersetzungseinheit.
gamemodes/jebiga.amx: gamemodes/jebiga.pwn $(wildcard src/*/*.inc) $(wildcard filterscripts/*.pwn)
	@./compile.sh

run: compile
	@./run.sh

clean:
	@rm -f gamemodes/jebiga.amx filterscripts/*.amx
	@echo "Uebersetzte Skripte entfernt."

distclean: clean
	@rm -rf .toolchain .build pawno/include/*.inc plugins/*.so
	@echo "Toolchain und Includes entfernt - 'make setup' stellt sie wieder her."
