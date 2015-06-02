#include<iostream>
#include<random>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdint>
#include<chrono>
#include<array>

#include "r.h"
#include"connector.h"
using namespace std;


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
bool read_bin(char const * const filename, uint8_t* out, int const size){
	assert(filename!=NULL); assert(out!=NULL); assert(size>0);
	int fd = open(filename,O_RDONLY);
	struct stat st;
	fstat(fd, &st);
	if(size!=st.st_size){
		close(fd);
		return false;
	}
	int bytes_left = size;
	int bytes_read = 0;
	while(bytes_left>0){
		int w = read(fd, out+bytes_read, bytes_left);
		if(w<=0){
			close(fd);
			return false;
		}else{
			bytes_left -= w;
			bytes_read += w;
		}
	}
	close(fd);
	return true;
}

void place_piece(array<char,40> &map, uint8_t const * const probability, 
		std::mt19937 &rng, std::string pieces){
	std::shuffle(pieces.begin(),pieces.end(),rng);
	for(auto const &c:pieces){
		int max=0;
		for(int i=0; i<40; i+=1){
			if(map[i]==0)
				max+=probability[i];
		}
		std::uniform_int_distribution<int> dist(0,max-1);
		int random = dist(rng);
		int sum=0;
		for(int i=0; i<40; i+=1){
			if(map[i]==0){
				sum+=probability[i];
				if(sum>random){
					map[i]=c;
					break;
				}
			}
		}
	}
}

array<char,40> populate_map(std::mt19937 &rng){
	array<char,40> map;
	map.fill(0);
	uint8_t bin[200];
	if(!read_bin("setup.bin",bin,sizeof(bin))){
		cout<<"failed to read setup.bin\n";
		exit(1);
	}
	place_piece(map,bin,rng,"F");
	place_piece(map,bin+40,rng,"BBBBBB");
	place_piece(map,bin+80,rng,"M988");
	place_piece(map,bin+120,rng,"777666655554444");
	place_piece(map,bin+160,rng,"333331");
	for(auto &m:map) if(m==0) m='2';
	return map;
}

int main(int argc, char* argv[]){
	init_data init = initialize(argc,argv);
	int fd = init.fd;

	std::mt19937 rng(init.seed);
	std::uniform_int_distribution<int> cointoss(0,1);

	sendln(fd,cointoss(rng)?"RED":"BLUE");
	(void)receiveln(fd);// we don't care if we got RED or BLUE

	std::array<char,40> map = populate_map(rng);
	sendln(fd,std::string(map.begin(),map.end()));
	
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
