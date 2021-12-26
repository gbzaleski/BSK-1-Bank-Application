from django.shortcuts import render
from django.http import HttpResponse
import os
import json
import pam
import io

def index(request):
    context = {
        "deposits": None,
        "credits": None,
        }

    if request.POST:
        if request.POST.get('login', False) and request.POST.get('pass', False):
            login = request.POST['login']
            password = request.POST['pass']

            p = pam.pam()
            if p.authenticate(login, password): #PAM authentication

                creditslist = []
                for filename in os.listdir("../credits"):
                    if filename.split("-")[0] == login:
                        with io.open(f"../credits/{filename}", mode="r", encoding="utf-8") as f:
                            content = f.read()
                            creditslist.append(content)
                
                depositslist = []
                for filename in os.listdir("../deposits"):
                    if filename.split("-")[0] == login:
                        with io.open(f"../deposits/{filename}", mode="r", encoding="utf-8") as f:
                            content = f.read()
                            depositslist.append(content)

                context = {
                    "show": True,
                    "credits": creditslist,
                    "deposits": depositslist,
                    }
            else:
                context = {
                    "notice": "Wrong credentials!"
                    }

    return render(request, 'loginform.html', context)
