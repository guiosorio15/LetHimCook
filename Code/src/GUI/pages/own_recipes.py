import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"
PLACEHOLDER_IMAGE = "https://via.placeholder.com/150"  # Placeholder image URL


class RecipesPage(ft.UserControl):
    def __init__(self, page, username):
        self.page = page
        self.username = username
        super().__init__()
        self.recipes = []  # List to store recipes
        self.recipe_grid = None  # Initialize as None; will be set up in build

    def build(self):
        # Initialize GridView in the build method
        self.recipe_grid = ft.GridView(
            expand=True,
            runs_count=4,
            spacing=10,
            run_spacing=10,
            child_aspect_ratio=1.0,
        )

        # Return the layout of the page
        return ft.Container(
            width=1080,
            height=650,
            content=ft.Column(
                controls=[
                    ft.Text(value="Recipes", size=30, weight="bold"),
                    self.recipe_grid,
                    ft.Container(
                        content=ft.FloatingActionButton(
                            icon=ft.icons.ADD,
                            bgcolor="blue",
                            on_click=self.open_create_recipe_dialog,
                        ),
                        alignment=ft.alignment.bottom_right,
                        padding=ft.padding.only(bottom=20, right=20),
                    ),
                ],
                expand=True,
                spacing=10,
            ),
            padding=20,
            expand=True,
        )

    def did_mount(self):
        """Executed every time the page is accessed."""
        self.fetch_recipes()  # Update recipes when the page is loaded

    def fetch_recipes(self):
        """Fetch recipes from the server and populate the grid."""
        try:
            url = f"{BASE_URL}/list_recipes"
            payload = {"username": self.username}
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                recipe_ids = response.json().get("recipes", [])
                self.recipes = []  # Clear the local list to avoid duplicates
                for recipe_id in recipe_ids:
                    recipe_url = f"{BASE_URL}/get_recipe"
                    recipe_payload = {"id": recipe_id}
                    recipe_response = requests.post(recipe_url, json=recipe_payload)

                    if recipe_response.status_code == 200:
                        recipe_data = recipe_response.json()
                        self.recipes.append({
                            "id": recipe_id,
                            "title": recipe_data.get("titulo", "Untitled"),
                            "ingredients": recipe_data.get("ingredientes", ""),
                            "steps": recipe_data.get("passos", ""),
                            "image_url": recipe_data.get("image_url", PLACEHOLDER_IMAGE),
                        })

                self.update_recipe_grid()
            else:
                print(f"Failed to fetch recipes: {response.text}")
        except Exception as e:
            print(f"Error fetching recipes: {e}")

    def update_recipe_grid(self):
        """Update the grid with the fetched recipes."""
        if not self.recipe_grid:
            print("GridView has not been initialized yet.")
            return

        self.recipe_grid.controls.clear()
        for recipe in self.recipes:
            self.recipe_grid.controls.append(self.create_recipe_card(recipe))
        self.recipe_grid.update()

    def create_recipe_card(self, recipe):
        """Create a card for a recipe with an image and title."""
        return ft.Container(
            width=180,
            height=230,
            content=ft.Column(
                controls=[
                    ft.Container(
                        content=ft.Image(
                            src=recipe["image_url"],
                            fit=ft.ImageFit.COVER,
                            height=140,
                            width=140,
                        ),
                        alignment=ft.alignment.center,
                        border_radius=ft.border_radius.all(70),
                        bgcolor="#ADD8E6",
                        margin=ft.margin.only(top=10),
                    ),
                    ft.Container(
                        content=ft.Column(
                            controls=[
                                ft.Row(
                                    controls=[
                                        ft.Icon(
                                            name=ft.icons.PERSON,
                                            size=16,
                                            color="black",
                                        ),
                                        ft.Text(
                                            recipe["title"],
                                            weight="bold",
                                            size=14,
                                            color="black",
                                            overflow=ft.TextOverflow.ELLIPSIS,
                                        ),
                                    ],
                                    alignment=ft.MainAxisAlignment.CENTER,
                                    spacing=5,
                                ),
                            ],
                            alignment=ft.MainAxisAlignment.END,
                            spacing=0,
                        ),
                        bgcolor="#F4F4F4",
                        padding=ft.padding.all(10),
                        border_radius=ft.border_radius.only(
                            bottom_left=10, bottom_right=10
                        ),
                        margin=ft.margin.only(top=10),
                    ),
                ],
                spacing=0,
                alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
            ),
            border=ft.border.all(color=ft.colors.GREY, width=1),
            border_radius=15,
            bgcolor="#ADD8E6",
            on_click=lambda _: self.open_recipe_details_dialog(recipe),
        )

    def open_recipe_details_dialog(self, recipe):
        """Open a pop-up with the recipe details."""
        self.page.dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text(recipe["title"], size=20, weight="bold", text_align=ft.TextAlign.CENTER),
            content=ft.Container(
                content=ft.Column(
                    controls=[
                        ft.Text("Ingredients:", weight="bold", text_align=ft.TextAlign.CENTER),
                        ft.Text(recipe["ingredients"]),
                        ft.Text("Steps:", weight="bold"),
                        ft.Text(recipe["steps"]),
                    ],
                    spacing=10,
                ),
                width=550,  # Standardized width
                height=400,  # Standardized height
                padding=ft.padding.all(20),
            ),
            actions=[
                ft.TextButton("Delete", on_click=lambda _: self.show_delete_confirmation(recipe)),
                ft.TextButton("Edit", on_click=lambda _: self.open_edit_recipe_dialog(recipe)),
                ft.TextButton("Close", on_click=lambda _: self.close_dialog())
            ],
            actions_alignment="end",
        )
        self.page.dialog.open = True
        self.page.update()

    def delete_recipe(self, recipe):
        """Delete a recipe from the server and update the grid."""
        try:
            url = f"{BASE_URL}/delete_recipe"
            payload = {"recipe_id": recipe["id"], "username": self.username}  # Updated payload
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print("Recipe deleted successfully!")
                # Remove the recipe from the local list
                self.recipes = [r for r in self.recipes if r["id"] != recipe["id"]]
                # Update the grid
                self.update_recipe_grid()
                # Close the dialog
                self.close_dialog()
            else:
                print(f"Error deleting recipe: {response.text}")
        except Exception as e:
            print(f"Error deleting recipe: {e}")


    def show_delete_confirmation(self, recipe):
        """Show a confirmation dialog before deleting a recipe."""
        self.page.dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Confirm Delete", size=20, weight="bold", text_align=ft.TextAlign.CENTER),
            content=ft.Text(
                f"Are you sure you want to delete the recipe '{recipe['title']}'?",
                text_align=ft.TextAlign.CENTER,
            ),
            actions=[
                ft.TextButton("Yes", on_click=lambda _: self.delete_recipe(recipe)),
                ft.TextButton("No", on_click=lambda _: self.close_dialog()),
            ],
            actions_alignment="end",
        )
        self.page.dialog.open = True
        self.page.update()

    def open_edit_recipe_dialog(self, recipe):
        """Open a dialog to edit the selected recipe."""
        # Fields pre-filled with the current recipe data
        self.edit_recipe_name = ft.TextField(
            label="Recipe Name", value=recipe["title"], width=500
        )
        self.edit_ingredients = ft.TextField(
            label="Ingredients (comma separated)", 
            value=recipe["ingredients"], 
            multiline=True, 
            width=500, 
            height=150
        )
        self.edit_steps = ft.TextField(
            label="Steps", 
            value=recipe["steps"], 
            multiline=True, 
            width=500, 
            height=200
        )
        self.error_message = ft.Text(value="", color="red")

        self.page.dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Edit Recipe", size=20, weight="bold"),
            content=ft.Container(
                content=ft.Column(
                    controls=[
                        self.edit_recipe_name,
                        self.edit_ingredients,
                        self.edit_steps,
                        self.error_message,
                    ],
                    spacing=10,
                    width=520,
                ),
                width=550,  # Standardized width
                height=400,  # Standardized height
                padding=ft.padding.all(20),
            ),
            actions=[
                ft.ElevatedButton(
                    "Save", 
                    on_click=lambda _: self.save_edited_recipe(recipe)
                ),
                ft.TextButton("Cancel", on_click=lambda _: self.close_dialog()),
            ],
            actions_alignment="end",
        )
        self.page.dialog.open = True
        self.page.update()

    def save_edited_recipe(self, recipe):
        """Save the edited recipe to the server."""
        new_title = self.edit_recipe_name.value.strip()
        new_ingredients = self.edit_ingredients.value.strip()
        new_steps = self.edit_steps.value.strip()

        if not new_title or not new_ingredients or not new_steps:
            self.error_message.value = "All fields are required."
            self.error_message.update()
            return

        try:
            # Update the recipe on the server
            url = f"{BASE_URL}/edit_recipe"
            payload = {
                "id": recipe["id"],
                "titulo": new_title,
                "ingredientes": new_ingredients,
                "passos": new_steps,
            }
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print("Recipe updated successfully!")
                # Update the local list and grid
                for r in self.recipes:
                    if r["id"] == recipe["id"]:
                        r["title"] = new_title
                        r["ingredients"] = new_ingredients
                        r["steps"] = new_steps
                        break
                self.update_recipe_grid()
                self.close_dialog()
            else:
                self.error_message.value = f"Error: {response.text}"
                self.error_message.update()
        except Exception as e:
            self.error_message.value = f"Server error: {e}"
            self.error_message.update()

    def open_create_recipe_dialog(self, e):
        """Open the dialog to create a new recipe."""
        self.page.dialog = self.create_recipe_dialog_component()
        self.page.dialog.open = True
        self.page.update()

    def create_recipe_dialog_component(self):
        """Create and return the recipe creation dialog component."""
        self.recipe_name = ft.TextField(label="Recipe Name", width=500)
        self.ingredients = ft.TextField(label="Ingredients (comma separated)", multiline=True, width=500, height=150)
        self.steps = ft.TextField(label="Steps", multiline=True, width=500, height=200)
        self.error_message = ft.Text(value="", color="red")

        return ft.AlertDialog(
            modal=True,
            title=ft.Text("Add a New Recipe", size=20, weight="bold"),
            content=ft.Container(
                content=ft.Column(
                    controls=[self.recipe_name, self.ingredients, self.steps, self.error_message],
                    spacing=10,
                    width=520,
                ),
                padding=ft.padding.all(20),
                width=550,
            ),
            actions=[
                ft.ElevatedButton("Save", on_click=self.save_recipe),
                ft.TextButton("Cancel", on_click=lambda _: self.close_dialog()),
            ],
            actions_alignment="end",
        )

    def save_recipe(self, e):
        """Save a new recipe to the server."""
        name = self.recipe_name.value.strip()
        ingredients = self.ingredients.value.strip()
        steps = self.steps.value.strip()

        if not name or not ingredients or not steps:
            self.error_message.value = "All fields are required."
            self.error_message.update()
            return

        try:
            url = f"{BASE_URL}/add_recipe"
            payload = {
                "username": self.username,
                "titulo": name,
                "ingredientes": ingredients,
                "passos": steps,
            }
            response = requests.post(url, json=payload)

            if response.status_code == 201:
                print("Recipe added successfully!")
                self.fetch_recipes()  # Update the recipe list after adding
                self.close_dialog()
            else:
                self.error_message.value = f"Error: {response.text}"
                self.error_message.update()
        except Exception as e:
            self.error_message.value = f"Server error: {e}"
            self.error_message.update()

    def close_dialog(self):
        """Close the active dialog."""
        if self.page.dialog:
            self.page.dialog.open = False
            self.page.update()


# Function to instantiate the recipes page
def own_recipes_page(page, username):
    return RecipesPage(page, username)



