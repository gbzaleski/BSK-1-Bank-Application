# syntax=docker/dockerfile:1
FROM ubuntu:18.04
COPY . /
RUN apt-get update && apt-get install -y make gcc acl sudo libpam0g-dev
RUN apt-get install -y python3
RUN apt-get install -y python3-pip
RUN pip3 install django
RUN pip3 install python-pam
RUN ls -a
RUN bash skrypt_bank.sh uzytkownicy.txt
RUN gcc -o bank_officer_app bank.c -lpam -lpam_misc
RUN apt-get install -y openssh-server
CMD ./autostart.sh && /etc/init.d/ssh start && cd bank && python3 manage.py runserver 0.0.0.0:8000
