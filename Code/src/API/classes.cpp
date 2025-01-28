#include <iostream>
#include <vector>
#include <string>

class Ingredient 
{
    public:
        std::string name;
        float quantity;
        Ingredient(const std::string& ingr_name, float ingr_quant): name(ingr_name), quantity(ingr_quant) {}
};

class Review 
{
    public:
        int userID;
        int rate;
        std::string comment;
        Review(int user, int rating,const std::string& review): userID(user), rate(rating), comment(review) {}
};

class Notification
{
    public:
        std::string message;
        bool readStatus;
        void markASRead()
        {
            readStatus=true;
        }
};

class Recipe
{
public:
    int recipeID;
    int creatorID;
    std::string name;
    std::string instructions;
    std::vector<int> likes;
    std::vector<Ingredient> ingredients;
    std::vector<Review> reviews;

    void printRecipe()
    {
        std::cout << "Receita: " << name << "\nInstruções: " << instructions << std::endl;
    }

    void addReview(int userID, int rating, const std::string& comment)
    {
        reviews.push_back(Review(userID,rating,comment));
    }
    void addLike(int userID)
    {
        likes.push_back(userID);
    }
    void addIngredient(std::string ingr_name, int ingr_quant) 
    {
        ingredients.push_back(Ingredient(ingr_name,ingr_quant));
    }
    void setName(std::string rec_name)
    {
        name = rec_name;
    }
    void setID(int id)
    {
        recipeID = id;
    }
};
class User 
{
    private:
        int id;
        std::string username;
        std::string password;

        std::vector<Recipe> UserRecipes;
        std::vector<Notification> UserNotifs;
    public:
        std::vector<int> followers;
        std::vector<int> following;

        void setUsername(const std::string& newUsername) {
            username = newUsername;
        }

        std::string getUsername() const {
            return username;
        }

        void setPassword(const std::string& newPassword) {
            if (newPassword.length() == 10) {
                password = newPassword;
            } else {
                std::cout << "A senha deve ter exatamente 10 caracteres." << std::endl;
            }
        }

        std::string getPassword() const {
            return password;
        }

        void createRecipe(std::string rec_name) 
        {
            Recipe new_rec = Recipe();
            new_rec.setName(rec_name);
            //placeholder (it will have to ask the server what the next available ID is)
            int newNumber = 12457;
            //
            new_rec.setID(newNumber);
            new_rec.creatorID = id;
            addRecipe(new_rec);
        }

        void addRecipe(Recipe &rec_to_add)
        {
            UserRecipes.push_back(rec_to_add);
        }

        void deleteRecipe(int ID) 
        {
            for (int i = 0; i < int(UserRecipes.size()); i++) if(UserRecipes[i].recipeID == ID) UserRecipes.erase(UserRecipes.begin() + i);
        }

        void editRecipe() 
        {
        
        }

        void addNotification(std::string message)
        {
            Notification new_notif = Notification();
            new_notif.message = message;
            new_notif.readStatus = false;
            UserNotifs.push_back(new_notif);
        }
};
class MealPlan 
{
    public:
        std::vector<int> weekday;
        bool time;
        std::vector<Recipe> recipes;

        void plan()
        {

        }
        void showPlan()
        {

        }

        void editPlan()
        {
            
        }
};