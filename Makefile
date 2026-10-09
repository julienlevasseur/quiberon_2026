FQBN      := arduino:avr:uno
SKETCH    := firmware/quiberon_bike
BUILD     := build
AVR_CORE  := arduino:avr@1.8.8
FASTLED   := FastLED@3.10.6
PORT      ?= /dev/ttyACM0
# RAM is the scarce resource (300 LEDs = 900 B of 2048): shrink the USB serial buffers.
EXTRA     := -DSERIAL_RX_BUFFER_SIZE=16 -DSERIAL_TX_BUFFER_SIZE=32

.PHONY: all setup test firmware upload monitor clean sim diagrams

all: test firmware

# One-time: Uno toolchain and pinned libraries (arduino-cli must be on PATH).
setup:
	arduino-cli core update-index
	arduino-cli core install $(AVR_CORE)
	arduino-cli lib install $(FASTLED)

# Host unit and scenario tests: no hardware needed.
test:
	cmake -S test -B $(BUILD)/host -DCMAKE_BUILD_TYPE=Debug >/dev/null
	cmake --build $(BUILD)/host -j
	$(BUILD)/host/tests

# Compile for the Uno and enforce the RAM/flash budget.
firmware:
	@mkdir -p $(BUILD)
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD)/uno --warnings default \
		--build-property "compiler.cpp.extra_flags=$(EXTRA)" \
		--format json $(SKETCH) > $(BUILD)/compile.json || true
	python3 tools/check_size.py < $(BUILD)/compile.json

upload: firmware
	arduino-cli upload --fqbn $(FQBN) --input-dir $(BUILD)/uno -p $(PORT) $(SKETCH)

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=115200

clean:
	rm -rf $(BUILD)

# Visual simulator: one HTML replay per scenario in build/sim/ (open index.html).
sim:
	cmake -S test -B $(BUILD)/host -DCMAKE_BUILD_TYPE=Debug >/dev/null
	cmake --build $(BUILD)/host -j --target sim
	@mkdir -p $(BUILD)/sim
	$(BUILD)/host/sim $(BUILD)/sim

# Documentation drawings: docs/img/ and docs/guide/img/ (needs g++ and python3).
diagrams:
	python3 tools/diagrams/garland_wiring.py
	python3 tools/diagrams/build_guide.py
