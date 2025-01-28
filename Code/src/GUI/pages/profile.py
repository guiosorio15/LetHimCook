import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"
PLACEHOLDER_PROFILE_PIC = "https://via.placeholder.com/150"
PLACEHOLDER_BANNER = "https://via.placeholder.com/800x200"

class ProfilePage(ft.UserControl):
    def __init__(self, page, username, show_login_page):
        super().__init__()
        self.page = page
        self.username = username
        self.show_login_page = show_login_page

    def build(self):
        return ft.Container(
            width=1080,
            height=650,
            content=ft.Stack(
                controls=[
                    # Banner
                    ft.Container(
                        content=ft.Image(
                            src=PLACEHOLDER_BANNER,
                            fit=ft.ImageFit.COVER,
                        ),
                        height=200,
                        width=1080,
                    ),
                    # Foto de perfil sobreposta
                    ft.Container(
                        content=ft.Image(
                            src=PLACEHOLDER_PROFILE_PIC,
                            fit=ft.ImageFit.COVER,
                        ),
                        height=150,
                        width=150,
                        border_radius=ft.border_radius.all(75),
                        border=ft.border.all(3, ft.colors.WHITE),
                        alignment=ft.alignment.bottom_left,
                        margin=ft.margin.only(left=30, top=130),
                    ),
                    # Nome do usuário e informações de seguidores
                    ft.Container(
                        content=ft.Column(
                            controls=[
                                ft.Text(
                                    self.username,
                                    size=24,
                                    weight="bold",
                                    color="white",
                                ),
                                # Adicione as informações de followers e followed aqui
                                ft.Row(
                                    controls=[
                                        ft.Text(
                                            f"x connections",
                                            size=12,
                                            weight="bold",
                                            color="white",
                                        ),
                                    ],
                                    spacing=20,
                                ),
                            ],
                        ),
                        alignment=ft.alignment.top_left,
                        margin=ft.margin.only(left=80, top=290),
                    ),
                    # Botões no canto inferior direito
                    ft.Container(
                        content=ft.Row(
                            controls=[
                                ft.ElevatedButton(
                                    text="Edit Profile",
                                    icon=ft.icons.EDIT,
                                    bgcolor="blue",
                                    color="white",
                                    on_click=self.edit_profile,
                                ),
                                ft.ElevatedButton(
                                    text="Delete Account",
                                    icon=ft.icons.DELETE,
                                    bgcolor="red",
                                    color="white",
                                    on_click=self.open_delete_dialog,
                                ),
                            ],
                            alignment=ft.MainAxisAlignment.END,
                            spacing=10,
                        ),
                        alignment=ft.alignment.bottom_right,
                        padding=ft.padding.only(right=5, bottom=20),
                    ),
                ],
                expand=True,
            ),
        )

    def edit_profile(self, e):
        print("Edit Profile button clicked")

    def open_delete_dialog(self, e):
        self.page.dialog = self.create_delete_dialog()
        self.page.dialog.open = True
        self.page.update()

    def create_delete_dialog(self):
        self.password_field = ft.TextField(label="Confirm your password", password=True, autofocus=True)
        self.error_message = ft.Text(value="", color="red")

        return ft.AlertDialog(
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
                ft.ElevatedButton("Delete Account", icon=ft.icons.DELETE, bgcolor="red", on_click=self.process_delete_account),
                ft.TextButton("Cancel", on_click=self.close_delete_dialog)
            ],
            actions_alignment="center"
        )

    def close_delete_dialog(self, e):
        if self.page.dialog:
            self.page.dialog.open = False
            self.page.update()

    def process_delete_account(self, e):
        url = f"{BASE_URL}/delete_user"
        payload = {
            "username": self.username,
            "password": self.password_field.value
        }
        response = requests.post(url, json=payload)

        if response.status_code == 200:
            print("Account deleted successfully.")
            self.close_delete_dialog(e)
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

# Function to instantiate the profile page
def profile_page(page, username, show_login_page):
    return ProfilePage(page, username, show_login_page)
