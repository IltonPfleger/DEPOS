default: all

clean:
	(rm -r -f build/* && rm -r -f logs/* && rm -r -f ../python_handler/logs/* && rm -r -f ../external/*/logs/*)
	(cd bin && find . -type f -not -name "*.md" -not -name "*.key" -not -name "*.pem" -exec rm {} +)

test:
	(cd build && cmake .. -DAPPLICATION=${APPLICATION} -D'NO_DATA_SOURCE=ON' && make smartdata)

debug-test:
	(cd build && cmake .. -DAPPLICATION=${APPLICATION} -DCMAKE_BUILD_TYPE=Debug -D'NO_DATA_SOURCE=ON' && make smartdata)

debug:
	(cd build && cmake .. -DAPPLICATION=${APPLICATION} -DCMAKE_BUILD_TYPE=Debug && make smartdata)

all:
	(cd build && cmake .. -DAPPLICATION=${APPLICATION} && make smartdata)

hardware:
	cd build && cmake .. \
		-DAPPLICATION=${APPLICATION} \
		-DCMAKE_C_FLAGS="-pthread" \
		-DUSE_LIDAR_HARDWARE="ON" \
		-DUSE_IMU_HARDWARE="ON" \
		-DUSE_CAMERA_HARDWARE="ON" \
		-DUSE_CLOUD_INTEGRATION="ON" \
		-DOpenZen_DIR="./lib/openzen/build" \
		-Ddepthai_DIR="./lib/depthai-core/build" && make smartdata -j${nproc}

hardware_preview:
	cd build && cmake .. \
		-DAPPLICATION=${APPLICATION} \
		-DCMAKE_C_FLAGS="-pthread" \
		-DUSE_LIDAR_HARDWARE="ON" \
		-DUSE_LIDAR_VIEWER="ON" \
		-DUSE_IMU_HARDWARE="ON" \
		-DUSE_CAMERA_HARDWARE="ON" \
		-DUSE_CAMERA_VIEWER="ON" \
		-DOpenZen_DIR="./lib/openzen/build" \
		-Ddepthai_DIR="./lib/depthai-core/build" && make smartdata -j${nproc}
