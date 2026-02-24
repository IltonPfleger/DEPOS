default: build

clone: 
	git clone https://github.com/luxonis/depthai-core.git --depth 1
	git clone https://bitbucket.org/lpresearch/openzen.git --depth 1
	git clone https://github.com/yhirose/cpp-httplib.git --depth 1
	git clone https://github.com/artyom-beilis/dlprimitives.git --depth 1
	@cd dlprimitives && git apply ../dlprimitives.patch

update:
	@cd depthai-core && git pull
	@cd dlprimitives && git pull

build:
	@cd depthai-core && git submodule update --init --recursive && cmake -S. -Bbuild && cmake --build build
	@cd openzen && git submodule update --init --recursive && cmake -S. -Bbuild && cmake --build build && mv build/OpenZenConfigTargets.cmake build/OpenZenTargets.cmake
	@cd dlprimitives && mkdir -p build && cd build && cmake .. -DPYTHON_EXECUTABLE=$(which python3.10) && make -j4 && cp -u dlprim/config.hpp ../include/dlprim/