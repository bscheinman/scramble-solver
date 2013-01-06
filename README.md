boggle-solver
=============

NOTE: requires trie and queue collections from c-collections repository

Right now we don't eliminate duplicate words, so suggested usage is something like:
./scramble /usr/share/dict/words board.txt | sort -k2 -nr | uniq
