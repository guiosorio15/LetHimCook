#include "crow.h"
#include <sqlite3.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <string>
#include <random>
#include <memory>
#include <fstream>
#include <regex>
#include "base64.h"

// DataBase Class
class Database {
private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db;

public:
    Database(const std::string& db_name) : db(nullptr, &sqlite3_close)
    {
        sqlite3* raw_db = nullptr;
        if (sqlite3_open(db_name.c_str(), &raw_db)) throw std::runtime_error("Error opening Database: " + std::string(sqlite3_errmsg(raw_db)));

        db.reset(raw_db);
        criarTabelaUtilizador();
        criarTabelaReceitas();
        criarTabelaReceitasSalvas();
        criarTabelaSeguidores();
        criarTabelaNotifs();
    }

    void criarTabelaUtilizador()
    {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "profile_pic TEXT, "
            "banner TEXT, "
            "username TEXT NOT NULL UNIQUE, "
            "password TEXT NOT NULL);";
        char* mensagemErro = nullptr;
        int resultado = sqlite3_exec(db.get(), sql, nullptr, nullptr, &mensagemErro);

        if (resultado != SQLITE_OK)
        {
            std::string erro(mensagemErro);
            sqlite3_free(mensagemErro);
            throw std::runtime_error("Error creating table: " + erro);
        }
    }

    bool utilizadorExiste(const std::string& username)
    {
        const std::string sql = "SELECT 1 FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        bool existe = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        return existe;
    }

    bool senhaCorreta(const std::string& username, const std::string& password)
    {
        const std::string sql = "SELECT 1 FROM users WHERE username = ? AND password = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

        bool correta = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        return correta;
    }

    bool inserirUtilizador(int id, const std::string& username, const std::string& password)
    {
        const std::string sql = "INSERT INTO users (id, username, password) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar inserção: " + std::string(sqlite3_errmsg(db.get())));


        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao inserir utilizador: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool idUtilizadorExiste(int id) {
        const std::string sql = "SELECT COUNT(*) FROM users WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, id);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);
        return count > 0;
    }

    bool apagarUtilizador(const std::string& username, const std::string& password)
    {
        const std::string sql = "DELETE FROM users WHERE username = ? AND password = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar exclusão: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

        bool sucesso = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return sucesso;
    }

    int obterUtilizadorId(const std::string& username)
    {
        const std::string sql = "SELECT id FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        int user_id = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW) user_id = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);
        return user_id;
    }

    std::string obterUsername(int user_id)
    {
        const std::string sql = "SELECT username FROM users WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);

        std::string username;
        if (sqlite3_step(stmt) == SQLITE_ROW)  username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        sqlite3_finalize(stmt);
        return username;
    }

    std::vector<int> procurarUtilizadores(std::string search)
    {
        //É PRECISO METER ISTO PARA PROCURAR COMO SUBSTRING
        search = "%" + search + "%";

        const std::string sql = "SELECT id FROM users WHERE username LIKE ?;";
        sqlite3_stmt* stmt;
        std::vector<int> ids;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));
        }

        if (sqlite3_bind_text(stmt, 1, search.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao associar parâmetro: " + std::string(sqlite3_errmsg(db.get())));
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0); 
            ids.push_back(id); 
        }

        sqlite3_finalize(stmt);

        return ids;
    }

    bool atualizarPfp(std::string imagePath, int id)
    {
        const std::string sql = "UPDATE users SET profile_pic = ? WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar atualização de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, imagePath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao atualizar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool atualizarBanner(std::string imagePath, int id)
    {
        const std::string sql = "UPDATE users SET banner = ? WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar atualização de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, imagePath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao atualizar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    void criarTabelaReceitas()
    {
        const char* sql = "CREATE TABLE IF NOT EXISTS receitas ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "titulo TEXT NOT NULL, "
            "ingredientes TEXT NOT NULL, "
            "passos TEXT NOT NULL, "
            "user_id INTEGER NOT NULL, "
            "image TEXT, "
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE);";
        char* mensagemErro = nullptr;
        int resultado = sqlite3_exec(db.get(), sql, nullptr, nullptr, &mensagemErro);

        if (resultado != SQLITE_OK)
        {
            std::string erro(mensagemErro);
            sqlite3_free(mensagemErro);
            throw std::runtime_error("Erro ao criar tabela de receitas: " + erro);
        }
    }

    bool inserirReceita(int id, const std::string& titulo, const std::string& ingredientes, const std::string& passos, int user_id)
    {
        const std::string sql = "INSERT INTO receitas (id, titulo, ingredientes, passos, user_id) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar inserção de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, titulo.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, ingredientes.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, passos.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, user_id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao inserir receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool editarReceita(int id, const std::string& titulo, const std::string& ingredientes, const std::string& passos)
    {
        const std::string sql = "UPDATE receitas SET titulo = ?, ingredientes = ?, passos = ? WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar atualização de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, titulo.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ingredientes.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, passos.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao atualizar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool apagarReceita(int recipe_id, int user_id)
    {
        const std::string sql = "DELETE FROM receitas WHERE id = ? AND user_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar remoção de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, recipe_id);
        sqlite3_bind_int(stmt, 2, user_id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao apagar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool idReceitaExiste(int id)
    {
        const std::string sql = "SELECT COUNT(*) FROM receitas WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, id);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count > 0;
    }

    int obterUtilizadorPorReceita(int recipe_id)
    {
        const std::string sql = "SELECT user_id FROM recipes WHERE recipe_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, recipe_id);
        sqlite3_step(stmt);

        int user_id = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);
        return user_id;
    }

    std::vector<int> obterReceitasPorUtilizador(int user_id)
    {
        const std::string sql = "SELECT id FROM receitas WHERE user_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);

        std::vector<int> receita_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW) receita_ids.push_back(sqlite3_column_int(stmt, 0));

        sqlite3_finalize(stmt);
        return receita_ids;
    }

    std::tuple<std::string, std::string, std::string> obterReceitaPorId(int id)
    {
        const std::string sql = "SELECT titulo, ingredientes, passos FROM receitas WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, id);

        std::string titulo, ingredientes, passos;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            titulo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            ingredientes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            passos = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }
        else
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro: Receita com ID " + std::to_string(id) + " não encontrada.");
        }

        sqlite3_finalize(stmt);
        return std::make_tuple(titulo, ingredientes, passos);
    }

    void criarTabelaReceitasSalvas()
    {
        const char* sql = "CREATE TABLE IF NOT EXISTS saved_recipes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "recipe_id INTEGER NOT NULL, "
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY(recipe_id) REFERENCES receitas(id) ON DELETE CASCADE);";
        char* mensagemErro = nullptr;
        int resultado = sqlite3_exec(db.get(), sql, nullptr, nullptr, &mensagemErro);

        if (resultado != SQLITE_OK)
        {
            std::string erro(mensagemErro);
            sqlite3_free(mensagemErro);
            throw std::runtime_error("Erro ao criar tabela de receitas salvas: " + erro);
        }
    }

    bool salvarReceita(int user_id, int recipe_id)
    {
        const std::string sql = "INSERT INTO saved_recipes (user_id, recipe_id) VALUES (?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar inserção de receita salva: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, recipe_id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao salvar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    std::vector<int> obterReceitasSalvas(int user_id)
    {
        const std::string sql = "SELECT recipe_id FROM saved_recipes WHERE user_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);

        std::vector<int> recipe_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            recipe_ids.push_back(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return recipe_ids;
    }

    std::vector<int> obterRecipeSavers(int recipe_id)
    {
        const std::string sql = "SELECT user_id FROM saved_recipes WHERE recipe_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, recipe_id);

        std::vector<int> recipe_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            recipe_ids.push_back(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return recipe_ids;
    }

    bool atualizarImagemReceita(std::string imagePath, int id)
    {
        const std::string sql = "UPDATE receitas SET image = ? WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar atualização de receita: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_text(stmt, 1, imagePath.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao atualizar receita: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    void criarTabelaSeguidores()
    {
        const char* sql =
            "CREATE TABLE IF NOT EXISTS followers ("
            "follower_id INTEGER NOT NULL, "
            "followed_id INTEGER NOT NULL, "
            "PRIMARY KEY (follower_id, followed_id), "
            "FOREIGN KEY(followed_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY(follower_id) REFERENCES users(id) ON DELETE CASCADE);";

        char* mensagemErro = nullptr;
        int resultado = sqlite3_exec(db.get(), sql, nullptr, nullptr, &mensagemErro);

        if (resultado != SQLITE_OK)
        {
            std::string erro(mensagemErro);
            sqlite3_free(mensagemErro);
            throw std::runtime_error("Erro ao criar tabela de seguidores: " + erro);
        }
    }

    bool adicionarSeguidor(int follower_id, int followed_id)
    {
        const std::string sql = "INSERT INTO followers (follower_id, followed_id) VALUES (?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt); 
            throw std::runtime_error("Erro ao preparar relação de seguidor: " + erro);
        }

        sqlite3_bind_int(stmt, 1, follower_id);
        sqlite3_bind_int(stmt, 2, followed_id);

        int resultado = sqlite3_step(stmt);
        sqlite3_finalize(stmt); 

        if (resultado != SQLITE_DONE) 
        {
            std::string erro = sqlite3_errmsg(db.get());
            throw std::runtime_error("Erro ao relacionar utilizadores: " + erro);
        }

        return true; 
    }

    bool removerSeguidor(int follower_id, int followed_id)
    {
        const std::string sql = "DELETE FROM followers WHERE follower_id = ? AND followed_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar remoção de seguidor: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, follower_id);
        sqlite3_bind_int(stmt, 2, followed_id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt); 
            throw std::runtime_error("Erro ao remover seguidor: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    int countFollowers(int followed_id)
    {
        const std::string sql = "SELECT COUNT(*) FROM followers WHERE followed_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao preparar contagem de seguidores: " + erro);
        }

        sqlite3_bind_int(stmt, 1, followed_id);

        int count = 0;
        int resultado = sqlite3_step(stmt);

        if (resultado == SQLITE_ROW) count = sqlite3_column_int(stmt, 0); 

        else if (resultado != SQLITE_DONE)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao contar seguidores: " + erro);
        }

        sqlite3_finalize(stmt);

        return count;
    }

    std::vector<int> listFollowers(int followed_id)
    {
        const std::string sql = "SELECT follower_id FROM followers WHERE followed_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, followed_id);

        std::vector<int> follower_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            follower_ids.push_back(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return follower_ids;
    }

    bool verificarFollower(int follower_id, int followed_id)
    {
        const std::string sql = "SELECT COUNT(*) FROM followers WHERE followed_id = ? AND follower_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, followed_id);
        sqlite3_bind_int(stmt, 2, follower_id);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count > 0;
    }

    void criarTabelaMealPlanner() {
        const char* sql = "CREATE TABLE IF NOT EXISTS meal_planner ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "recipe_id INTEGER NOT NULL, "
            "meal_type TEXT NOT NULL, " // Almoço ou Jantar
            "day_of_week TEXT NOT NULL, " // Segunda-feira, Terça-feira, etc.
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY(recipe_id) REFERENCES receitas(id) ON DELETE CASCADE);";
        char* mensagemErro = nullptr;
        int resultado = sqlite3_exec(db.get(), sql, nullptr, nullptr, &mensagemErro);

        if (resultado != SQLITE_OK) {
            std::string erro(mensagemErro);
            sqlite3_free(mensagemErro);
            throw std::runtime_error("Erro ao criar tabela Meal Planner: " + erro);
        }
    }

    bool adicionarRefeicao(int user_id, int recipe_id, const std::string& meal_type, const std::string& day_of_week) {
        const std::string sql = "INSERT INTO meal_planner (user_id, recipe_id, meal_type, day_of_week) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar inserção no Planeador de Refeições: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, recipe_id);
        sqlite3_bind_text(stmt, 3, meal_type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, day_of_week.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao adicionar refeição: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    bool removerRefeicao(int user_id, int recipe_id, const std::string& day_of_week) {
        const std::string sql = "DELETE FROM meal_planner WHERE user_id = ? AND recipe_id = ? AND day_of_week = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Erro ao preparar a remoção de refeição: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, recipe_id);
        sqlite3_bind_text(stmt, 3, day_of_week.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao remover refeição: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
        return true;
    }

    void criarTabelaNotifs()
    {
        const std::string sql = "CREATE TABLE IF NOT EXISTS notifications ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL, "
            "origin_user_id INTEGER NOT NULL, "
            "message TEXT NOT NULL, "
            "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY(origin_user_id) REFERENCES users(id) ON DELETE CASCADE"
            ");";

        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao criar tabela de notificações: " + erro);
        }

        int resultado = sqlite3_step(stmt);
        if (resultado != SQLITE_DONE)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao executar criação da tabela de notificações: " + erro);
        }

        sqlite3_finalize(stmt);
    }

    void adicionarNotificacao(int user_id, int origin_user_id, const std::string& message)
    {
        const std::string sql = "INSERT INTO notifications (user_id, origin_user_id, message) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao preparar inserção de notificação: " + erro);
        }

        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_int(stmt, 2, origin_user_id);
        sqlite3_bind_text(stmt, 3, message.c_str(), -1, SQLITE_TRANSIENT);

        int resultado = sqlite3_step(stmt);
        if (resultado != SQLITE_DONE)
        {
            std::string erro = sqlite3_errmsg(db.get());
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao inserir notificação: " + erro);
        }

        sqlite3_finalize(stmt);
    }

    std::vector<int> obterNotifs(int user_id)
    {
        const std::string sql = "SELECT id FROM notifications WHERE user_id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, user_id);

        std::vector<int> notif_ids;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            notif_ids.push_back(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return notif_ids;
    }

    std::pair<int, std::string> notifInfo(int notif_id)
    {
        const std::string sql = "SELECT origin_user_id, message FROM notifications WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, notif_id);

        int origin_user_id = 0;
        std::string message;

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            origin_user_id = sqlite3_column_int(stmt, 0);
            const unsigned char* msg = sqlite3_column_text(stmt, 1);
            if (msg)
                message = reinterpret_cast<const char*>(msg);
        }
        else
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro: Notificação com ID " + std::to_string(notif_id) + " não encontrada.");
        }

        sqlite3_finalize(stmt);
        return std::make_pair(origin_user_id, message);
    }

    bool idNotifExiste(int id)
    {
        const std::string sql = "SELECT COUNT(*) FROM notifications WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error("Erro ao preparar consulta: " + std::string(sqlite3_errmsg(db.get())));

        sqlite3_bind_int(stmt, 1, id);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return count > 0;
    }

    void apagarNotif(int id)
    {
        const std::string sql = "DELETE FROM notifications WHERE id = ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Erro ao preparar a remoção de notificação: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Erro ao remover notificação: " + std::string(sqlite3_errmsg(db.get())));
        }

        sqlite3_finalize(stmt);
    }
};

// App Class
class App {
private:
    crow::SimpleApp app;
    Database db;

public:
    App(const std::string& db_name) : db(db_name) {}

    void configurarRotas()
    {
        // Add_User Route
        CROW_ROUTE(app, "/add_user").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username") || !body.has("password")) return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();
                std::string password = body["password"].s();

                if (username.empty() || password.empty()) return crow::response(400, "Erro: Nome de utilizador ou senha não pode ser vazio.");


                if (db.utilizadorExiste(username))  return crow::response(400, "Erro: Nome de utilizador já está em uso.");

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(1, 9999);

                int id;
                do
                {
                    id = dist(gen);
                } while (db.idUtilizadorExiste(id));

                if (db.inserirUtilizador(id, username, password)) return crow::response(201, "Utilizador criado com sucesso! ID: " + std::to_string(id));

                return crow::response(500, "Erro interno ao criar utilizador.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        // Login Route
        CROW_ROUTE(app, "/login").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username") || !body.has("password")) return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();
                std::string password = body["password"].s();

                if (username.empty() || password.empty()) return crow::response(400, "Erro: Nome de user ou senha não pode ser vazio.");

                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: user não encontrado.");

                if (db.senhaCorreta(username, password))
                {
                    crow::json::wvalue resposta;
                    resposta["message"] = "Login bem-sucedido!";
                    return crow::response(200, resposta);
                }
                return crow::response(401, "Erro: Palavra-passe incorreta.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        // Delete User Route
        CROW_ROUTE(app, "/delete_user").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username") || !body.has("password")) return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();
                std::string password = body["password"].s();

                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: User não encontrado.");

                if (db.senhaCorreta(username, password))
                {
                    if (db.apagarUtilizador(username, password))  return crow::response(200, "User excluído com sucesso.");

                    return crow::response(500, "Erro interno ao excluir o user.");
                }
                return crow::response(401, "Erro: Palavra-passe incorreta.");
            }
            catch (const std::exception& e) {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/get_user").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("user_id")) return crow::response(400, "Erro: Dados inválidos.");


                    int user_id = body["user_id"].i();
                    if (!db.idUtilizadorExiste(user_id)) return crow::response(404, "Erro: Utilizador não encontrado.");

                    std::string username = db.obterUsername(user_id);
                    crow::json::wvalue resposta;
                    resposta["username"] = username;

                    return crow::response(200, resposta);
                }
                catch (const std::exception& e) {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        // Logout Route
        CROW_ROUTE(app, "/logout").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                return crow::response(200, "Logout bem-sucedido.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        // Add_Recipe Route
        CROW_ROUTE(app, "/add_recipe").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username") || !body.has("titulo") || !body.has("ingredientes") || !body.has("passos"))
                {
                    return crow::response(400, "Erro: Dados inválidos.");
                }

                std::string username = body["username"].s();
                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: Utilizador não encontrado.");

                int user_id = db.obterUtilizadorId(username);

                std::string titulo = body["titulo"].s();
                std::string ingredientes = body["ingredientes"].s();
                std::string passos = body["passos"].s();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(1, 9999);

                int id;
                do
                {
                    id = dist(gen);
                } while (db.idReceitaExiste(id));

                if (db.inserirReceita(id, titulo, ingredientes, passos, user_id))
                {
                    std::vector<int> followers = db.listFollowers(user_id);

                    for (int i = 0; i < followers.size(); i++)
                    {
                        std::string mensagem = "Este utilizador adicionou uma nova receita chamada: " + titulo + "!";
                        db.adicionarNotificacao(followers[i], user_id, mensagem);
                    }
                    return crow::response(201, "Receita adicionada com sucesso!");
                }

                return crow::response(500, "Erro interno ao adicionar receita.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        //edit recipe route
        CROW_ROUTE(app, "/edit_recipe").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("id") || !body.has("titulo") || !body.has("ingredientes") || !body.has("passos"))
                        return crow::response(400, "Erro: Dados inválidos.");

                    int id = body["id"].i();
                    std::string titulo = body["titulo"].s();
                    std::string ingredientes = body["ingredientes"].s();
                    std::string passos = body["passos"].s();

                    if(!db.idReceitaExiste(id))
                        return crow::response(400, "Erro: id de receita não existe.");

                    if (db.editarReceita(id, titulo, ingredientes, passos))
                    {
                        //obter pessoas que guardaram receita
                        std::vector<int>savers = db.obterRecipeSavers(id);
                        
                        for (int i = 0; i < savers.size(); i++)
                        {
                            //obter id do dono da receita
                            int user_id = db.obterUtilizadorPorReceita(id);
                            std::string mensagem = "Este utilizador editou a receita chamada: " + titulo + "!";
                            db.adicionarNotificacao(savers[i], user_id, mensagem);
                        }
                        return crow::response(200, "Receita editada com sucesso.");
                    }
                    else
                        return crow::response(500, "Erro interno ao editar receita.");
                }
                catch (const std::exception& e)
                {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        CROW_ROUTE(app, "/list_recipes").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username")) return crow::response(400, "Erro: Dados inválidos.");


                std::string username = body["username"].s();
                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: Utilizador não encontrado.");

                int user_id = db.obterUtilizadorId(username);
                auto receitas = db.obterReceitasPorUtilizador(user_id);

                crow::json::wvalue resposta;
                std::vector<crow::json::wvalue> lista;
                for (int id : receitas) lista.push_back(id);
                resposta["recipes"] = std::move(lista);
                return crow::response(200, resposta);
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/get_recipe").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("id")) return crow::response(400, "Erro: Dados inválidos.");


                int id = body["id"].i();
                if (!db.idReceitaExiste(id)) return crow::response(404, "Erro: Receita não encontrada.");

                auto receita = db.obterReceitaPorId(id);

                crow::json::wvalue resposta;
                resposta["titulo"] = std::get<0>(receita);
                resposta["ingredientes"] = std::get<1>(receita);
                resposta["passos"] = std::get<2>(receita);

                return crow::response(200, resposta);
            }
            catch (const std::exception& e) {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/delete_recipe").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("recipe_id") || !body.has("username"))
                        return crow::response(400, "Erro: Dados inválidos.");

                    int recipe_id = body["recipe_id"].i();
                    std::string username = body["username"].s();

                    if (!db.utilizadorExiste(username))
                        return crow::response(404, "Erro: Utilizador não encontrado.");
                    int user_id = db.obterUtilizadorId(username);

                    if (!db.idReceitaExiste(recipe_id))
                        return crow::response(404, "Erro: Receita não encontrada.");

                    if (db.apagarReceita(recipe_id, user_id))
                        return crow::response(200, "Receita apagada com sucesso.");
                    else
                        return crow::response(500, "Erro interno ao apagar receita.");
                }
                catch (const std::exception& e) {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        CROW_ROUTE(app, "/save_recipe").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username") || !body.has("recipe_id"))
                    return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();
                int recipe_id = body["recipe_id"].i();

                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: Utilizador não encontrado.");

                int user_id = db.obterUtilizadorId(username);

                if (!db.idReceitaExiste(recipe_id)) return crow::response(404, "Erro: Receita não encontrada.");

                if (db.salvarReceita(user_id, recipe_id)) return crow::response(201, "Receita guardada com sucesso!");

                return crow::response(500, "Erro interno ao guardar receita.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/list_saved_recipes").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username"))
                    return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();

                if (!db.utilizadorExiste(username)) return crow::response(404, "Erro: Utilizador não encontrado.");

                int user_id = db.obterUtilizadorId(username);
                auto receitas_salvas = db.obterReceitasSalvas(user_id);

                crow::json::wvalue resposta;
                std::vector<crow::json::wvalue> lista;
                for (int id : receitas_salvas)
                    lista.push_back(id);
                resposta["saved_recipes"] = std::move(lista);

                return crow::response(200, resposta);
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/follow").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("follower_username") || !body.has("followed_username"))
                    return crow::response(400, "Erro: Dados inválidos.");

                std::string follower_username = body["follower_username"].s();
                std::string followed_username = body["followed_username"].s();

                if (!db.utilizadorExiste(follower_username) || !db.utilizadorExiste(followed_username))
                    return crow::response(404, "Erro: Utilizador não encontrado.");

                int follower_id = db.obterUtilizadorId(follower_username);
                int followed_id = db.obterUtilizadorId(followed_username);

                if (db.adicionarSeguidor(follower_id, followed_id))
                {
                    db.adicionarNotificacao(followed_id, follower_id, "Este utilizador começou a seguir-te!");
                    return crow::response(200, "Seguiu utilizador com sucesso");
                }

                return crow::response(500, "Erro interno ao seguir utilizador.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/unfollow").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("follower_username") || !body.has("followed_username"))
                    return crow::response(400, "Erro: Dados inválidos.");

                std::string follower_username = body["follower_username"].s();
                std::string followed_username = body["followed_username"].s();

                if (!db.utilizadorExiste(follower_username) || !db.utilizadorExiste(followed_username))
                    return crow::response(404, "Erro: Utilizador não encontrado.");

                int follower_id = db.obterUtilizadorId(follower_username);
                int followed_id = db.obterUtilizadorId(followed_username);

                if (db.removerSeguidor(follower_id, followed_id))
                    return crow::response(200, "Deixou de seguir utilizador com sucesso");

                return crow::response(500, "Erro interno ao deixar de seguir utilizador.");
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/count_followers").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("followed_username"))
                        return crow::response(400, "Erro: Dados inválidos.");

                    std::string followed_username = body["followed_username"].s();

                    if (!db.utilizadorExiste(followed_username))
                        return crow::response(404, "Erro: Utilizador não encontrado.");

                    int followed_id = db.obterUtilizadorId(followed_username);

                    int n_followers = db.countFollowers(followed_id);

                    crow::json::wvalue resposta;
                    resposta["n_followers"] = n_followers;

                    return crow::response(200, resposta);
                }
                catch (const std::exception& e)
                {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        CROW_ROUTE(app, "/check_follow").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("follower_username") || !body.has("followed_username"))
                        return crow::response(400, "Erro: Dados inválidos.");

                    std::string follower_username = body["follower_username"].s();
                    std::string followed_username = body["followed_username"].s();

                    if (!db.utilizadorExiste(follower_username) || !db.utilizadorExiste(followed_username))
                        return crow::response(404, "Erro: Utilizador não encontrado.");

                    int follower_id = db.obterUtilizadorId(follower_username);
                    int followed_id = db.obterUtilizadorId(followed_username);

                    if (db.verificarFollower(follower_id, followed_id))
                        return crow::response(200, "Utilizador segue o outro");

                    return crow::response(500, "Erro interno ao deixar de seguir utilizador.");
                }
                catch (const std::exception& e)
                {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        CROW_ROUTE(app, "/search").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("search"))
                        return crow::response(400, "Erro: Dados inválidos.");

                    std::string search = body["search"].s();

                    std::vector<int> user_ids = db.procurarUtilizadores(search);

                    if (user_ids.size() >= 0)
                    {
                        crow::json::wvalue resposta;
                        std::vector<crow::json::wvalue> list;
                        for (int id : user_ids) list.push_back(id);
                        resposta["user_ids"] = std::move(list);
                        return crow::response(200, resposta);
                    }
                       
                    return crow::response(500, "Erro interno ao pesuisar por utilizador");
                }
                catch (const std::exception& e)
                {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });
    
        CROW_ROUTE(app, "/upload_image").methods("POST"_method)([this](const crow::request& req)
        {
            // Tenta interpretar o corpo como JSON
            auto body = crow::json::load(req.body);
            if (!body || !body.has("image") || !body.has("op")) return crow::response(400, "Erro: Dados inválidos.");

            std::string op = body["op"].s();
            std::string image_base64 = body["image"].s();

            std::string image_data;
            try 
            {
                image_data = base64_decode(image_base64);
            }
            catch (const std::exception& e) 
            {
                return crow::response(400, "Erro ao descodificar a imagem");
            }

            int file_counter = 0;
            std::ifstream counter_file_in("file_counter.txt");
            if (counter_file_in.is_open()) {
                counter_file_in >> file_counter;
                counter_file_in.close();
            }

            std::string file_name = "images/i_" + std::to_string(file_counter++) + ".jpg";
            std::ofstream file(file_name, std::ios::binary);
            file.write(image_data.data(), image_data.size());
            file.close();

            std::ofstream counter_file_out("file_counter.txt", std::ios::trunc);
            counter_file_out << file_counter;
            counter_file_out.close();

            //guarda na database dependendo da op
            if (op[0] == 'r')
            {
                //ver se body tem id de receita
                if (!body.has("id"))
                    return crow::response(400, "Erro: id não fornecido");
                int id = body["id"].i();
                //verificar se tem id da receita
                if (!db.idReceitaExiste(id))
                    return crow::response(400, "Erro: id de receita não existe.");
                //adicionar caminho à tabela de receitas
                if (!db.atualizarImagemReceita(file_name, id))
                    return crow::response(400, "Erro: interno");
            }
            else if (op[0] == 'p')
            {
                if (!body.has("username"))
                    return crow::response(400, "Erro: username não fornecido");
                std::string username = body["username"].s();

                if (!db.utilizadorExiste(username))
                    return crow::response(400, "Erro: user não existe");
                int id = db.obterUtilizadorId(username);
                if (!db.atualizarPfp(file_name, id))
                    return crow::response(400, "Erro: interno");
            }
            else if (op[0] == 'b')
            {
                if (!body.has("username"))
                    return crow::response(400, "Erro: username não fornecido");
                std::string username = body["username"].s();

                if (!db.utilizadorExiste(username))
                    return crow::response(400, "Erro: user não existe");
                int id = db.obterUtilizadorId(username);
                if (!db.atualizarBanner(file_name, id))
                    return crow::response(400, "Erro: interno");
            }
            // Responde com sucesso e os campos adicionais
            return crow::response(200, "Imagem guardada com sucesso");
        });
    
        CROW_ROUTE(app, "/download_image").methods("POST"_method)([this](const crow::request& req)
        {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("path"))
                return crow::response(400, "Erro: Dados inválidos.");

            std::string file_path = body["path"].s();

            std::ifstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                return crow::response(404, "Ficheiro não encontrado!");
            }

            std::ostringstream file_content;
            file_content << file.rdbuf();
            file.close();

            return crow::response(200, file_content.str());
        });

        CROW_ROUTE(app, "/adicionar_refeicao").methods("POST"_method)([this](const crow::request& req) {
            try {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("user_id") || !body.has("recipe_id") || !body.has("meal_type") || !body.has("day_of_week")) {
                    return crow::response(400, "Erro: Dados inválidos.");
                }

                int user_id = body["user_id"].i();
                int recipe_id = body["recipe_id"].i();
                std::string meal_type = body["meal_type"].s();
                std::string day_of_week = body["day_of_week"].s();

                if (db.adicionarRefeicao(user_id, recipe_id, meal_type, day_of_week)) {
                    return crow::response(201, "Refeição adicionada com sucesso!");
                }
                return crow::response(500, "Erro interno ao adicionar refeição.");
            }
            catch (const std::exception& e) {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
            });

        CROW_ROUTE(app, "/remover_refeicao").methods("POST"_method)([this](const crow::request& req) {
            try {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("user_id") || !body.has("recipe_id") || !body.has("day_of_week")) {
                    return crow::response(400, "Erro: Dados inválidos.");
                }

                int user_id = body["user_id"].i();
                int recipe_id = body["recipe_id"].i();
                std::string day_of_week = body["day_of_week"].s();

                if (db.removerRefeicao(user_id, recipe_id, day_of_week)) {
                    return crow::response(200, "Refeição removida com sucesso!");
                }
                return crow::response(500, "Erro interno ao remover a refeição.");
            }
            catch (const std::exception& e) {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
            });

        CROW_ROUTE(app, "/get_notifications").methods("POST"_method)([this](const crow::request& req)
        {
            try
            {
                auto body = crow::json::load(req.body);
                if (!body || !body.has("username"))
                    return crow::response(400, "Erro: Dados inválidos.");

                std::string username = body["username"].s();

                if(!db.utilizadorExiste(username)) return crow::response(400, "Erro: Utilizador não existe.");

                int user_id = db.obterUtilizadorId(username);

                std::vector<int> notif_ids = db.obterNotifs(user_id);

                crow::json::wvalue resposta;
                std::vector<crow::json::wvalue> lista;
                for (int id : notif_ids)
                    lista.push_back(id);
                resposta["notifications"] = std::move(lista);

                return crow::response(200, resposta);
            }
            catch (const std::exception& e)
            {
                return crow::response(500, std::string("Erro interno: ") + e.what());
            }
        });

        CROW_ROUTE(app, "/get_notif").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("id")) return crow::response(400, "Erro: Dados inválidos.");


                    int id = body["id"].i();
                    if (!db.idNotifExiste(id)) return crow::response(404, "Erro: Notificação não encontrada.");

                    auto notification = db.notifInfo(id);

                    crow::json::wvalue resposta;
                    resposta["origin_user_id"] = std::get<0>(notification);
                    resposta["message"] = std::get<1>(notification);

                    return crow::response(200, resposta);
                }
                catch (const std::exception& e) {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });

        CROW_ROUTE(app, "/read_notif").methods("POST"_method)([this](const crow::request& req)
            {
                try
                {
                    auto body = crow::json::load(req.body);
                    if (!body || !body.has("id")) return crow::response(400, "Erro: Dados inválidos.");


                    int id = body["id"].i();
                    if (!db.idNotifExiste(id)) return crow::response(404, "Erro: Notificação não encontrada.");

                    db.apagarNotif(id);

                    return crow::response(200, "Notificação apagada com sucesso");
                }
                catch (const std::exception& e) {
                    return crow::response(500, std::string("Erro interno: ") + e.what());
                }
            });
    }

    void run(int port) {
        app.port(port).multithreaded().run();
    }

};

// Função Principal
int main()
{
    try
    {
        App app("users.db");
        app.configurarRotas();
        app.run(18080);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}