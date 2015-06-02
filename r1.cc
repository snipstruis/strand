#include<iostream>
#include<random>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdint>
#include<chrono>
#include<array>
#include<cstdlib>

#include "r.h"
#include"connector.h"
using namespace std;

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
