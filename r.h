#pragma once
#include<string>
#include<vector>
#include<array>
#include<cassert>
#include<cstdint>
#include<iostream>
#include"connector.h"

struct init_data{
	uint64_t seed;
	int fd;
};

init_data initialize(int argc, char* argv[]){
	init_data d;
	if(argc==4){
		d.fd   = connect(argv[1],atol(argv[2]));
		d.seed = atoll(argv[3]);
	}else if(argc==3){
		d.fd   = connect(argv[1],atol(argv[2]));
		d.seed = std::chrono::system_clock::now().time_since_epoch().count();
	}else{
		d.fd   = connect("localhost",3720);
		d.seed = std::chrono::system_clock::now().time_since_epoch().count();
	}

	if(d.fd<0){
		std::cout << "unable to connect\n"; 
		exit(1);
	}else std::cout<<"connected!\n";

	return d;
}

std::string make_coordinate(int x, int y){
	assert(x>=0 && x<10);
	assert(y>=0 && y<10);
	std::string str = "";
	str += (x+'A');
	if(y==0) str+="10";
	else str += (9-y+'1');
	return str;
}

using Map = std::array<std::string,10>;

#define PUSH_BACK_MOVE(X,Y) ret.push_back(make_coordinate(x,y)+" "+make_coordinate(X,Y))

std::vector<std::string> list_valid_moves(Map map){
	std::vector<std::string> ret;
	for(int y=0; y<10; y+=1){
		assert(map[y].size()==10);
		for(int x=0; x<10; x+=1){
			if(map[y][x]=='2'){
				int xmax,xmin,ymax,ymin;
				// find all empty squares in a cross
				for(xmax=1; (x+xmax<10)&&map[y][x+xmax]=='.'; xmax+=1) PUSH_BACK_MOVE(x+xmax,y     );
				for(xmin=1; (x-xmin>=0)&&map[y][x-xmin]=='.'; xmin+=1) PUSH_BACK_MOVE(x-xmin,y     );
				for(ymax=1; (y+ymax<10)&&map[y+ymax][x]=='.'; ymax+=1) PUSH_BACK_MOVE(x     ,y+ymax);
				for(ymin=1; (y-ymin>=0)&&map[y-ymin][x]=='.'; ymin+=1) PUSH_BACK_MOVE(x     ,y-ymin);
				if((x+xmax<10)&&map[y][x+xmax]=='#')                 PUSH_BACK_MOVE(x+xmax,y     );
				if((x-xmin>=0)&&map[y][x-xmin]=='#')                 PUSH_BACK_MOVE(x-xmin,y     );
				if((y+ymax<10)&&map[y+ymax][x]=='#')                 PUSH_BACK_MOVE(x     ,y+ymax);
				if((y-ymin>=0)&&map[y-ymin][x]=='#')                 PUSH_BACK_MOVE(x     ,y-ymin);
			}else if((map[y][x]>='0'&&map[y][x]<='9') || map[y][x]=='M'){
				if(x!=9 && (map[y][x+1]=='.' || map[y][x+1]=='#')) PUSH_BACK_MOVE(x+1,y);
				if(x!=0 && (map[y][x-1]=='.' || map[y][x-1]=='#')) PUSH_BACK_MOVE(x-1,y);
				if(y!=9 && (map[y+1][x]=='.' || map[y+1][x]=='#')) PUSH_BACK_MOVE(x,y+1);
				if(y!=0 && (map[y-1][x]=='.' || map[y-1][x]=='#')) PUSH_BACK_MOVE(x,y-1);
			}
		}
	}
	return ret;
};

Map receive_map(int fd){
	Map map;
	map[0] = receiveln(fd);
	map[1] = receiveln(fd);
	map[2] = receiveln(fd);
	map[3] = receiveln(fd);
	map[4] = receiveln(fd);
	map[5] = receiveln(fd);
	map[6] = receiveln(fd);
	map[7] = receiveln(fd);
	map[8] = receiveln(fd);
	map[9] = receiveln(fd);
	return map;
}
