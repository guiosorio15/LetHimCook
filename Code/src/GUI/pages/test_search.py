import flet as ft
import requests



BASE_URL = "http://127.0.0.1:18080"
PLACEHOLDER_PROFILE_PIC = "https://via.placeholder.com/150"
PLACEHOLDER_BANNER = "https://via.placeholder.com/900x200"


class SearchPage(ft.UserControl):
    def __init__(self, username, open_full_profile):
        super().__init__()
        self.search_results = []
        self.usernames_found = []
        self.user_list = None
        self.popup_container = None
        self.current_user = username
        self.following_users = set()
        self.open_full_profile = open_full_profile

    def build(self):
        self.popup_container = ft.Container(
            width=650,
            height=550,
            bgcolor=None,
            content=None,
            border_radius=ft.border_radius.all(35),
        )
        self.user_list = ft.Column(
            controls=[],
            spacing=10,
        )

        return ft.Container(
            content=ft.Column(
                controls=[
                    self.create_search_bar(),
                    ft.Row(
                        controls=[
                            ft.Container(
                                content=self.user_list,
                                width=300,
                                height=550,
                                padding=10,
                                border_radius=10,
                            ),
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
        return ft.Container(
            content=ft.TextField(
                label="Search users...",
                prefix_icon=ft.icons.SEARCH,
                on_change=self.perform_search,
            ),
            width=1200,
            height=50,
            expand=True,
        )

    def perform_search(self, e):
        search_query = e.control.value.strip()

        if not search_query:
            self.search_results = []
            self.usernames_found = []
            self.following_users.clear()
            self.update_user_list()
            return

        try:
            url = f"{BASE_URL}/search"
            payload = {"search": search_query}
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                user_ids = response.json().get("user_ids", [])
                self.usernames_found = []

                for user_id in user_ids:
                    url_user = f"{BASE_URL}/get_user"
                    payload_user = {"user_id": user_id}
                    user_response = requests.post(url_user, json=payload_user)

                    if user_response.status_code == 200:
                        username = user_response.json().get("username")
                        if username:
                            self.usernames_found.append(username)

                # Verifica quem já é seguido
                self.following_users.clear()
                for usr in self.usernames_found:
                    if self.check_follow(usr):
                        self.following_users.add(usr)

                self.update_user_list()
            else:
                print(f"Search failed: {response.text}")
        except Exception as ex:
            print(f"Error while searching: {ex}")

    def check_follow(self, username):
        if not self.current_user or self.current_user == username:
            return False
        url = f"{BASE_URL}/check_follow"
        payload = {"follower_username": self.current_user, "followed_username": username}
        response = requests.post(url, json=payload)
        return response.status_code == 200

    def update_user_list(self):
        self.user_list.controls.clear()

        for username in self.usernames_found:
            self.user_list.controls.append(self.create_user_card(username))

        self.user_list.update()

    def create_user_card(self, username):
        is_following = username in self.following_users
        follow_button_text = "Followed" if is_following else "Follow"
        follow_button_action = self.unfollow_user if is_following else self.follow_user

        return ft.Container(
            content=ft.Row(
                controls=[
                ft.Container(
                    content=ft.Text(
                        username,
                        size=14,
                        weight="bold",
                        color="white",
                    ),
                    on_click=lambda e, uname=username: self.open_full_profile(uname),
                    padding=ft.padding.all(5),
                    border_radius=ft.border_radius.all(5),
                    bgcolor="transparent",  # Opcional para dar feedback visual
                    ),
                    ft.ElevatedButton(
                        text=follow_button_text,
                        on_click=lambda e, username=username: follow_button_action(username),
                        bgcolor="teal" if not is_following else "gray",
                        color="white",
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
        try:
            print(f"Trying to follow: {username}")
            url = f"{BASE_URL}/follow"
            payload = {"follower_username": self.current_user, "followed_username": username}
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print(f"Successfully followed {username}")
                self.following_users.add(username)
                self.update_user_list()
            elif response.status_code == 404:
                print("User not found!")
            else:
                print(f"Error: {response.status_code}, {response.text}")
        except Exception as ex:
            print(f"Error while following user: {ex}")

    def unfollow_user(self, username):
        try:
            print(f"Trying to unfollow: {username}")
            url = f"{BASE_URL}/unfollow"
            payload = {"follower_username": self.current_user, "followed_username": username}
            response = requests.post(url, json=payload)

            if response.status_code == 200:
                print(f"Successfully unfollowed {username}")
                if username in self.following_users:
                    self.following_users.remove(username)
                self.update_user_list()
            else:
                print(f"Failed to unfollow {username}: {response.text}")
        except Exception as ex:
            print(f"Error while unfollowing user: {ex}")

    def handle_hover(self, e, username):
        if e.data == "true":
            print(f"Hovered over {username}")
            self.open_popup(username)
        else:
            print(f"Mouse left {username}")
            self.close_popup()

    def open_popup(self, username):
        self.popup_container.bgcolor = "bluegrey900"
        self.popup_container.content = ft.Stack(
            controls=[
                ft.Container(
                    content=ft.Image(
                        src=PLACEHOLDER_BANNER,
                        fit=ft.ImageFit.COVER,
                    ),
                    height=150,
                    width=900,
                ),
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
        self.popup_container.bgcolor = None
        self.popup_container.content = None
        self.popup_container.update()


def search_page(username, open_full_profile):
    return SearchPage(username, open_full_profile)
