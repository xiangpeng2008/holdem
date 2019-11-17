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


//to do n-1 fold can end game
class Holdem{
  public:
    Holdem(int const roomNb):roomNb(roomNb){
      std::iota(cards_this_game.begin(), cards_this_game.end(), 0)
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
      return newgame();
    }

    std::string newgame(){
      std::ostringstream stream;
      if(onTableId2index.size()<nbPeople){
	stream<<"you need "<<nbPeople<<" people to start the game, while there are only "<<onTableId2index.size()<<" now\n";
      } else if(gameOn){
	stream<<"game is not finished, please don't start a new one now !\n";
      }else{
	shuffle (cards_this_game.begin(), cards_this_game.end(), rng);
	role2id.clear();
	int idx=0;
	for (auto const& it : onTableId2index) {
	  role2id[*(cards_this_game[idx])].push_back(it.first);
	  ++idx;
	}
	which_round = 1;
	votesNb = 0;
	gameOn = true;
	passed_this_round.clear();
	folded_this_round.clear();
	stream<<"game restarted, please ask people to see their cards\n";
      }
      return stream.str();
    }
    std::string peopleOnTable(){ 
      std::ostringstream stream;
      for (auto const& it : onTableId2index) {
	stream<<it.first<<", ";
      }
      stream<<'\n';
      return stream.str();
    }
    std::string who(std::string usrname){
      std::ostringstream stream;
      if ( onTableId2index.find(usrname) == onTableId2index.end() ) {
	stream<<usrname<<", you havn't joint yet\n";
	return stream.str();
      } 
      auto role = *(cards_this_game[onTableId2index[usrname]]);
      std::vector<std::string> idsYouKnow;
      for(auto const & it: role.skill){
	idsYouKnow.insert(idsYouKnow.end(), role2id[*it].begin(), role2id[*it].end());
      }
      sort (idsYouKnow.begin(), idsYouKnow.end());
      stream<<usrname<<", you are "<<role<<": "<<role.discription;

      if(role.good){
	for(auto const & it:idsYouKnow){
	  stream<<" "<<it;
	}
      } else {
	for(auto const & it:idsYouKnow){
	  stream<<" "<<it<<":"<<*(cards_this_game[onTableId2index[it]]);
	}
      }
      stream<<'\n';
      return stream.str();
    }

    std::string progress(std::string usrname){ 
      std::ostringstream stream;
      if(votesNb==0){
	loop(5){
	}
      }else{
      }
      return stream.str();
    }

    std::string vote(std::string usrname, bool votePass){
      std::ostringstream stream;
      if(not gameOn){
	stream<<usrname<<", you can't vote anymore, game is already over\n";
	return stream.str();
      }
      if(std::find(votedThisRound.begin(), votedThisRound.end(),usrname)!=votedThisRound.end()){
	stream<<usrname<<", you already voted once this round, please don't vote twice !\n";
	return stream.str();
      }
      votedThisRound.push_back(usrname);
      ++votesNb;
      (votePass?succN:failN)[which_round]++;
      if(votesNb>=[which_round]){
	bool failed = failN[which_round]>=[which_round];
	failRdNbs=failRdNbs+failed;
	stats[which_round]=failed?"✗":"✓";
	which_round++;
	votesNb=0;
	votedThisRound.clear();
	if(failRdNbs==3){
	  gameOn=false;
	  stream<<"Well done ! "<<usrname<<" Evil won !\n";
	  return stream.str();
	}
      }
      stream<<usrname<<", your vote is well registred !\n";
      return stream.str();
    }

    std::string assassinate(std::string usrname,std::string name){
      std::ostringstream stream;
      if(not gameOn){
	stream<<usrname<<", game is already over !\n";
	return stream.str();
      }
      auto const & idAssassin = role2id[*AssassinPtr];
      if(std::find(idAssassin.begin(), idAssassin.end(),usrname)==idAssassin.end()){
	stream<<usrname<<", you are not Assassin, you assassinate what ??? Let me recall you something:\n"<<who(usrname);
      }
      gameOn =false;
      if ( onTableId2index.find(name) == onTableId2index.end() ) {
	stream<<name<<" is not on the table, please verify the name\n";
	return stream.str();
      } else {
	auto const & idMerlin = role2id[*MerlinPtr];
	if(std::find(idMerlin.begin(), idMerlin.end(),name)==idMerlin.end()){
	  stream<<"Oh! "<<usrname<<" you assassinated the wrong person ! Good people won !\n";
	}else{
	  stream<<"Well done ! "<<usrname<<" you assassinated the right person ! Evil won !\n";
	}
      }
      return stream.str();
    }
  private:
    int nbPeople;
    std::map<std::string,int> onTableId2index;
    std::map<Role,std::vector<std::string>,RoleCompare> role2id;
    bool gameOn = false;
    std::vector<int> cards_this_game(52);
    int which_round = 1;
    int votesNb = 0;
    int failRdNbs = 0;
    int const roomNb;
    std::vector<int>  succN;
    std::vector<int>  failN;
    std::vector<std::string> stats;
    std::vector<std::string> votedThisRound;
    decltype(std::default_random_engine()) rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
};

void error(const char *msg)
{
  perror(msg);
  exit(1);
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
    if(requestSize!=3){error("wrong args");}
    return create_room(std::stoi(splitedRequest[2]));
  }
  const auto & usrname = splitedRequest[0];
  if(func=="join"){ 
    if(requestSize!=3){error("wrong args");}
    int roomNb=std::stoi(splitedRequest[2]);
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
    if(requestSize!=2){error("wrong args");}
    return HoldemInstant.forceNewGame();
  }
  if(func=="newgame"){ 
    if(requestSize!=2){error("wrong args");}
    return HoldemInstant.newgame();
  }
  if(func=="peopleOnTable"){ 
    if(requestSize!=2){error("wrong args");}
    return HoldemInstant.peopleOnTable();
  }
  if(func=="who"){ 
    if(requestSize!=2){error("wrong args");}
    return HoldemInstant.who(usrname);
  }
  if(func=="progress"){ 
    if(requestSize!=2){error("wrong args");}
    return HoldemInstant.progress(usrname);
  }
  if(func=="vote"){ 
    if(requestSize!=3){error("wrong args");}
    return HoldemInstant.vote(usrname,splitedRequest[2]=="true");
  }
  if(func=="assassinate"){ 
    if(requestSize!=3){error("wrong args");}
    return HoldemInstant.assassinate(usrname,splitedRequest[2]);
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
