#!/bin/bash


buildProto() {
	echo "Build Protocol ..."
	protoc -I=./server/proto --cpp_out=./server/protocol/out ./server/proto/cs.proto
	protoc -I=./server/proto --cpp_out=./server/protocol/in ./server/proto/gs.proto	
}

buildKhaki() {
	echo "Build Network Lib Khaki ..."
	cd ./server/khaki 
	if [ -d "build" ]; then
		rm -rf build	
	fi
	mkdir build && cd build && cmake .. && make
}

buildGateway() {
	echo "Build Gateway ..."
	if [ -d "build" ]; then
		rm -rf build
	fi
	mkdir build && cd build && mkdir bin && cmake .. && make
}

case $1 in
	"-k")
	buildKhaki
	;;
	"-g")
	buildGateway
	;;
	"-p")
	buildProto
	;;
esac