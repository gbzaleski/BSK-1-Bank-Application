
# How to use
Build docker:\
```docker build -t bskbig .```

Run docker:\
```docker run --name bigbsk -verbose -p 8000:8000 bskbig```

Client app available at:\
```localhost:8000```

Every user has the starting password the same as username.

Officers can access the system via:\
```docker exec -it bigbsk ./bank_officer_app```\
or via SSH (use officer's username):\
```ssh officer@172.17.0.2 ```


Whole project is written in English language, except for - as requested - 'procent' (meaning per cent / percentage) and Polish comment in codes.\
Project was tested with valgrind for leaks and errors.

# Wymagany opis plików
```
/bank - aplikacja Django do wyświetlania informacji dla klientów
/credits - kredyty nazwane wg wzoru {username}-{index}.txt
/deposits - lokaty nazwane wg wzoru {username}-{index}.txt
autostart.sh - konfiguracja SSH dla Dockera
bank.c - kod w C aplikacji pracownika
firewall.nft - reguły ściany ogniowej
uzytkownicy.txt - spis użytkowników banku
skrypt_bank.sh - skrypt tworzący konta użytkowników banku na podstawie pliku uzytkownicy.txt
Dockerfile - plik konfiguracji Dockera
```