#!/bin/bash

groupadd customers # stworzenie grupy klientów
groupadd officers # stworzenie grupy pracowników

filename=$1 #filename='uzytkownicy.txt'

# Dodanie każdego uzytkownika do bazy
while IFS= read -r line 
do 
    user_data=($line)
    username=${user_data[0]}
    group=${user_data[1]}
    name=${user_data[2]}
    lastname=${user_data[3]}

    if [ "$group" == "client" ]; then
        useradd "$username"
        usermod -a -G customers "$username"
    else
        useradd "$username"
        usermod -a -G officers "$username"
    fi

    # Każdy ma hasło równe swojemu loginowi
    echo "${username}:${username}" | chpasswd

done < "$filename"

# Ustawienie odpowiednich dostępów każdej grupie do każdego folderu
setfacl -m g:customers:r-- credits
setfacl -m g:officers:rwx credits

setfacl -m g:customers:r-- deposits
setfacl -m g:officers:rwx deposits