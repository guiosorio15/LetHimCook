import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"
PLACEHOLDER_PROFILE_PIC = "https://via.placeholder.com/150"
PLACEHOLDER_BANNER = "https://via.placeholder.com/800x200"

class OtherProfilePage(ft.UserControl):
    def __init__(self, page, username, back_to_search):
        super().__init__()
        self.page = page
        self.username = username
        self.back_to_search = back_to_search
        self.user_info = {}  # To store additional user info, e.g., connections

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
                    # Profile Picture
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
                    # Username and connections
                    ft.Container(
                        content=ft.Column(
                            controls=[
                                ft.Text(
                                    self.username,
                                    size=24,
                                    weight="bold",
                                    color="white",
                                ),
                                ft.Row(
                                    controls=[
                                        ft.Text(
                                            f"{self.user_info.get('connections', 'x')} connections",
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
                    # Back Button
                    ft.Container(
                        content=ft.ElevatedButton(
                            text="Back",
                            icon=ft.icons.ARROW_BACK,
                            bgcolor="blue",
                            color="white",
                            on_click=self.handle_back_click,  # Proper handler
                        ),
                        alignment=ft.alignment.bottom_right,
                        padding=ft.padding.only(right=20, bottom=20),
                    ),
                ],
                expand=True,
            ),
        )

    def handle_back_click(self, e):
        """Handle the back button click."""
        print("Back button clicked, returning to search page...")
        self.back_to_search()  # Call the callback to go back to the search page


# Function to instantiate the other profile page
def other_profile_page(page, username, back_to_search):
    return OtherProfilePage(page, username, back_to_search)
