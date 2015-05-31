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

vector<string> list_valid_moves(Map map){
	vector<string> ret;
	for(int y=0; y<10; y+=1){
		assert(map[y].size()==10);
		for(int x=0; x<10; x+=1){
			if((map[y][x]>='0'&&map[y][x]<='9') || map[y][x]=='M'){
				if(x!=0 && (map[y][x-1]=='.' || map[y][x-1]=='#'))
					ret.push_back("MOVE "+make_coordinate(x,y)+" "+make_coordinate(x-1,y));
				if(x!=9 && (map[y][x+1]=='.' || map[y][x+1]=='#'))
					ret.push_back("MOVE "+make_coordinate(x,y)+" "+make_coordinate(x+1,y));
				if(y!=0 && (map[y-1][x]=='.' || map[y-1][x]=='#'))
					ret.push_back("MOVE "+make_coordinate(x,y)+" "+make_coordinate(x,y-1));
				if(y!=9 && (map[y+1][x]=='.' || map[y+1][x]=='#'))
					ret.push_back("MOVE "+make_coordinate(x,y)+" "+make_coordinate(x,y+1));
			}
		}
	}
	cout<<"valid moves:\n";
	for(auto const &x: ret){
		cout<<x<<endl;
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
		string input = receiveln(fd);
		cout <<"input: [" << input << "]\n";
		if(input=="START" || input=="EMPTY" || input.compare(0,6,"ATTACK")==0){
			Map map = receive_map(fd);
			vector<string> valid_moves = list_valid_moves(map);
			std::uniform_int_distribution<int> select(0,valid_moves.size()-1);
			auto selection = select(rng);
			cout<<"selection: "<<selection<<endl;
			sendln(fd, valid_moves[selection]);
			if(receiveln(fd)=="INVALID"){
				cout<<"onoes! I made an invalid move!\n";
				exit(1);
			}
		}else if(input=="WAIT"){
			continue;
		}else if(input==""){
			cout<<"received empty string\n";
			return 1;
		}else if(input=="WIN"){
			cout<<"hurray! I won!"<<endl;
			return 0;
		}else if(input=="LOSE"){
			cout<<"aww I lost!"<<endl;
			return 0;
		}else if(input=="INVALID"){
			cout<<"I broke something D:"<<endl;
			return 1;
		}else{
			cout<<"unexpected input: "<<input<<endl;
			return 1;
		}
	}
};
