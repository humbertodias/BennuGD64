BUILD_TYPE := Debug

build/bgdc:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build build --target bgdc

build/bgdi:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build build --target bgdi

clean:
	rm -rf build

format:
	find core modules -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
		-print0 | xargs -0 clang-format -i --style=file
