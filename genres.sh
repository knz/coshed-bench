#! /bin/sh
for l in *.log; do  
  h=$(head -n1 <$l) 
  t=$(tail -n1 <$l |cut -d'	' -f8- )
  printf '%s\t%s\n' "$h" "$t"; 
done | sed -e 's/  */	/g' >results.txt
