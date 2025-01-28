import flet as ft
import requests

BASE_URL = "http://127.0.0.1:18080"
PLACEHOLDER_PROFILE_PIC = "https://via.placeholder.com/150"
PLACEHOLDER_BANNER = "https://via.placeholder.com/900x200"


class SearchPage(ft.UserControl):
    def __init__(self):
        super().__init__()
        self.search_results = []  # List to hold search results
        self.usernames_found = []  # List of usernames found
        self.user_list = None  # Column to display the user list
        self.popup_container = None
        self.current_user = "current_user"  # Replace this with the actual logged-in user

    def build(self):
        # Initialize the popup container
        self.popup_container = ft.Container(
            width=650,  # Popup width
            height=550,  # Popup height
            bgcolor=None,  # Initially no background
            content=None,  # Empty initially
            border_radius=ft.border_radius.all(35),
        )

        # Initialize the user list container
        self.user_list = ft.Column(
            controls=[],  # This will be dynamically updated
            spacing=10,
        )

        # Main layout
        return ft.Container(
            content=ft.Column(
                controls=[
                    # Search bar
                    self.create_search_bar(),
                    ft.Row(
                        controls=[
                            # User list container
                            ft.Container(
                                content=self.user_list,
                                width=300,
                                height=550,
                                padding=10,
                                border_radius=10,
                            ),
                            # Popup container beside the user list
                            ft.Container(
                                content=self.popup_container,
                                alignment=ft.alignment.top_left,
                            ),
                        ],
                        alignment=ft.MainAxisAlignment.START,
                    ),
                ],
                alignment=ft.MainAxisAlignment.START,
            ),
            padding=20,
            expand=True,
        )

    def create_search_bar(self):
        """Create the search bar."""
        return ft.Container(
            content=ft.TextField(
                label="Search users...",
                prefix_icon=ft.icons.SEARCH,  # Search icon
                on_change=self.perform_search,  # Trigger search on input
            ),
            width=1200,
            height=50,
            margin=ft.margin.only(top=20, left=20),
            expand=True,
        )

    def perform_search(self, e):
        """Perform a search on the server and update the user list."""
        search_query = e.control.value.strip()

        # Clear results if the search field is empty
        if not search_query:
            self.search_results = []
            self.usernames_found = []
            self.update_user_list()
            return

        try:
            # Send a request to the `/search` endpoint
            url = f"{BASE_URL}/search"
            payload = {"search": search_query}
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                user_ids = response.json().get("user_ids", [])
                self.usernames_found = []  # Clear previous results

                # Fetch usernames for each user_id
                for user_id in user_ids:
                    url = f"{BASE_URL}/get_user"
                    payload = {"user_id": user_id}
                    user_response = requests.post(url, json=payload)

                    if user_response.status_code == 200:
                        username = user_response.json().get("username")
                        if username:
                            self.usernames_found.append(username)

                self.update_user_list()
            else:
                print(f"Search failed: {response.text}")
        except Exception as ex:
            print(f"Error while searching: {ex}")

    def update_user_list(self):
        """Update the user list with search results."""
        self.user_list.controls.clear()

        for username in self.usernames_found:
            self.user_list.controls.append(self.create_user_card(username))

        self.user_list.update()

    def create_user_card(self, username):
        """Create a card for each username in the results."""
        return ft.Container(
            content=ft.Row(
                controls=[
                    ft.Text(
                        username,
                        size=14,
                        weight="bold",
                        color="white",
                    ),
                    ft.ElevatedButton(
                        text="Follow",
                        on_click=lambda e, username=username: self.follow_user(username),
                        bgcolor="teal",  # Button background color to match the theme
                        color="white",  # Button text color
                        style=ft.ButtonStyle(
                            shape=ft.RoundedRectangleBorder(radius=10),
                            elevation=2,
                        ),
                    ),
                ],
                spacing=10,
                alignment=ft.MainAxisAlignment.SPACE_BETWEEN,
            ),
            bgcolor="bluegrey400",
            border_radius=10,
            padding=10,
            width=250,
            on_hover=lambda e, username=username: self.handle_hover(e, username),
        )

    def follow_user(self, username):
        """Follow the selected user."""
        try:
            print(f"Trying to follow: {username}")
            url = f"{BASE_URL}/follow"
            payload = {"follower_username": self.current_user, "followed_username": username}
            print(f"Payload: {payload}")
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print(f"Successfully followed {username}")
            elif response.status_code == 404:
                print("User not found!")
            else:
                print(f"Error: {response.status_code}, {response.text}")
        except Exception as ex:
            print(f"Error while following user: {ex}")

    def handle_hover(self, e, username):
        """Handle hover events."""
        if e.data == "true":  # Mouse entered
            print(f"Hovered over {username}")
            self.open_popup(username)
        else:  # Mouse left
            print(f"Mouse left {username}")
            self.close_popup()

    def open_popup(self, username):
        """Open a profile popup in the predestined container."""
        self.popup_container.bgcolor = "bluegrey900"  # Set background color on hover
        self.popup_container.content = ft.Stack(
            controls=[
                # Banner
                ft.Container(
                    content=ft.Image(
                        src=PLACEHOLDER_BANNER,
                        fit=ft.ImageFit.COVER,
                    ),
                    height=150,
                    width=900,
                ),
                # Profile picture overlapping banner
                ft.Container(
                    content=ft.Image(
                        src=PLACEHOLDER_PROFILE_PIC,
                        fit=ft.ImageFit.COVER,
                    ),
                    height=120,
                    width=120,
                    border_radius=ft.border_radius.all(75),
                    border=ft.border.all(3, ft.colors.WHITE),
                    alignment=ft.alignment.top_center,
                    margin=ft.margin.only(left=30, top=130),
                ),
                # Username below profile picture
                ft.Container(
                    content=ft.Text(
                        f"{username}",
                        size=24,
                        weight="bold",
                        color="white",
                    ),
                    alignment=ft.alignment.top_left,
                    margin=ft.margin.only(left=50, top=260),
                ),
            ],
        )
        self.popup_container.update()

    def close_popup(self):
        """Close the popup by clearing the content in the popup container."""
        self.popup_container.bgcolor = None  # Remove background color
        self.popup_container.content = None
        self.popup_container.update()


# Function to instantiate the search page
def search_page():
    return SearchPage()