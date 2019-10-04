#!/usr/bin/bash

IFS=""
ERR="$HOME/lab1_err"

while [ "$command" != "6" ]
do
	echo "Выберите команду:"
	echo "1. Напечатать имя текущего каталога"
	echo "2. Напечатать содержимое текущего каталога"
	echo "3. Вывести текущую дату и время"
	echo "4. Вывести содержимое файла на экран"
	echo "5. Удалить файл"
	echo "6. Выйти из программы"

	read command || break

	case "$command" in
		1) path=$(/bin/pwd 2>> "$ERR")
		   if [ $? -eq 0 ]
         then
           echo "${path##*/}"
         else
           echo 'Не удалось напечатать имя текущего каталога' >&2
       fi
    ;;
		2) ls 2>>"$ERR" || echo 'Не удалось напечатать содержимое текущего каталога' >&2 ;;
		3) date '+%a %h %d %H:%M %Z %Y' 2>>"$ERR" || echo 'Не удалось вывести текущую дату и время' >&2 ;;
		4) echo "Введите название файла:"
			read file_name
			cat "$file_name" 2>>"$ERR" || echo 'Не удалось вывести содержимое файла' >&2 ;;
		5) echo "Введите название файла:"
			read file_name

			if [ ! -f "$file_name" ]
      then
        echo "Такого файла нет" >> "$ERR"; echo "Такого файла нет"  >&2 ;
      else
        echo "rm: remove $file_name (yes/no)?"
			  read confirm
			  if [ "$confirm" = "yes" ]
				  then
				  rm -- "$file_name" 2>>"$ERR" || echo 'Не удалось удалить файл' >&2 ;
			  fi
      fi
    ;;
	esac

done
echo "Bye!"

