cards:"A23456789XJQK";

last_id:();
onTableId2index:()!();
nb_people:0;

passed_this_round:();
which_round:1;
records:();
gameOn:0b;

set_nb_people:{[nb] 
  nb_people:"J"$nb;
  `gameOn set 0b;
  $[nb_people within (2 10); 
    [ `onTableId2index set ()!();
      :"we are a ",(string nb_people)," people game. \nPlease ask others to join, and start a new game after everyone join";];
    :"please enter of number between 2 and 10"];
  };

progress:{
  $[0;:records;:(string last_id),", only ",(string )," of ",(string records[which_round;`tasks])," people voted this round, please wait them to finish! "];
  };

newgame:{ 
  if[gameOn;
    :"game is not finished, please don't start a new one now !";];
  `cards_this_game set ((nb_people#2),(3 1 1)) _ (neg 5+2*nb_people)?til 52;
  `which_round set 1;
  `gameOn set 1b;
  `passed_this_round set ();
  :"game restarted, please ask people to see their profiles";
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

myCards:{ :cards_this_game[onTableId2index[last_id],nb_people+til which_round-1]; };

vote:{
  if[not gameOn;:(string last_id),", you can vote anymore, game is already over!"];
  if[last_id in passed_this_round;
    :(string last_id),", you already voted once this round, please don't vote twice !";
    ];
  `passed_this_round set passed_this_round,last_id;
  s_f_nb:(`success`fail!`succN`failN)[x];
  records[which_round;s_f_nb]:records[which_round;s_f_nb]+1;
  if[>=records[which_round;];
    records[which_round;`stat]:`$$[records[which_round;`failN]>=records[which_round;`barrer];[`failRdNbs set failRdNbs+1;"✗"];"✓"];
    `which_round set which_round+1;
    `passed_this_round set ();
    if[failRdNbs=3;
      `gameOn set 0b;
      :"Well done ! ",(string last_id)," Evil won !";];
    ];
  :(string last_id),", your vote is well registred !";
  }

