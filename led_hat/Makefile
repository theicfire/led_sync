all:
		platformio run

upload:
		platformio run -t upload

test: build/angle_estimate
	./build/angle_estimate

build/angle_estimate:
	mkdir -p build
	g++ src/angle_estimate.cpp -o build/angle_estimate

clean:
	rm -rf build

