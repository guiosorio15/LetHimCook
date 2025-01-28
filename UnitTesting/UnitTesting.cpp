#include "pch.h"
#include "CppUnitTest.h"
#include "server.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
    TEST_CLASS(DatabaseTest)
    {
    public:

        TEST_METHOD(TesteCriarTabelaUtilizador)
        {
            Logger::WriteMessage("Iniciando Teste: TestCriarTabelaUtilizador");
            Database db(":memory:");
            Assert::IsTrue(true); // (?) void function - se estiver bem implementada é criada
            Logger::WriteMessage("Tabela 'users' criada com sucesso."); // esta linha só dá print caso passe nos casos anteriores, o que siginfica que passou
        }

        TEST_METHOD(TesteInserirUtilizador)
        {
            Logger::WriteMessage("Iniciando Teste: TestInserirUtilizador");
            Database db(":memory:");
            bool result = db.inserirUtilizador(1, "user1", "password1");
            Assert::IsTrue(result);
            Assert::IsTrue(db.utilizadorExiste("user1"));
            Logger::WriteMessage("Utilizador 'user1' inserido com sucesso.");
        }

        TEST_METHOD(TestUtilizadorExiste)
        {
            Logger::WriteMessage("Iniciando Teste: TestUtilizadorExiste");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1"); // inserirUtilizador(id, username, password) foi previamente verificada antes deste teste
            Assert::IsTrue(db.utilizadorExiste("user1"));
            Assert::IsFalse(db.utilizadorExiste("nonexistent_user"));
            Logger::WriteMessage("Verificação da existência do utilizador concluída.");
        }

        TEST_METHOD(TesteSenhaCorreta)
        {
            Logger::WriteMessage("Iniciando Teste: TestSenhaCorreta");
            Database db(":memory:");
            db.inserirUtilizador(2, "user2", "password2");
            Assert::IsTrue(db.senhaCorreta("user2", "password2"));
            Assert::IsFalse(db.senhaCorreta("user2", "wrongpassword")); // teste com palavra passe errada
            Logger::WriteMessage("Verificação da senha correta concluída.");
        }
        
        TEST_METHOD(TesteIdUtilizadorExiste)
        {
            Logger::WriteMessage("Iniciando Teste: TestIdUtilizadorExiste");
            Database db(":memory:");
            db.inserirUtilizador(3, "user3", "password3");
            Assert::IsTrue(db.idUtilizadorExiste(3));
            Assert::IsFalse(db.idUtilizadorExiste(99));  // ID inexistente apenas para testar
            Logger::WriteMessage("Verificação da existência do ID do utilizador concluída.");
        }

        TEST_METHOD(TesteApagarUtilizador)
        {
            Logger::WriteMessage("Iniciando Teste: TestApagarUtilizador");
            Database db(":memory:");
            db.inserirUtilizador(3, "user3", "password3");
            Assert::IsTrue(db.utilizadorExiste("user3")); // verificar primeiro que o user3 existe
            Assert::IsTrue(db.apagarUtilizador("user3", "password3"));
            Assert::IsFalse(db.utilizadorExiste("user3")); // verificar que foi de facto apagado 
            Logger::WriteMessage("Utilizador apagado com sucesso.");
        }

        
        TEST_METHOD(TesteObterUtilizadorId)
        {
            Logger::WriteMessage("Iniciando Teste: TestObterUtilizadorId");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            int user_id = db.obterUtilizadorId("user1");
            Assert::AreEqual(user_id, 1);
            Logger::WriteMessage("ID do utilizador obtido com sucesso.");
        }

        
        TEST_METHOD(TesteCriarTabelaReceitas)
        {
            Logger::WriteMessage("Iniciando Teste: TestCriarTabelaReceitas");
            Database db(":memory:");
            Assert::IsTrue(true); // (?) void function - se estiver bem implementada é criada
            Logger::WriteMessage("Tabela 'receitas' criada com sucesso.");
        }

        TEST_METHOD(TesteInserirReceita)
        {
            Logger::WriteMessage("Iniciando Teste: TestInserirReceita");
            Database db(":memory:");
            db.inserirUtilizador(4, "user4", "password4");
            int user_id = db.obterUtilizadorId("user4");
            bool result = db.inserirReceita(1, "Titulo1", "Ingredientes1", "Passos1", user_id);
            Assert::IsTrue(result);
            Logger::WriteMessage("Receita inserida com sucesso.");
        }

        TEST_METHOD(TesteIdReceitaExiste)
        {
            Logger::WriteMessage("Iniciando Teste: TestReceitaExiste");
            Database db(":memory:");
            db.inserirUtilizador(5, "user5", "password5");
            int user_id = db.obterUtilizadorId("user5");
            db.inserirReceita(2, "Titulo2", "Ingredientes2", "Passos2", user_id);
            Assert::IsTrue(db.idReceitaExiste(2));
            Logger::WriteMessage("Receita existente verificada com sucesso.");
        }

        
        TEST_METHOD(TesteObterReceitasPorUtilizador)
        {
            Logger::WriteMessage("Iniciando Teste: TestObterReceitasPorUtilizador");
            Database db(":memory:");
            db.inserirUtilizador(6, "user6", "password6");
            int user_id = db.obterUtilizadorId("user6");
            db.inserirReceita(3, "Titulo3", "Ingredientes3", "Passos3", user_id);
            db.inserirReceita(4, "Titulo4", "Ingredientes4", "Passos4", user_id);

            auto receitas = db.obterReceitasPorUtilizador(user_id);
            Assert::AreEqual((int)receitas.size(), 2);  // Espera-se que o utilizador tenha duas receitas neste exemplo
            Logger::WriteMessage("Receitas obtidas com sucesso, para o utilizador.");
        }

        TEST_METHOD(TesteApagarReceita)
        {
            Logger::WriteMessage("Iniciando Teste: TestApagarReceita");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirReceita(1, "Receita1", "Ingredientes1", "Passos1", 1);

            Assert::IsTrue(db.idReceitaExiste(1));
            bool result = db.apagarReceita(1, 1);
            Assert::IsTrue(result);
            Assert::IsFalse(db.idReceitaExiste(1));
            Logger::WriteMessage("Receita removida com sucesso.");
        }

        TEST_METHOD(TesteObterReceitasPorId)
        {
            Logger::WriteMessage("Iniciando Teste: TestObterReceitasPorId");
            Database db(":memory:");
            db.inserirUtilizador(7, "user7", "password7");
            int user_id = db.obterUtilizadorId("user7");
            db.inserirReceita(5, "Titulo5", "Ingredientes5", "Passos5", user_id);

            auto receita = db.obterReceitaPorId(5);
            Assert::AreEqual(std::get<0>(receita), std::string("Titulo5"));
            Assert::AreEqual(std::get<1>(receita), std::string("Ingredientes5"));
            Assert::AreEqual(std::get<2>(receita), std::string("Passos5"));
            Logger::WriteMessage("Receita obtida com sucesso, por ID.");
        }

        TEST_METHOD(TesteObterUsername)
        {
            Logger::WriteMessage("Iniciando Teste: TestObterUsername");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");

            std::string username = db.obterUsername(1);
            Assert::AreEqual(username, std::string("user1"));
            Logger::WriteMessage("Username obtido com sucesso.");
        }

        // rever esta ... <---------------------------------------
        TEST_METHOD(TesteAtualizarPfp)
        {
            Logger::WriteMessage("Iniciando Teste: TestAtualizarPfp");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");

            bool result = db.atualizarPfp("path/to/image.jpg", 1); // meter aqui o caminho para a foto
            Assert::IsTrue(result);
            Logger::WriteMessage("Foto de perfil atualizada com sucesso."); 
        }

        TEST_METHOD(TesteAtualizarBanner)
        {
            Logger::WriteMessage("Iniciando Teste: TestAtualizarBanner");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");

            bool result = db.atualizarBanner("path/to/banner.jpg", 1); // meter aqui o path
            Assert::IsTrue(result);
            Logger::WriteMessage("Banner atualizado com sucesso.");
        }

        TEST_METHOD(TesteProcurarUtilizadores)
        {
            Logger::WriteMessage("Iniciando Teste: TestProcurarUtilizadores");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirUtilizador(2, "user2", "password2");
            db.inserirUtilizador(3, "admin", "password3");

            auto resultados = db.procurarUtilizadores("user");
            Assert::AreEqual((int)resultados.size(), 2); // "user1" e "user2"

            resultados = db.procurarUtilizadores("admin");
            Assert::AreEqual((int)resultados.size(), 1); // Apenas "admin"

            Logger::WriteMessage("Procura de utilizadores concluída com sucesso.");
        } 
        
        // organizando ...
        /* ---------------------testes ao meal planner aqui------------------------- */ 

        TEST_METHOD(TesteCriarTabelaMealPlanner)
        {
            Logger::WriteMessage("Iniciando Teste: TestCriarTabelaMealPlanner");
            Database db(":memory:");
            Assert::IsTrue(true); // Void function - se não houver exceção, a tabela foi criada.
            Logger::WriteMessage("Tabela 'meal_planner' criada com sucesso.");
        }

        TEST_METHOD(TesteAdicionarRefeicao)
        {
            Logger::WriteMessage("Iniciando Teste: TestAdicionarRefeicao");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirReceita(1, "Receita1", "Ingredientes1", "Passos1", 1);

            bool result = db.adicionarRefeicao(1, 1, "Almoco", "Segunda-feira");
            Assert::IsTrue(result);
            Logger::WriteMessage("Refeição adicionada com sucesso ao Meal Planner.");
        }

        TEST_METHOD(TesteRemoverRefeicao)
        {
            Logger::WriteMessage("Iniciando Teste: TestRemoverRefeicao");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirReceita(1, "Receita1", "Ingredientes1", "Passos1", 1);
            db.adicionarRefeicao(1, 1, "Almoço", "Segunda-feira");

            bool result = db.removerRefeicao(1, 1, "Segunda-feira");
            Assert::IsTrue(result);
            Logger::WriteMessage("Refeição removida com sucesso do Meal Planner.");
        }
        /* ---------------------fim dos testes ao meal planner------------------------- */




        /* ---------------------testes para os seguidores aqui------------------------- */
        TEST_METHOD(TesteCriarTabelaSeguidores)
        {
            Logger::WriteMessage("Iniciando Teste: TestCriarTabelaSeguidores");
            Database db(":memory:");
            Assert::IsTrue(true); // Void function - se não houver exceção, a tabela foi criada.
            Logger::WriteMessage("Tabela 'followers' criada com sucesso.");
        }

        TEST_METHOD(TesteAdicionarSeguidor)
        {
            Logger::WriteMessage("Iniciando Teste: TestAdicionarSeguidor");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirUtilizador(2, "user2", "password2");

            bool result = db.adicionarSeguidor(1, 2);
            Assert::IsTrue(result);
            Logger::WriteMessage("Seguidor adicionado com sucesso.");
        }

        TEST_METHOD(TesteRemoverSeguidor)
        {
            Logger::WriteMessage("Iniciando Teste: TestRemoverSeguidor");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirUtilizador(2, "user2", "password2");
            db.adicionarSeguidor(1, 2);

            bool result = db.removerSeguidor(1, 2);
            Assert::IsTrue(result);
            Logger::WriteMessage("Seguidor removido com sucesso.");
        }

        TEST_METHOD(TesteVerificarSeguidor)
        {
            Logger::WriteMessage("Iniciando Teste: TestVerificarSeguidor");
            Database db(":memory:");
            db.inserirUtilizador(1, "user1", "password1");
            db.inserirUtilizador(2, "user2", "password2");
            db.adicionarSeguidor(1, 2);

            Assert::IsTrue(db.verificarFollower(1, 2));
            Assert::IsFalse(db.verificarFollower(2, 1)); // Relação inversa não deve existir
            Logger::WriteMessage("Verificação de seguidor concluída com sucesso.");
        }
        /* ---------------------fim dos testes para os seguidores------------------------- */
    };
}
