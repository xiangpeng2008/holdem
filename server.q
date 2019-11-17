system "p 7780";

last_id:();
onTableId2index:()!();
nb_people:0;
cards_this_game:();

passed_this_round:();
folded_this_round:();

which_round:1;
records:();
gameOn:0b;

set_nb_people:{[nb] 
  `nb_people set "J"$nb;
  `gameOn set 0b;
  $[nb_people within (1 10); 
    [ `onTableId2index set ()!();
      :"we are a ",(string nb_people)," people game. \nPlease ask others to join, and start a new game after everyone join";];
    :"please enter of number between 1 and 10"];
  };

newgame:{ 
  if[gameOn;
    :"game is not finished, please don't start a new one now !";];
  `cards_this_game set (sums 0, (nb_people#2),(3 1)) _ (neg 5+2*nb_people)?til 52;
  `which_round set 1;
  `gameOn set 1b;
  `passed_this_round set ();
  `folded_this_round set ();
  :"game restarted, please ask people to see their cards";
  };

python:{[id;command] 
  `last_id set `$id; 
  show res:get lower command;
  :res;
  };

join:{
  if[last_id in key onTableId2index; :"you already joined";];
  if[((count onTableId2index)>=nb_people); :"you are too late! All ",(string nb_people)," seats are occupied";];
  onTableId2index[last_id]:count onTableId2index;
  :(string last_id)," you have joined successfully ! ", (string count onTableId2index)," people joined now";
  };

people_on_table:{ :string key onTableId2index; };

fold:{[]
  if[last_id in folded_this_round;
    :(string last_id),", you already folded !";
    ];
  `folded_this_round set distinct folded_this_round,last_id;
  `passed_this_round set distinct passed_this_round,last_id;
  if[nb_people=1+count folded_this_round;
    `which_round set 4;
    `gameOn set 0b;];
  :(string last_id),", you folded !";
  }

cards:{ :cards_this_game[onTableId2index[last_id],nb_people+til which_round-1]; };

reset_passed_this_round:{[] `passed_this_round set folded_this_round; };

pass:{
  if[not gameOn;:(string last_id),", there's no card to be shown!"];
  if[last_id in folded_this_round;
    :(string last_id),", you already folded !";
    ];
  if[last_id in passed_this_round;
    :(string last_id),", you already passed !";
    ];
  `passed_this_round set passed_this_round,last_id;
  if[nb_people=count passed_this_round;
    `which_round set which_round+1;
    if[which_round=4;`gameOn set 0b;];
    reset_passed_this_round`;
    ];
  :(string last_id),", you passed !";
  }

