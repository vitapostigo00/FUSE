#include "fuseHeaders.h"
#include "fileSystemLib.c"
#include "dataSystemLib.c"
#include "fileSystemUtils.c"

int main(){
	init("filesystem.bin");
	init_datasystem("dataSystem.bin");
	//createFile("pepe", "/home/master/cosas/paco");
	int idx=exists("pepe");
	printf("%s\n", cat(fs[idx].hasData));
}
