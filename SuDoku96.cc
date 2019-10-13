#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <math.h>
#include <algorithm> 
#include <fstream>

using namespace std;
typedef int64_t ll;
#define loop(n) for(int64_t i=0;i<n;++i)
#define loop1(n) for(int64_t i=1;i<=n;++i)
#define loopab(a,b) for(int64_t i=a;i<b;++i)
#define max(a,b) a>b?a:b
#define cout(x) cout<<x<<endl;
#define printd(x) printf("%.10f\n",mm);

const double EulerConstant = exp(1.0);

template <typename T> int64_t sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

template <typename T> 
T gcd(T a, T b) {
  return b == 0 ? a : gcd(b, a % b);
}

int64_t count_2m_it = 0;
ll mod=1e8;
ll limitN = 120000;
//ll limitN = 1000;
vector<ll> primes;
vector<ll> rads(limitN+1,1);
bool abcHit(ll a, ll b, ll c){
  if(rads[a]*rads[b]*rads[c]>=c){
    return false;
  }
  if(gcd(rads[a],rads[b])!=1){
    return false;
  }
  //cout<<a<<":"<<rads[a]<<' '<<b<<":"<<rads[b]<<' '<<c<<":"<<rads[c]<<endl;
  //cout<<a<<' '<<b<<' '<<c<<endl;
  return true;
}
typedef vector<vector<int>> ct;
class Sudoku{
  public:
    Sudoku(ct const & sudo):sudo(sudo){
      cout<<"sudo initial"<<endl;
      init();
      while(progressed){
	solve();
      }
      //print();
    };
    ct sudo;
    void debugPrint(){
      int n = 0;
      for(auto const & it:unsolvedPos){
	++n;
	cout<<it.first<<it.second<<' ';
	for(auto it:possibles_values[it]){
	  cout<<it;
	}
	cout<<endl;
	if(n>5){
	  //break;
	}
      }
    }
  private:
    bool progressed = true;

    vector<ct> guessNodesSudo;
    vector<pair<pair<int,int>,int>> guessNodesVal;

    map<pair<int,int>,set<int>> possibles_values;
    vector<pair<int,int>> unsolvedPos;


    void update(int i,int j,int k){
      sudo[i][j] = k;
      reducePossibles_values(i,j,k);
      //cout<<"update "<<i<<j<<' '<<k<<endl;
      //print();
      progressed = true;
    }
    void subSolveInverse(vector<pair<int,int>>& vp){
      for(auto it=vp.begin();it!=vp.end();){
	if(sudo[it->first][it->second]!=0){ it = vp.erase(it); 
	} else{ ++it; }
      }
      map<int,int> mii;
      for(const auto& it:vp){
	for(const auto & it_sub: possibles_values[it]){
	  ++mii[it_sub];
	}
      }
      for(const auto& it:mii){
	if(it.second==1){
	  for(const auto& itVp:vp){
	    auto container = possibles_values[itVp];
	    if(container.find(it.first) != container.end()){
	      //print();
	      update(itVp.first,itVp.second,it.first);
	      unsolvedPos.erase(std::remove(unsolvedPos.begin(), unsolvedPos.end(), itVp), unsolvedPos.end());
	      //cout<<"subSolveInverse "<<itVp.first<<' '<<itVp.second<<' '<<it.first<<endl;
	      break;
	    }
	  }
	}
      }
    }

    void solve_inverse(){
      loop(9){
	vector<pair<int,int>> vp;
	for(int j=0;j<9;++j){ 
	  vp.push_back(make_pair(i,j));
	}
	subSolveInverse(vp);

	vp.clear();
	for(int j=0;j<9;++j){ 
	  vp.push_back(make_pair(j,i));
	}
	subSolveInverse(vp);

	vp.clear();
	for(int j=0;j<9;++j){ 
	  vp.push_back(make_pair(i/3*3+j/3,i%3*3+j%3));
	}
	subSolveInverse(vp);
      }
    }

    void solve_forward(){
      for(auto it=unsolvedPos.begin();it!=unsolvedPos.end();){
	auto set_vs = possibles_values[*it];
	if(set_vs.size()==0){
	  revertToPrevNode();
	  return;
	} else if (set_vs.size()==1){
	  update(it->first,it->second,*(set_vs.begin()));
	  it = unsolvedPos.erase(it);
	} else {
	  ++it;
	}
      }
    }
    void revertToPrevNode(){
      //debugPrint();
      cout<<"guess again\n";
      if(guessNodesSudo.empty()){
	cout<<"abort\n";
	abort();
      }
      //cout<<"print before guess again\n";
      //print();
      sudo = guessNodesSudo.back();
      //cout<<"print when guess again\n";
      //print();
      init();
      auto & lastGuess = guessNodesVal.back();
      if(possibles_values[lastGuess.first].size()>lastGuess.second+1){
	++lastGuess.second;
	update(
	    lastGuess.first.first,
	    lastGuess.first.second,
	    *next(possibles_values[lastGuess.first].begin(),lastGuess.second)
	    );
	unsolvedPos.erase(std::remove(unsolvedPos.begin(), unsolvedPos.end(), lastGuess.first), unsolvedPos.end());
      } else {
	guessNodesSudo.pop_back();
	guessNodesVal.pop_back();
	cout<<"guess backward\n";
	revertToPrevNode();
      }
    }
    void solve(){
      progressed = false;
      solve_forward();
      solve_inverse();

      if(not progressed){
	if(not unsolvedPos.empty()){
	  //debugPrint();
	  cout<<"guess forward\n";
	  guessNodesSudo.push_back(sudo);
	  auto guessPair = *(unsolvedPos.begin());
	  auto guessVal = *(possibles_values[guessPair].begin());
	  guessNodesVal.push_back(make_pair(guessPair,0));
	  update(guessPair.first,guessPair.second,guessVal);
	  unsolvedPos.erase(std::remove(unsolvedPos.begin(), unsolvedPos.end(), guessPair), unsolvedPos.end());
	  //print();
	}
      }
      //debugPrint();
    }
    void reducePossibles_values(){
      for(auto const & it:unsolvedPos){
	auto & set_vs = possibles_values[it];
	auto startI = it.first/3*3;
	auto startJ = it.second/3*3;
	loop(9){
	  set_vs.erase(sudo[it.first][i]);
	  set_vs.erase(sudo[i][it.second]);
	  set_vs.erase(sudo[startI+i%3][startJ+i/3]);
	}
      }
    }
    void reduce1value(const pair<int,int> & p, int v){
      possibles_values[p].erase(v);
    };
    void reducePossibles_values(int i, int j, int v){
      auto startI = i/3*3;
      auto startJ = j/3*3;
      for(int k=0;k<9;++k){
	reduce1value(make_pair(i,k),v);
	reduce1value(make_pair(k,j),v);
	reduce1value(make_pair(startI+k%3,startJ+k/3),v);
      }
    }
    void init(){
      unsolvedPos.clear();
      for(int i=0;i<9;++i){
	for(int j=0;j<9;++j){
	  if(sudo[i][j]==0){
	    auto p = make_pair(i,j);
	    possibles_values[p]={1,2,3,4,5,6,7,8,9};
	    unsolvedPos.push_back(p);
	  }
	}
      }
      reducePossibles_values();
    }
    void print(){
      cout<<"print\n";
      for(int i=0;i<9;++i){
	for(int j=0;j<9;++j){
	  auto v = char(48+sudo[i][j]);
	  cout<<('0'==v?'*':v);
	}
	cout<<endl;
      }
    }
};

int main(){
  std::ifstream sudoFile("/Users/xiangpeng/Documents/cpp/euler_project/files/p096_sudoku.txt");
  string str;
  //loop(11){ sudoFile >> str;}
  //loop(55){ sudoFile >> str;}
  int res = 0;
  loop(50){
    sudoFile >> str;
    sudoFile >> str;
    ct datas;
    loop(9){
      sudoFile >> str;
      std::vector<int> data(str.begin(), str.end());
      for(auto & it:data){
	it-=48;
      }
      datas.push_back(data);
    }
    Sudoku S(datas);
    //S.debugPrint();
    res +=100*S.sudo[0][0]+10*S.sudo[0][1]+S.sudo[0][2];
  }
  cout<<res<<endl;
  return 0;
}



