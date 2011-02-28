#! /bin/sh

for i in b.* c.*; do
   ./$i > $i.log
done

