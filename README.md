![Let Him Cook](https://raw.githubusercontent.com/USERNAME/REPO/main/LetHimCook.webp)

# LetHimCook!
LetHimCook é uma aplicação desktop desenvolvida em Python e C++, que funciona como uma rede social para partilha de receitas. Os utilizadores podem explorar receitas, criar as suas próprias, seguir outros utilizadores e planear refeições de forma interativa. Este repositório contém o cliente (Python com Flet) e o servidor (C++ com Crow Framework).

Funcionalidades principais:

Autenticação: Login, registo e gestão de contas.
Gestão de receitas: Adicionar, editar, apagar e guardar receitas.
Interações sociais: Seguir utilizadores e receber notificações em tempo real.
Pesquisa: Procurar perfis de utilizadores.
Planeamento de refeições: Organizar receitas para dias da semana.
Configurações: Alteração de tema, idioma e preferências.
Como executar:

Pré-requisitos:

Servidor:

Instalar Crow Framework (via vcpkg):
vcpkg install crow, Visual Studio 2022 e SQLite.

Cliente: 
Python 3.8+ com flet e requests

Servidor (C++):

Abra a solução no Visual Studio 2022.
Configure o vcpkg para incluir o Crow.
Compile e execute o ficheiro server.cpp na porta 18080.

Cliente (Python):
Execute o ficheiro app.py:
python app.py

Requisitos:
Servidor: C++17+, Crow Framework e SQLite.
Cliente: Python 3.8+, Flet e Requests.

Este projeto promove a gestão e partilha de receitas de forma colaborativa.
