

.PHONY: psql, pbc, pbpy 

psql:
	psql -h 127.0.0.1  -U speedrun -d src2 


pbc:
	nanopb/generator/nanopb_generator  -s type:FT_POINTER implant.proto
	mv implant.pb.h implant/include/
	mv implant.pb.c implant/

pbpy:
	protoc  implant.proto --python_out=./speedrun/speedrun/  



