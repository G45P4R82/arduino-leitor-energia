.PHONY: logs

PYTHON := ../../.venv/bin/python
PORT ?= /dev/ttyUSB0
BAUD ?= 115200
LOG_DIR := logs
LOG_FILE := $(LOG_DIR)/iot003-serial.log

logs:
	@mkdir -p $(LOG_DIR)
	@echo "Monitorando $(PORT) a $(BAUD) baud"
	@echo "Salvando em $(LOG_FILE)"
	@$(PYTHON) -m serial.tools.miniterm $(PORT) $(BAUD) 2>&1 | tee $(LOG_FILE)
