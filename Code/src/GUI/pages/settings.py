import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"


class SettingsPage(ft.UserControl):
    def __init__(self, page, username, show_login_page):
        super().__init__()
        self.page = page
        self.username = username
        self.show_login_page = show_login_page

    def build(self):
        return ft.Container(
            width=1280,
            height=720,
            padding=20,
            content=ft.Column(
                controls=[
                    ft.Text("Settings", size=26, weight="bold"),
                    # Account Settings
                    ft.Container(
                        content=ft.Text("Account Settings", size=18, weight="bold"),
                        padding=ft.padding.only(top=10),
                    ),
                    ft.Row(
                        controls=[
                            ft.ElevatedButton(
                                text="Change Password",
                                icon=ft.icons.LOCK,
                                bgcolor="blue",
                                color="white",
                                on_click=self.open_change_password_dialog,
                                width=300,
                            ),
                            ft.ElevatedButton(
                                text="Delete Account",
                                icon=ft.icons.DELETE,
                                bgcolor="red",
                                color="white",
                                on_click=self.open_delete_dialog,
                                width=300,
                            ),
                        ],
                        spacing=10,
                    ),
                    ft.Divider(height=20),

                    # Theme Settings
                    ft.Container(
                        content=ft.Text("Theme Settings", size=18, weight="bold"),
                        padding=ft.padding.only(top=10),
                    ),
                    ft.Row(
                        controls=[
                            ft.Switch(
                                label="Dark Mode",
                                on_change=self.toggle_theme,
                                value=self.page.theme_mode == ft.ThemeMode.DARK,
                            ),
                        ],
                        spacing=10,
                    ),
                    ft.Divider(height=20),

                    # Notifications
                    ft.Container(
                        content=ft.Text("Notifications", size=18, weight="bold"),
                        padding=ft.padding.only(top=10),
                    ),
                    ft.Row(
                        controls=[
                            ft.Switch(label="Push Notifications", on_change=self.toggle_push_notifications),
                        ],
                        spacing=10,
                    ),
                    ft.Divider(height=20),

                    # General Settings
                    ft.Container(
                        content=ft.Text("General Settings", size=18, weight="bold"),
                        padding=ft.padding.only(top=10),
                    ),
                    ft.Dropdown(
                        label="Language",
                        options=[
                            ft.dropdown.Option("English"),
                        ],
                        on_change=self.change_language,
                        width=300,
                    ),
                    ft.ElevatedButton(
                        text="Reset to Defaults",
                        icon=ft.icons.RESET_TV,
                        on_click=self.reset_to_defaults,
                        width=300,
                    ),
                ],
                spacing=15,  # Reduce spacing between elements
                expand=True,
            ),
        )

    def open_delete_dialog(self, e):
        """Open the dialog to confirm account deletion."""
        self.password_field = ft.TextField(label="Confirm your password", password=True, autofocus=True)
        self.error_message = ft.Text(value="", color="red")

        self.page.dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Confirm Account Deletion", color="red", size=20, weight="bold"),
            content=ft.Column(
                controls=[
                    ft.Text("Please confirm your password to delete your account."),
                    self.password_field,
                    self.error_message
                ],
                tight=True
            ),
            actions=[
                ft.ElevatedButton(
                    "Delete Account",
                    icon=ft.icons.DELETE,
                    bgcolor="red",
                    on_click=self.process_delete_account
                ),
                ft.TextButton("Cancel", on_click=self.close_dialog)
            ],
            actions_alignment="center"
        )
        self.page.dialog.open = True
        self.page.update()

    def process_delete_account(self, e):
        """Send a request to delete the account."""
        url = f"{BASE_URL}/delete_user"
        payload = {
            "username": self.username,
            "password": self.password_field.value
        }
        response = requests.post(url, json=payload)

        if response.status_code == 200:
            print("Account deleted successfully.")
            self.close_dialog(e)
            self.show_login_page()
        elif response.status_code == 404:
            self.error_message.value = "User does not exist. Please check the username."
            self.error_message.update()
        elif response.status_code == 401:
            self.error_message.value = "Incorrect password. Please try again."
            self.error_message.update()
        else:
            self.error_message.value = "An unexpected error occurred. Please try again later."
            self.error_message.update()

    def open_change_password_dialog(self, e):
        """Open the dialog to change the user's password."""
        self.current_password_field = ft.TextField(label="Current Password", password=True, autofocus=True)
        self.new_password_field = ft.TextField(label="New Password", password=True)
        self.confirm_password_field = ft.TextField(label="Confirm New Password", password=True)
        self.error_message = ft.Text(value="", color="red")

        self.page.dialog = ft.AlertDialog(
            modal=True,
            title=ft.Text("Change Password", color="blue", size=20, weight="bold"),
            content=ft.Column(
                controls=[
                    self.current_password_field,
                    self.new_password_field,
                    self.confirm_password_field,
                    self.error_message
                ],
                tight=True
            ),
            actions=[
                ft.ElevatedButton(
                    "Save",
                    icon=ft.icons.SAVE,
                    bgcolor="blue",
                    on_click=self.process_change_password
                ),
                ft.TextButton("Cancel", on_click=self.close_dialog)
            ],
            actions_alignment="center"
        )
        self.page.dialog.open = True
        self.page.update()

    def process_change_password(self, e):
        """Process the password change request."""
        current_password = self.current_password_field.value
        new_password = self.new_password_field.value
        confirm_password = self.confirm_password_field.value

        if not current_password or not new_password or not confirm_password:
            self.error_message.value = "All fields are required."
            self.error_message.update()
            return

        if new_password != confirm_password:
            self.error_message.value = "New password and confirmation do not match."
            self.error_message.update()
            return

        url = f"{BASE_URL}/change_password"
        payload = {
            "username": self.username,
            "current_password": current_password,
            "new_password": new_password
        }
        response = requests.post(url, json=payload)

        if response.status_code == 200:
            print("Password changed successfully.")
            self.close_dialog(e)
        elif response.status_code == 401:
            self.error_message.value = "Incorrect current password."
            self.error_message.update()
        else:
            self.error_message.value = "An error occurred. Please try again later."
            self.error_message.update()

    def close_dialog(self, e):
        """Close the active dialog."""
        if self.page.dialog:
            self.page.dialog.open = False
            self.page.update()

    def toggle_theme(self, e):
        """Toggle between Dark and Light mode."""
        if e.control.value:
            self.page.theme_mode = ft.ThemeMode.DARK
        else:
            self.page.theme_mode = ft.ThemeMode.LIGHT
        self.page.update()

    def toggle_push_notifications(self, e):
        print("Push Notifications toggled:", e.control.value)

    def change_language(self, e):
        print("Language changed to:", e.control.value)

    def reset_to_defaults(self, e):
        print("Settings reset to default values.")


# Function to instantiate the settings page
def settings_page(page, username, show_login_page):
    return SettingsPage(page, username, show_login_page)
