#include<iostream>
#include<random>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdint>
#include<chrono>
#include<array>
#include<cstdlib>
#include"connector.h"
using namespace std;

string make_coordinate(int x, int y){
	assert(x>=0 && x<10);
	assert(y>=0 && y<10);
	string str = "";
	str += (x+'A');
	if(y==0) str+="10";
	else str += (9-y+'1');
	return str;
}

using Map = array<string,10>;

#define PUSH_BACK_MOVE(X,Y) ret.push_back(make_coordinate(x,y)+" "+make_coordinate(X,Y))

vector<string> list_valid_moves(Map map){
	vector<string> ret;
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

int main(int argc, char* argv[]){
	int fd;
	int64_t seed;
	if(argc==4){
		fd   = connect(argv[1],atol(argv[2]));
		seed = atoll(argv[3]);
		cout << "using user-provided seed " << seed << endl;
	}else if(argc==3){
		fd   = connect(argv[1],atol(argv[2]));
		seed = std::chrono::system_clock::now().time_since_epoch().count();
		cout << "using time-based seed " << seed << endl;
	}else{
		fd   = connect("localhost",3720);
		seed = std::chrono::system_clock::now().time_since_epoch().count();
		cout << "using time-based seed " << seed << endl;
	}

	if(fd<0){
		cout << "unable to connect\n"; 
		return 1;
	}else cout<<"connected!\n";

	std::mt19937 rng(seed);
	std::uniform_int_distribution<int> cointoss(0,1);

	sendln(fd,cointoss(rng)?"RED":"BLUE");
	(void)receiveln(fd);// we don't care if we got RED or BLUE

	string pieces = "BBBBBBM98877766665555444433333222222221F";
	std::shuffle(pieces.begin(),pieces.end(),rng);
	sendln(fd,pieces);
	
	if(receiveln(fd)!="OK"){
		cout<<"piece setup rejected!\n";
		return 1;
	} else cout<<"piece setup accepted\n";

	while(true){
		// receive opponent move
		string opponent_move = receiveln(fd);

		// receive map
		Map map = receive_map(fd);

		// send response
		if(opponent_move.compare(0,6,"DEFEND")==0){
			vector<string> valid_moves = list_valid_moves(map);
			assert(valid_moves.size()!=0);
			std::uniform_int_distribution<int> select(0,valid_moves.size()-1);
			auto selection = select(rng);
			sendln(fd, "MOVE "+valid_moves[selection]);
		}else if(opponent_move=="WIN"){
			cout<<"hey, I won!"<<endl;
			exit(0);
		}else if(opponent_move=="LOSE"){
			cout<<"aww, I lost!"<<endl;
			exit(0);
		}else{
			cout<<"opponent move: unexpected input: "<<opponent_move<<endl;
			exit(1);
		}

		// receive response
		string response = receiveln(fd);
		if(response.compare(0,6,"ATTACK")==0){
			continue;
		}else if(response=="WIN"){
			cout<<"hey, I won!"<<endl;
			exit(0);
		}else if(response=="LOSE"){
			cout<<"aww, I lost!"<<endl;
			exit(0);
		}else{
			cout<<"server response: unexpected input: "<<response<<endl;	
			exit(1);
		}
	}
};
