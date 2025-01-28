import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"

class LoginPage(ft.UserControl):
    def __init__(self, on_login, go_to_signup):
        super().__init__()
        self.on_login = on_login
        self.go_to_signup = go_to_signup

    def build(self):
        # UI components for the login page
        self.username = ft.TextField(label="Username", width=250, autofocus=True)
        self.password = ft.TextField(label="Password", password=True, width=250)
        self.error_message = ft.Text(value="", color="red")

        # Server status indicator
        self.server_status_text = ft.Text(size=14, color="white")

        # Layout of the login page
        return ft.Stack(
            controls=[
                ft.Row(
                    controls=[
                        ft.Container(
                            width=500,  # Define total width of the container to help with centering
                            height=500,  # Define total height of the container to help with centering
                            content=ft.Column(
                                controls=[
                                    ft.Text(value="LetHimCook!", size=30, weight="bold"),
                                    ft.Container(height=20),
                                    self.username,
                                    self.password,
                                    ft.ElevatedButton(text="Login", on_click=self.handle_login),
                                    self.error_message,
                                    ft.TextButton(
                                        text="Don't have an account yet? Sign up now.",
                                        on_click=lambda e: self.go_to_signup(),
                                        style=ft.ButtonStyle(
                                            color=ft.colors.BLUE,
                                        ),
                                    ),
                                ],
                                alignment=ft.MainAxisAlignment.CENTER,
                                horizontal_alignment=ft.CrossAxisAlignment.CENTER,
                                spacing=10,
                            ),
                            alignment=ft.alignment.center,
                            padding=20,
                        )
                    ],
                    alignment=ft.MainAxisAlignment.CENTER,
                    vertical_alignment=ft.CrossAxisAlignment.CENTER,
                    expand=True
                ),
                # Adding server status to the bottom-left corner
                ft.Container(
                    content=self.server_status_text,
                    alignment=ft.alignment.bottom_left,
                    padding=ft.padding.all(10),
                )
            ],
            expand=True
        )

    def did_mount(self):
        # Method to be called after the control is added to the UI
        self.update_server_status()

    def update_server_status(self):
        # Check server status and update the text accordingly
        try:
            # Attempting to make a request to the server status endpoint
            response = requests.get(f"{BASE_URL}/status", timeout=5)
            if response.status_code == 200:
                self.server_status_text.value = "Server Status: ON"
            else:
                self.server_status_text.value = "Server Status: OFF"
        except requests.exceptions.ConnectionError:
            self.server_status_text.value = "Server Status: OFF"
        except requests.exceptions.Timeout:
            self.server_status_text.value = "Server Status: OFF"
        except Exception as ex:
            # For any other exceptions, set server status to OFF
            self.server_status_text.value = f"Server Status: OFF ({str(ex)})"

        # Update the UI to reflect the server status
        self.server_status_text.update()

    def handle_login(self, e):
        url = f"{BASE_URL}/login"
        payload = {
            "username": self.username.value,
            "password": self.password.value
        }
        try:
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print("Login successful. Calling on_login...")
                self.on_login(self.username.value)  # Pass username to the main layout
            elif response.status_code == 404:  # User not found
                self.error_message.value = "User does not exist. Please check the username."
                self.error_message.update()
            elif response.status_code == 401:  # Incorrect password
                self.error_message.value = "Incorrect password. Please try again."
                self.error_message.update()
            else:  # Other unexpected errors
                self.error_message.value = "An unexpected error occurred. Please try again later."
                self.error_message.update()
        except requests.exceptions.ConnectionError:
            self.error_message.value = "Unable to connect to the server. Please try again later."
            self.error_message.update()
        except requests.exceptions.Timeout:
            self.error_message.value = "Request timed out. Please try again."
            self.error_message.update()
        except Exception as ex:
            self.error_message.value = f"An error occurred: {str(ex)}"
            self.error_message.update()

# Function to instantiate the login page
def login_page(on_login, go_to_signup):
    return LoginPage(on_login, go_to_signup)
