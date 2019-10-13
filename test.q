//test
nbPeople:6;
peoples:("xiangpeng";"huoda";"yijia"; "qikai";"songong";"sunyue")
python["xiangpeng";"set_nb_people \"",(string nbPeople),"\""];
python[;"join`"]'[peoples];
python["xiangpeng";"people_on_table`"];
python["xiangpeng";"newgame`"];
python[;"who`"]'[peoples];
python["xiangpeng";"records"];
python[;"vote `success"]'[peoples];
python["xiangpeng";"progress`"];
python[;"vote `fail"]'[peoples];
python["xiangpeng";"progress`"];

python[;"vote `fail"]'[4#peoples];
python[;"vote `success"]'[4#peoples];

python["xiangpeng";"progress`"];

python["xiangpeng";"assassinate \"xiangpeng\""];
python["qikai";"assassinate \"xiangpeng\""];

