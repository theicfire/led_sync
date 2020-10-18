all:
		platformio run

upload:
		platformio run -t upload

test: build/angle_estimate
	./build/angle_estimate

testcopy: build/angle_estimate
	./build/angle_estimate | pbcopy

build/angle_estimate: src/angle_estimate.cpp
	mkdir -p build
	g++ src/angle_estimate.cpp -o build/angle_estimate

clean:
	rm -rf build

