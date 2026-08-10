build/bgdc:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target bgdc

build/bgdi:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --target bgdi

clean:
	rm -rf build
