import flet as ft

class Sidebar(ft.UserControl):
    def __init__(self, on_section_selected, username):
        self.on_section_selected = on_section_selected
        self.username = username  # Store the username
        self.current_section = "Home"  # Track the current selected section
        super().__init__()

    def build(self):
        # Build the sidebar based on the current state
        return ft.Container(
            content=ft.Column(
                controls=[
                    # User info at the top of the sidebar (always visible)
                    ft.Container(
                        content=ft.Row(
                            controls=[
                                ft.Container(
                                    width=42,
                                    height=42,
                                    bgcolor="bluegrey900",
                                    alignment=ft.alignment.center,
                                    border_radius=8,
                                    content=ft.Text(
                                        value=self.username[0].upper(),  # First letter of the username
                                        size=20,
                                        weight="bold",
                                        color="white"
                                    ),
                                ),
                                ft.Column(
                                    spacing=2,
                                    controls=[
                                        ft.Text(
                                            value=self.username,
                                            size=14,
                                            weight="bold",
                                            color="white"
                                        ),
                                    ]
                                )
                            ],
                            alignment="start",
                        ),
                        padding=ft.padding.all(10),
                        on_click=lambda e: self.on_section_selected_handler("Profile"),  # Click to open the profile
                    ),
                    ft.Divider(height=10, color="transparent"),
                    # Sidebar menu items (removing "Profile")
                    self.create_list_tile(ft.icons.HOME, "Home", "Home"),
                    self.create_list_tile(ft.icons.SEARCH, "Search", "Search"),
                    self.create_list_tile(ft.icons.BOOK, "Own Recipes", "Own Recipes"),
                    self.create_list_tile(ft.icons.BOOKMARK, "Saved Recipes", "Saved Recipes"),
                    self.create_list_tile(ft.icons.FOOD_BANK, "Meal Planner", "Meal Planner"),
                    self.create_list_tile(ft.icons.NOTIFICATIONS, "Notifications", "Notifications"),
                    self.create_list_tile(ft.icons.SETTINGS, "Settings", "Settings"),
                    # Flexible space to push the logout button to the end
                    ft.Container(expand=True),
                    # Logout button
                    self.create_list_tile(ft.icons.LOGOUT, "Log Out", "Logout", highlight_color="red"),
                ],
                expand=True,
                spacing=5,
            ),
            padding=10,
            width=200,  # Fixed width for the sidebar
            bgcolor=ft.colors.BLUE_GREY_900,
            border_radius=10,
        )

    def create_list_tile(self, icon_name, title, section, highlight_color="white10"):
        # Check if the current section is the one selected
        is_selected = section == self.current_section
        bgcolor = "bluegrey700" if is_selected else None

        return ft.Container(
            width=180,  # Fixed width for items
            height=50,
            border_radius=10,
            on_hover=lambda e: self.highlight_tile(e, highlight_color),
            on_click=lambda _: self.on_section_selected_handler(section),
            content=ft.Row(
                controls=[
                    ft.Icon(
                        name=icon_name,
                        color="white54" if not is_selected else "white",
                        size=24
                    ),
                    ft.Text(
                        value=title,
                        color="white" if not is_selected else "white",
                        size=14,
                        weight="bold",
                    ),
                ],
                spacing=10,
                alignment="start",
            ),
            padding=ft.padding.symmetric(horizontal=10, vertical=10),
            bgcolor=bgcolor
        )

    def on_section_selected_handler(self, section):
        # Update the current section to the selected one
        self.current_section = section
        # Call the original section selection function
        self.on_section_selected(section)
        # Update the control to reflect the new selected section
        self.update()

    def highlight_tile(self, e, color):
        if e.data == "true":
            e.control.bgcolor = color
            e.control.update()
        else:
            e.control.bgcolor = None
            e.control.update()
