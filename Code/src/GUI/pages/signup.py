import flet as ft
import requests
import re

BASE_URL = "http://127.0.0.1:18080"

class SignupPage(ft.UserControl):
    def __init__(self, on_signup, show_login_page):
        super().__init__()
        self.on_signup = on_signup
        self.show_login_page = show_login_page
        self.snackbar = ft.SnackBar(content=ft.Text(""))

    def build(self):
        # Components for the signup page
        self.username = ft.TextField(label="Username", width=250, autofocus=True)
        self.password = ft.TextField(label="Password", password=True, width=250)
        self.confirm_password = ft.TextField(label="Confirm Password", password=True, width=250)
        self.success_message = ft.Text(value="", color="green")

        # Layout of the signup page
        return ft.Container(
            content=ft.Row(
                controls=[
                    ft.Container(
                        width=500,  # Define total width of the container to help with centering
                        content=ft.Column(
                            controls=[
                                ft.Text(value="Create a New Account", size=30, weight="bold"),
                                ft.Container(height=20),  # Add space between the title and text fields
                                self.username,
                                self.password,
                                self.confirm_password,
                                ft.Container(height=20),  # Add space before the sign-up button
                                ft.ElevatedButton(text="Sign Up", on_click=self.handle_signup),
                                self.success_message,
                                ft.TextButton(
                                    text="Already have an account? Log in now.",
                                    on_click=lambda e: self.show_login_page(),
                                    style=ft.ButtonStyle(
                                        color=ft.colors.BLUE,
                                    ),
                                ),
                                self.snackbar
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                            horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                            spacing=10  # Increase spacing between elements
                        ),
                        alignment=ft.alignment.center,
                        padding=20,
                    )
                ],
                alignment=ft.MainAxisAlignment.CENTER,
                vertical_alignment=ft.CrossAxisAlignment.CENTER,
                expand=True
            ),
            alignment=ft.alignment.center,
            expand=True
        )

    def handle_signup(self, e):
        # Validate username and password based on the defined rules
        username_error = self.validate_username(self.username.value)
        password_errors = self.validate_password(self.password.value)

        if username_error:
            self.show_snackbar(username_error)
            return
        if password_errors:
            self.show_snackbar("\n".join(password_errors))
            return
        if self.password.value != self.confirm_password.value:
            self.show_snackbar("Passwords do not match.")
            return

        # Check if the username and password fields are filled
        if self.username.value and self.password.value:
            print(f"Creating user: {self.username.value}")

            url = f"{BASE_URL}/add_user"
            payload = {
                "username": self.username.value,
                "password": self.password.value
            }
            response = requests.post(url, json=payload)

            if response.status_code == 201:  # User created successfully
                print("User created successfully.")
                self.success_message.value = "User created successfully! You can now log in."
                self.success_message.update()
                # Call the callback function to proceed to the next step, if needed
                self.on_signup
            elif response.status_code == 400:  # Username already in use or validation error
                print("Signup failed: username already in use.")
                self.show_snackbar("Username is already in use. Please choose another one.")
            else:  # Other unexpected errors
                print("Unexpected error during signup.")
                self.show_snackbar("An unexpected error occurred. Please try again later.")
        else:
            # Error message if any field is empty
            print("Signup failed: fields are missing.")
            self.show_snackbar("All fields are required.")

    def validate_username(self, username):
        if len(username) < 3:
            return "Username must be at least 3 characters long."
        if not re.match("^[A-Za-z0-9_]+$", username):
            return "Username can only contain letters, numbers, and underscores."
        return None

    def validate_password(self, password):
        errors = []
        if len(password) < 8:
            errors.append("Password must be at least 8 characters long.")
        if not re.search("[a-z]", password):
            errors.append("Password must contain at least one lowercase letter.")
        if not re.search("[A-Z]", password):
            errors.append("Password must contain at least one uppercase letter.")
        if not re.search("[0-9]", password):
            errors.append("Password must contain at least one number.")
        if not re.search("[!@#$%^&*()_+=]", password):
            errors.append("Password must contain at least one special character.")
        if len(password) > 20:
            errors.append("Password must be at most 20 characters long.")
        return errors if errors else None

    def show_snackbar(self, message):
        self.snackbar.content.value = message
        self.snackbar.open = True
        self.update()

# Function to instantiate the signup page
def signup_page(on_signup, show_login_page):
    return SignupPage(on_signup, show_login_page)
