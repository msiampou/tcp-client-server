#!/bin/bash

#check if directory exists
if [ ! -d "$1" ]; then
  echo "Directory doesn't exists"
  exit 1;
fi

#check if test file exists
if [ ! -f "$2" ]; then
  echo "File doesn't exists"
  exit 1;
fi

#check if w and p are integers
if [[ ! $3 =~ ^[0-9]+$ ]] || [[ ! $3 -gt 0 ]] || [[ ! $4 =~ ^[0-9]+$ ]] || [[ ! $4 -gt 0 ]]; then
    echo "Argument '$3' and '$4' are not integers"
    exit 1;
fi

if [ "$3" == "2" ] || [ "$3" == "1" ] || [ "$4" == "2" ] || [ "$4" == "1" ]; then
    echo "Argument '$3' or '$4' are do not have proper values"
    exit 1;
fi

#check if lines of txt file are more than 10.000
a=$(wc $2)
lines=$(echo $a|cut -d' ' -f1)
if [ "10000" -gt "$lines" ]; then
    echo "Small txt file"
    exit 1;
fi

#create directories
for ((dir=0;dir<$3;dir++))
do
    mkdir directory/site"$dir"
done

count=0
links=()
for ((dir=0;dir<$3;dir++))
do
    for ((pg=0;pg<$4;pg++))
    do

        #create pages' names and store them into a link
        r=$RANDOM
        EL[$count]="../site"$dir"/page"$dir"_"$r".html"
        IL[$count]="page"$dir"_"$r".html"
    	links[$count]="directory/site"$dir"/page"$dir"_"$r".html"
        let "count += 1"

    done
done

count=0;

start=0
let "end=$4-1"
let "MAXCOUNT = $3*$4 -1"
for ((dir=0;dir<$3;dir++))
do
    echo ...
    echo Creating web site $dir ...
    arr=() #keeps final links
    for ((pg=0;pg<$4;pg++))
    do

        # 1 < k < lines-2000
        k=$(($RANDOM % $(($lines-2000)) +1));

        # 1000 < m < 2000
        m=$(($RANDOM % 1000 + 1000));

        #select f internal links
        let "f = $4/2 + 1"

        #select q internal links
        let "q = $3/2 + 1"

        ####  HTML HEADERS  ####
        text="<!DOCTYPE html>"
        echo $text >> ${links[$count]}
        text="<html>"
        echo $text >> ${links[$count]}
        text="  <body>"
        echo $text >> ${links[$count]}


    ### CREATING INTERNAL AND EXTERNAL LINKS AND COPY THEM TO arr() ###

              ######   INTERNAL LINKS   ######
        #avoid using the same page for internal links
        #generate f+1 random numbers and then delete
        #the entry that contains the number of this page
        intlinks=($(shuf -i $start-$end -n $(($f+1))))
        for ((i=0;i<${#intlinks[@]};i++)); do
        	if [[ ${intlinks[$i]} == $count ]]; then
                intlinks=( "${intlinks[@]:0:$i}" "${intlinks[@]:$((i + 1))}" )
            fi
        done

        pos=0
        for i in "${intlinks[@]}";
        do
            arr[$pos]=${IL["$i"]}
            let "pos+=1"
        done

              ######   EXTERNAL LINKS   ######
        #avoid using the same site for external links
        #generate q+pages random numbers and then delete
        #the entries where entry/pages = curr site
        extlinks=($(shuf -i 0-$MAXCOUNT -n $(($q+$4))))
        for ((i=0;i<$(($q+$4));i++)); do
            let "val = ${extlinks[$i]} / $3"
        	if [[ $val == $dir ]]; then
                unset 'extlinks[$i]'
            fi
        done

        flag=0
        for i in "${extlinks[@]}";
        do
            if [[ $flag < $q ]]; then
                arr[$pos]=${EL["$i"]}
                let "pos+=1"
            fi
            let "flag+=1"
        done

        let "div = $m/($f+$q)"
        let "mod = $m%($f+$q)"

        echo Creating page ${links[$count]} with $(($pos*$div)) lines starting at line $k ...

        ln=()
        for ((i=0;i<$pos;i++)); do
        	ln[$i]=$div
        done
        for ((i=0;i<$mod;i++)); do
        	let "ln[$i]+=1"
        done

        for ((i=0;i<$pos;i++));
        do
            s=$k
            e=$(($k+${ln[$i]}))
            #copy k+floor lines from text_file.txt
            sed -n -e "$s,$e p" -e "$e q" text_file.txt >> ${links[$count]}

            #add a link at the end of each sentance
            if [[ $i < $f ]]; then
                echo "Adding internal link"
            else
                echo "Adding external link"
            fi
            echo "<a href=${arr[$i]}>link$i_text</a>" >> ${links[$count]}

            let "s=e+1"
            let "e=s+k"
        done

        ####  HTML HEADERS  ####
        text="  </body>"
        echo $text >> ${links[$count]}
        text="</html>"
        echo $text >> ${links[$count]}

        let "count += 1"
    done
    let "start=end+1"
    let "end = end + $4"
done

echo ...
echo All pages have at least 1 incoming link
echo Done.
