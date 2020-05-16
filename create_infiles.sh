#!/bin/bash

diseasesFile=$1
countriesFile=$2
input_dir=$3
numFilesPerDirectory=$4
numRecordsPerFile=$5
nameArray=(Albert Alisha Andrea Bobby Brian Bruce Cathy Colin Diana Danny Earl Elizabeth Gabriella Grace Jack Jimmy Joan Katherine)
lenName=${#nameArray[@]}
surnameArray=(Ayala Ayers Bailey Baird Baker Baldwin Ball Ballard Banks Cunningham Curry Curtis Dalton Daniel Frost Fry Frye Fuentes Fuller Gaines Gallagher Gallegos)
lenSur=${#surnameArray}

if [ ! -f $diseasesFile ] || [ ! -f $countriesFile ] || [ "$numFilesPerDirectory" -lt "0" ] || [ "$numFilesPerDirectory" -eq "0" ] || [ "$numRecordsPerFile" -lt "0" ] || [ "$numRecordsPerFile" -eq "0" ]; then
	echo "exit"
	exit
fi

day=30
month=12
year=2020
year_floor=1990
idnum=0

mkdir $input_dir

num=1
while read entry; do

	mkdir ./$input_dir/$entry
	# chmod 755 ./$input_dir/$entry

	if [ -e $"temp.txt" ] ; then		# every country has its own temp file
		rm temp.txt
	fi
	
	for ((i=1;i<=numFilesPerDirectory;i++)) do
		date=$RANDOM
		let "date %= $day"
		((date+=$num))
		mon=$RANDOM
		let "mon %= $month"
		((mon+=$num))
		date+="-$mon-"
		y=0
		while [ "$y" -le $year_floor ]
		do 
			y=$RANDOM
			let "y %= $year"
			((y+=$num))
		done
		date+=$y

		touch ./$input_dir/$entry/$date 
		j=1

		patients=$RANDOM
		let "patients %= (numFilesPerDirectory/2)"
		((patients+=1))

		for((k=0;k<patients;k++))
		do
			if [ -e $"temp.txt" ] ; then
				record=$(head -n 1 $"temp.txt") 
				echo "${record/ENTER/"EXIT"}" >> ./$input_dir/$entry/$date
				sed -i '1d' $"temp.txt"
			fi
		done
		((j+=$patients))
		
		for ((j;j<=numFilesPerDirectory;j++))
		do
			record="$idnum"
			((idnum+=$num))
			state=$RANDOM
			let "state %= 5"
			if [ $state == 0 ]; then
				record+=" EXIT"
			else
				record+=" ENTER"
			fi
			name=$RANDOM
			let "name %= lenName"
			name=${nameArray[name]}
			record+=" $name"
			lastName=$RANDOM
			let "lastName %= lenName"
			lastName=${surnameArray[lastName]}
			record+=" $lastName"
			disease=$(shuf -n 1 $diseasesFile)
			record+=" $disease"
			age=$RANDOM
			let "age %= 120"
			((age+=$num))
			record+=" $age"
			echo "$record" >> ./$input_dir/$entry/$date
			chmod 755 ./$input_dir/$entry/$date
			echo "$record" >> temp.txt

		done
	done

done < $countriesFile
rm temp.txt
numWorkers=10
bufferSize=512
# make clean
# make
# ./diseaseAggregator -w $numWorkers -b $bufferSize -i $input_dir
