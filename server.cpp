#include <chrono>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <math.h>
#include <algorithm> 
#include <fstream>
#include <sstream>
#include <random> 
#include <typeinfo>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <string.h>

typedef int64_t ll;
#define loop(n) for(int64_t i=0;i<n;++i)
#define loop1(n) for(int64_t i=1;i<=n;++i)
#define loopab(a,b) for(int64_t i=a;i<b;++i)
#define max(a,b) a>b?a:b
#define cout(x) std::cout<<x<<endl;
#define printd(x) printf("%.10f\n",mm);

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

class Holdem{
  public:
    Holdem(int const roomNb):roomNb(roomNb), cards_this_game(52){
      std::iota(cards_this_game.begin(), cards_this_game.end(), 0);
      shuffle (cards_this_game.begin(), cards_this_game.end(), rng);
    };
    std::string setNbPeople(int nb){
      std::ostringstream stream;
      if((nb > 23) or (nb < 2)){
	stream<<"please enter a number between 2 and 23\n";
      } else {
	nbPeople = nb;
	gameOn  = false;
	onTableId2index.clear();
	stream << "we are a "<<nb<<" people game\nPlease ask everyone to join room "<<roomNb<<", and start a new game after everyone join\n";
      }
      return stream.str();
    }
    std::string join(std::string usrname){
      std::ostringstream stream;
      if ( onTableId2index.find(usrname) != onTableId2index.end() ) {
	stream << "you already joined\n";
      } else if (onTableId2index.size() > nbPeople){
	stream<<"you are too late! All " << nbPeople <<" seats are occupied\n";
      } else {
	onTableId2index[usrname] = onTableId2index.size();
	stream<<usrname<<" you have joined successfully ! "<<onTableId2index.size()<<" people joined now\n";
      }
      return stream.str();
    }

    std::string forceNewGame(){
      gameOn = false;
      return newGame();
    }

    std::string newGame(){
      std::ostringstream stream;
      if(onTableId2index.size()<nbPeople){
	stream<<"you need "<<nbPeople<<" people to start the game, while there are only "<<onTableId2index.size()<<" now\n";
      } else if(gameOn){
	stream<<"game is not finished, please don't start a new one now !\n";
      }else{
	shuffle (cards_this_game.begin(), cards_this_game.end(), rng);
	which_round = 1;
	gameOn = true;
	passed_this_round.clear();
	folded_this_round.clear();
	stream<<"game restarted, please ask people to see their cards\n";
      }
      return stream.str();
    }

    std::string people_on_table(){ 
      std::ostringstream stream;
      for (auto const& it : onTableId2index) {
	stream<<it.first<<", ";
      }
      stream<<'\n';
      return stream.str();
    }

    std::string fold(std::string usrname){ 
      std::ostringstream stream;
      if(std::find(folded_this_round.begin(), folded_this_round.end(),usrname)!=folded_this_round.end()){
	stream<<usrname<<", you already folded once, please don't fold twice !\n";
	return stream.str();
      }
      folded_this_round.push_back(usrname);
      if(not (std::find(passed_this_round.begin(), passed_this_round.end(),usrname)!=passed_this_round.end())){
	passed_this_round.push_back(usrname);
      }
      if(nbPeople==1+folded_this_round.size()){
	which_round = 4;
	gameOn = false;
      }
      stream<<usrname<<", you folded !\n";
      return stream.str();
    }

    std::string pass(std::string usrname){
      std::ostringstream stream;
      if(not gameOn){
	stream<<usrname<<", there's no card to be shown !\n";
	return stream.str();
      }
      if(std::find(folded_this_round.begin(), folded_this_round.end(),usrname)!=folded_this_round.end()){
	stream<<usrname<<", you already folded, no need to pass !\n";
	return stream.str();
      }
      if(std::find(passed_this_round.begin(), passed_this_round.end(),usrname)!=passed_this_round.end()){
	stream<<usrname<<", you already passed  once, please don't pass twice !\n";
	return stream.str();
      }
      passed_this_round.push_back(usrname);
      if(nbPeople==passed_this_round.size()){
	if((++which_round)==4){ gameOn = false; }
	passed_this_round = folded_this_round;
      }
      stream<<usrname<<", you passed !\n";
      return stream.str();
    }

    std::string cards(std::string usrname){ 
      std::ostringstream stream;
      if ( onTableId2index.find(usrname) == onTableId2index.end() ) { return stream.str(); }
      auto index = onTableId2index[usrname];
      stream << cards_this_game[index*2-2] << ',' << cards_this_game[index*2-1];
      if(which_round > 1) {
	stream << ';' << cards_this_game[51] << ',' << cards_this_game[50] << ',' << cards_this_game[49];
      }
      if(which_round > 2) {
	stream << ';' << cards_this_game[48];
      }
      if(which_round > 3) {
	stream << ';' << cards_this_game[47];
      }
      return stream.str();
    }

  private:
    int nbPeople;
    std::map<std::string,int> onTableId2index;
    bool gameOn = false;
    int const roomNb;
    std::vector<int> cards_this_game;
    int which_round = 1;
    std::vector<std::string> passed_this_round;
    std::vector<std::string> folded_this_round;
    decltype(std::default_random_engine()) rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
};

std::string error(const char *msg)
{
  perror(msg);
  return msg;
}

std::vector<Holdem> rooms;
std::string create_room(int nb){
  rooms.push_back(Holdem(rooms.size()));
  return rooms.back().setNbPeople(nb);
}

std::map<std::string,int> usrRoomNb;
std::string respondClient(std::string request){
  const auto& splitedRequest = split(request,',');
  if(splitedRequest.size()<2){
    return "invalid command";
  }
  std::string func = splitedRequest[1];
  auto requestSize = splitedRequest.size();
  if(func=="create_room"){ 
    if(requestSize!=3){return error("wrong args");}
    try {
      return create_room(std::stoi(splitedRequest[2]));
    } catch (const std::invalid_argument& ia) {
      return "invalid number of people";
    }
  }
  const auto & usrname = splitedRequest[0];
  if(func=="join`"){ 
    if(requestSize!=3){return error("wrong args");}
    int roomNb;
    try {
      roomNb=std::stoi(splitedRequest[2]);
    } catch (const std::invalid_argument& ia) {
      return "invalid room number";
    }
    if((roomNb>=rooms.size()) or (roomNb<0) ){
      return "Room doesn't exists\n";
    }
    usrRoomNb[usrname] = roomNb;
    return rooms[roomNb].join(usrname);
  }

  if ( usrRoomNb.find(usrname) == usrRoomNb.end() ) {
    return "please join or create a room first\n";
  }
  auto& HoldemInstant = rooms[usrRoomNb[usrname]];

  if(func=="forceNewGame"){
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.forceNewGame();
  }
  if(func=="newGame`"){ 
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.newGame();
  }
  if(func=="people_on_table`"){ 
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.people_on_table();
  }
  if(func=="cards`"){ 
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.cards(usrname);
  }
  if(func=="pass`"){ 
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.pass(usrname);
  }
  if(func=="fold`"){ 
    if(requestSize!=2){return error("wrong args");}
    return HoldemInstant.fold(usrname);
  }
  return "";
}


int main(int argc, char *argv[]){

  int sockfd, newsockfd;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }

  sockfd =  socket(AF_INET, SOCK_STREAM, 0);
  // clear address structure
  memset((char *) &serv_addr, '\0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;  
  serv_addr.sin_addr.s_addr = INADDR_ANY;  
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

  // The accept() call actually accepts an incoming connection
  clilen = sizeof(cli_addr);
  listen(sockfd,50);
  while((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))){
    while(true){
      memset(buffer, '\0', 256);
      read(newsockfd,buffer,255);
      printf("client: %s\n",buffer);
      auto bufferStr = std::string(buffer);
      if(bufferStr.length()==0){
	break;
      }
      std::string clienStr = respondClient(bufferStr);
      std::cout<<clienStr<<'\n';
      send(newsockfd, clienStr.c_str(), clienStr.length(), 0);
    }
    //close(newsockfd);
    shutdown(newsockfd, SHUT_RDWR);
  }

  close(sockfd);
  return 0; 
}
