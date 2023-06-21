#include "async.h"

int main() {
    auto id3 = connect(3);
   // receive(id3);

//    receive1("cmd1", 4, id3);
//    receive1("cmd2", 4, id3);
//    receive1("cmd3", 4, id3);
//    receive1("", 0, id3);
//
//    receive1("{", 1, id3);
//    receive1("cmd1", 4, id3);
//    receive1("cmd2", 4, id3);
//    receive1("cmd3", 4, id3);
//    receive1("", 0, id3);
//    receive1("}", 1, id3);
//
//    receive1("{", 1, id3);
//    receive1("cmd2", 4, id3);

   // std::this_thread::sleep_for(std::chrono::seconds(1));
    receive("cmd1", 4, id3);
    receive("cmd2", 4, id3);
    receive("{", 1, id3);
    receive("cmd3", 4, id3);
    receive("cmd4", 4, id3);
    receive("}", 1, id3);

    auto id4 = connect(4);
    receive("cmd1", 4, id4);
    receive("cmd2", 4, id4);
    receive("{", 1, id4);
    receive("cmd3", 4, id4);
    receive("cmd4", 4, id4);
    receive("}", 1, id4);

    receive("{", 1, id3);
    receive("cmd5", 4, id3);
    receive("cmd6", 4, id3);
    receive("{", 1, id3);
    receive("cmd7", 4, id3);
    receive("cmd8", 4, id3);
    receive("}", 1, id3);
    receive("cmd9", 4, id3);
    receive("}", 1, id3);
    receive("{", 1, id3);
    receive("cmd10", 4, id3);
    receive("cmd11", 4, id3);

    receive("{", 1, id4);
    receive("cmd5", 4, id4);
    receive("cmd6", 4, id4);
    receive("{", 1, id4);
    receive("cmd7", 4, id4);
    receive("cmd8", 4, id4);
    receive("}", 1, id4);
    receive("cmd9", 4, id4);
    receive("}", 1, id4);
    receive("{", 1, id4);
    receive("cmd10", 4, id4);
    receive("cmd11", 4, id4);



    disconnect(id3);
    disconnect(id4);
   // receive(id4);
 //   disconnect(id4);


}



/*
 5
cmd1
cmd2
cmd3
cmd4
cmd5

 18
cmd1
cmd2
{
cmd3
cmd4
}
{
cmd5
cmd6
{
cmd7
cmd8
}
cmd9
}
{
cmd10
cmd11


 */
