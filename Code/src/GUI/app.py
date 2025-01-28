import flet as ft
from components.sidebar import Sidebar
from pages.home import home_page
from pages.test_search import search_page
from pages.own_recipes import own_recipes_page
from pages.saved_recipes import saved_recipes_page
from pages.notifications import notifications_page
from pages.profile import profile_page
from pages.settings import settings_page
from pages.login import login_page
from pages.signup import signup_page
from pages.logout import logout_dialog
from pages.other_profile import other_profile_page
from pages.meal_planner import meal_planner_page

class AppController:
    def __init__(self, page: ft.Page):
        self.page = page
        self.page.title = "LetHimCook!"
        self.page.window_width = 1280
        self.page.window_height = 720
        self.page.window_resizable = False
        self.page.padding = 20
        self.page.theme_mode = ft.ThemeMode.DARK

        self.content = ft.Column(expand=True)  # Main content container
        self.sidebar = None  # Sidebar will be initialized later
        self.username = None  # Placeholder for the authenticated username
        self.SHOW_LOGIN_FIRST = True  # Flag to control login behavior

    def run(self):
        """Entry point to start the application."""
        if self.SHOW_LOGIN_FIRST:
            self.show_login_page()
        else:
            self.load_main_layout("Guest")  # Default to "Guest" user

    def show_login_page(self):
        """Displays the login page."""
        print("Showing login page...")

        def go_to_signup():
            self.page.controls.clear()
            signup_content = ft.Column(expand=True)
            signup_content.controls.append(signup_page(self.show_signup_page, self.show_login_page))
            self.page.add(signup_content)
            self.page.update()

        def handle_successful_login(username):
            self.username = username
            self.load_main_layout(username)

        self.page.controls.clear()
        self.content.controls.clear()
        self.content.controls.append(login_page(handle_successful_login, go_to_signup))
        self.page.add(self.content)
        self.page.update()

    def show_signup_page(self):
        """Displays the signup page."""
        print("Showing signup page...")
        self.page.controls.clear()
        signup_content = ft.Column(expand=True)
        signup_content.controls.append(signup_page(self.show_signup_page, self.show_login_page))
        self.page.add(signup_content)
        self.page.update()

    def open_full_profile(self, username):
        """Opens the full profile of another user."""
        print(f"Opening full profile of {username}")
        self.content.controls.clear()  # Clear the content
        self.content.controls.append(
            other_profile_page(
                page=self.page,
                username=username,
                back_to_search=lambda: self.load_main_layout(self.username)  # Return to the main layout
            )
        )

        # Ensure `self.content` is part of the page layout
        if self.content not in self.page.controls:
            self.page.controls.clear()  # Clear current layout
            self.page.add(ft.Row([self.sidebar, self.content], expand=True))

        self.page.update()  # Update the page


    def load_main_layout(self, username):
        print("Loading main layout...")

        def on_section_selected(section):
            print(f"Section selected: {section}")
            self.content.controls.clear()

            if section == "Home":
                self.content.controls.append(home_page())
            elif section == "Search":
                self.content.controls.append(search_page(username, self.open_full_profile))
            elif section == "Own Recipes":
                self.content.controls.append(own_recipes_page(self.page, username))
            elif section == "Saved Recipes":
                self.content.controls.append(saved_recipes_page(self.page, username))
            elif section == "Meal Planner":
                self.content.controls.append(meal_planner_page(username))
            elif section == "Notifications":
                self.content.controls.append(notifications_page())
            elif section == "Profile":
                self.content.controls.append(profile_page(self.page, username, self.show_login_page))
            elif section == "Settings":
                self.content.controls.append(settings_page(self.page, self.username, self.show_login_page))
            elif section == "Logout":
                logout_dialog(self.page, self.show_login_page)

            self.page.update()

        self.sidebar = Sidebar(on_section_selected, username)
        self.page.controls.clear()
        main_layout = ft.Row([self.sidebar, self.content], expand=True)
        self.page.add(main_layout)
        self.page.update()

        on_section_selected("Home")



# Flet entry point
def main(page: ft.Page):
    app = AppController(page)
    app.run()

ft.app(target=main)
