.PHONY: build build-demo clean

build:
	CC=arm-none-eabi-gcc CXX=arm-none-eabi-g++ pebble build

build-demo:
	DEMO_DATA=1 CC=arm-none-eabi-gcc CXX=arm-none-eabi-g++ pebble build

clean:
	rm -rf build/
	pebble clean
